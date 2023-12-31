// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StaticGeometry.h"

#include "BaseLoader.h"
#include "Model.h"
#include "NodeVisitor.h"
#include "Serializer.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "scenegraph/BinaryConverter.h"
#include "utils.h"

namespace SceneGraph {

	StaticGeometry::StaticGeometry(Graphics::Renderer *r) :
		Node(r, NODE_SOLID)
	{
	}

	StaticGeometry::~StaticGeometry()
	{
	}

	StaticGeometry::StaticGeometry(const StaticGeometry &sg, NodeCopyCache *cache) :
		Node(sg, cache),
		m_boundingBox(sg.m_boundingBox),
		m_meshes(sg.m_meshes)
	{
	}

	Node *StaticGeometry::Clone(NodeCopyCache *cache)
	{
		return this; //geometries are shared
	}

	void StaticGeometry::Accept(NodeVisitor &nv)
	{
		nv.ApplyStaticGeometry(*this);
	}

	void StaticGeometry::Render(const matrix4x4f &trans, const RenderData *rd)
	{
		PROFILE_SCOPED()
		Graphics::Renderer *r = GetRenderer();
		r->SetTransform(trans);
		for (auto &it : m_meshes)
			r->DrawMesh(it.meshObject.Get(), it.material.Get());

		//DrawBoundingBox(m_boundingBox);
	}

	void StaticGeometry::Render(const std::vector<matrix4x4f> &trans, const RenderData *rd)
	{
		PROFILE_SCOPED()
		Graphics::Renderer *r = GetRenderer();

		const size_t numTrans = trans.size();
		if (!m_instBuffer.Valid() || (numTrans > m_instBuffer->GetSize())) {
			// create the InstanceBuffer with the maximum number of transformations we might use within it.
			m_instBuffer.Reset(r->CreateInstanceBuffer(numTrans, Graphics::BUFFER_USAGE_DYNAMIC));
		}

		// Update the InstanceBuffer data
		Graphics::InstanceBuffer *ib = m_instBuffer.Get();
		matrix4x4f *pBuffer = ib->Map(Graphics::BUFFER_MAP_WRITE);
		if (pBuffer) {
			// Copy the transforms into the buffer
			for (const matrix4x4f &mt : trans) {
				(*pBuffer) = mt;
				++pBuffer;
			}
			ib->Unmap();
			ib->SetInstanceCount(numTrans);
		}

		// we'll set the transformation within the vertex shader so identity the global one
		r->SetTransform(matrix4x4f::Identity());

		if (m_instanceMaterials.empty()) {
			// process each mesh
			for (auto &it : m_meshes) {
				// Due to the shader needing to change we have to get the material and force it to the instanced variant
				Graphics::MaterialDescriptor mdesc = it.material->GetDescriptor();
				mdesc.instanced = true;

				const Graphics::RenderStateDesc oldDesc = r->GetMaterialRenderState(it.material.Get());
				// create the "new" material with the instanced description
				RefCountedPtr<Graphics::Material> mat(r->CloneMaterial(it.material.Get(), mdesc, oldDesc));
				m_instanceMaterials.push_back(std::move(mat));
			}
		}

		// process each mesh
		int i = 0;
		for (auto &it : m_meshes) {
			// finally render using the instance material
			r->DrawMeshInstanced(it.meshObject.Get(), m_instanceMaterials[i].Get(), m_instBuffer.Get());
			++i;
		}
	}

	typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material>>> MaterialContainer;
	void StaticGeometry::Save(NodeDatabase &db)
	{
		PROFILE_SCOPED()
		Node::Save(db);
		// write obsolete m_blendMode value
		if (SGM_VERSION == 6) db.wr->Int32(Graphics::BLEND_SOLID);
		db.wr->Vector3d(m_boundingBox.min);
		db.wr->Vector3d(m_boundingBox.max);

		db.wr->Int32(m_meshes.size());

		for (auto mesh : m_meshes) {
			//do ptr to material name mapping
			const std::string &matname = db.model->GetNameForMaterial(mesh.material.Get());
			db.wr->String(matname);

			//save vertex attrib description
			const auto &vbDesc = mesh.vertexBuffer->GetDesc();
			Uint32 attribCombo = 0;
			for (Uint32 i = 0; i < Graphics::MAX_ATTRIBS; i++)
				attribCombo |= vbDesc.attrib[i].semantic;

			db.wr->Int32(attribCombo);

			const bool hasTangents = (attribCombo & Graphics::ATTRIB_TANGENT);

			//save positions, normals and uvs interleaved (only known format now)
			const Uint32 posOffset = vbDesc.GetOffset(Graphics::ATTRIB_POSITION);
			const Uint32 nrmOffset = vbDesc.GetOffset(Graphics::ATTRIB_NORMAL);
			const Uint32 uv0Offset = vbDesc.GetOffset(Graphics::ATTRIB_UV0);
			const Uint32 tanOffset = hasTangents ? vbDesc.GetOffset(Graphics::ATTRIB_TANGENT) : 0;
			const Uint32 stride = vbDesc.stride;
			db.wr->Int32(vbDesc.numVertices);
			Uint8 *vtxPtr = mesh.vertexBuffer->Map<Uint8>(Graphics::BUFFER_MAP_READ);
			if (hasTangents) {
				for (Uint32 i = 0; i < vbDesc.numVertices; i++) {
					db.wr->Vector3f(*reinterpret_cast<vector3f *>(vtxPtr + i * stride + posOffset));
					db.wr->Vector3f(*reinterpret_cast<vector3f *>(vtxPtr + i * stride + nrmOffset));
					db.wr->Float(reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset)->x);
					db.wr->Float(reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset)->y);
					db.wr->Vector3f(*reinterpret_cast<vector3f *>(vtxPtr + i * stride + tanOffset));
				}
			} else {
				for (Uint32 i = 0; i < vbDesc.numVertices; i++) {
					db.wr->Vector3f(*reinterpret_cast<vector3f *>(vtxPtr + i * stride + posOffset));
					db.wr->Vector3f(*reinterpret_cast<vector3f *>(vtxPtr + i * stride + nrmOffset));
					db.wr->Float(reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset)->x);
					db.wr->Float(reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset)->y);
				}
			}
			mesh.vertexBuffer->Unmap();

			//indices
			const Uint32 *indexPtr = mesh.indexBuffer->Map(Graphics::BUFFER_MAP_READ);
			const Uint32 numIndices = mesh.indexBuffer->GetSize();
			db.wr->Int32(numIndices);
			for (Uint32 i = 0; i < numIndices; i++)
				db.wr->Int32(indexPtr[i]);
			mesh.indexBuffer->Unmap();
		}
	}

	StaticGeometry *StaticGeometry::Load(NodeDatabase &db)
	{
		PROFILE_SCOPED()
		using namespace Graphics;

		StaticGeometry *sg = new StaticGeometry(db.loader->GetRenderer());
		Serializer::Reader &rd = *db.rd;

		if (SGM_VERSION == 6) rd.Int32(); // read obsolete m_blendMode value
		sg->m_boundingBox.min = rd.Vector3d();
		sg->m_boundingBox.max = rd.Vector3d();

		const Uint32 numMeshes = rd.Int32();
		for (Uint32 mesh = 0; mesh < numMeshes; mesh++) {
			//material
			RefCountedPtr<Graphics::Material> material;
			const std::string matName = rd.String();
			if (starts_with(matName, "decal_")) {
				const unsigned int di = atoi(matName.substr(6).c_str());
				material = db.loader->GetDecalMaterial(di);
			} else
				material = db.model->GetMaterialByName(matName);

			//vertex format check
			const Uint32 vtxFormat = db.rd->Int32();
			if (vtxFormat != (ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0 | ATTRIB_TANGENT) && vtxFormat != (ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0)) {
				throw LoadingError("Unsupported vertex format");
			}

			const bool hasTangents = (vtxFormat & Graphics::ATTRIB_TANGENT);

			//vertex buffer
			// XXX evaluate whether we can use VertexBufferDesc::FromAttribSet here
			Graphics::VertexBufferDesc vbDesc;
			vbDesc.attrib[0].semantic = Graphics::ATTRIB_POSITION;
			vbDesc.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbDesc.attrib[1].semantic = Graphics::ATTRIB_NORMAL;
			vbDesc.attrib[1].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbDesc.attrib[2].semantic = Graphics::ATTRIB_UV0;
			vbDesc.attrib[2].format = Graphics::ATTRIB_FORMAT_FLOAT2;
			if (hasTangents) {
				vbDesc.attrib[3].semantic = Graphics::ATTRIB_TANGENT;
				vbDesc.attrib[3].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			}
			vbDesc.usage = Graphics::BUFFER_USAGE_STATIC;
			vbDesc.numVertices = db.rd->Int32();

			RefCountedPtr<Graphics::VertexBuffer> vtxBuffer(db.loader->GetRenderer()->CreateVertexBuffer(vbDesc));
			const Uint32 posOffset = vtxBuffer->GetDesc().GetOffset(Graphics::ATTRIB_POSITION);
			const Uint32 nrmOffset = vtxBuffer->GetDesc().GetOffset(Graphics::ATTRIB_NORMAL);
			const Uint32 uv0Offset = vtxBuffer->GetDesc().GetOffset(Graphics::ATTRIB_UV0);
			const Uint32 tanOffset = hasTangents ? vtxBuffer->GetDesc().GetOffset(Graphics::ATTRIB_TANGENT) : 0;
			const Uint32 stride = vtxBuffer->GetDesc().stride;
			Uint8 *vtxPtr = vtxBuffer->Map<Uint8>(BUFFER_MAP_WRITE);
			if (hasTangents) {
				for (Uint32 i = 0; i < vbDesc.numVertices; i++) {
					*reinterpret_cast<vector3f *>(vtxPtr + i * stride + posOffset) = db.rd->Vector3f();
					*reinterpret_cast<vector3f *>(vtxPtr + i * stride + nrmOffset) = db.rd->Vector3f();
					const float uvx = db.rd->Float();
					const float uvy = db.rd->Float();
					*reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset) = vector2f(uvx, uvy);
					*reinterpret_cast<vector3f *>(vtxPtr + i * stride + tanOffset) = db.rd->Vector3f();
				}
			} else {
				for (Uint32 i = 0; i < vbDesc.numVertices; i++) {
					*reinterpret_cast<vector3f *>(vtxPtr + i * stride + posOffset) = db.rd->Vector3f();
					*reinterpret_cast<vector3f *>(vtxPtr + i * stride + nrmOffset) = db.rd->Vector3f();
					const float uvx = db.rd->Float();
					const float uvy = db.rd->Float();
					*reinterpret_cast<vector2f *>(vtxPtr + i * stride + uv0Offset) = vector2f(uvx, uvy);
				}
			}
			vtxBuffer->Unmap();

			//index buffer
			const Uint32 numIndices = db.rd->Int32();
			RefCountedPtr<Graphics::IndexBuffer> idxBuffer(db.loader->GetRenderer()->CreateIndexBuffer(numIndices, Graphics::BUFFER_USAGE_STATIC));
			Uint32 *idxPtr = idxBuffer->Map(BUFFER_MAP_WRITE);
			for (Uint32 i = 0; i < numIndices; i++)
				idxPtr[i] = db.rd->Int32();
			idxBuffer->Unmap();

			sg->AddMesh(vtxBuffer, idxBuffer, material);
		}
		return sg;
	}

	void StaticGeometry::AddMesh(
		RefCountedPtr<Graphics::VertexBuffer> vb,
		RefCountedPtr<Graphics::IndexBuffer> ib,
		RefCountedPtr<Graphics::Material> mat)
	{
		Mesh m;
		m.vertexBuffer = vb;
		m.indexBuffer = ib;
		m.material = mat;
		m.meshObject.Reset(m_renderer->CreateMeshObject(vb.Get(), ib.Get()));
		m_meshes.push_back(m);
	}

	StaticGeometry::Mesh &StaticGeometry::GetMeshAt(unsigned int i)
	{
		return m_meshes.at(i);
	}

} // namespace SceneGraph
