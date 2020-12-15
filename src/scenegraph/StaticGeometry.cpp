// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
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
#include "utils.h"

namespace SceneGraph {

	StaticGeometry::StaticGeometry(Graphics::Renderer *r) :
		Node(r, NODE_SOLID),
		m_blendMode(Graphics::BLEND_SOLID),
		m_renderState(nullptr)
	{
	}

	StaticGeometry::~StaticGeometry()
	{
	}

	StaticGeometry::StaticGeometry(const StaticGeometry &sg, NodeCopyCache *cache) :
		Node(sg, cache),
		m_boundingBox(sg.m_boundingBox),
		m_blendMode(sg.m_blendMode),
		m_meshes(sg.m_meshes),
		m_renderState(sg.m_renderState)
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
		SDL_assert(m_renderState);
		Graphics::Renderer *r = GetRenderer();
		r->SetTransform(trans);
		for (auto &it : m_meshes)
			r->DrawBufferIndexed(it.vertexBuffer.Get(), it.indexBuffer.Get(), m_renderState, it.material.Get());

		//DrawBoundingBox(m_boundingBox);
	}

	void StaticGeometry::Render(const std::vector<matrix4x4f> &trans, const RenderData *rd)
	{
		PROFILE_SCOPED()
		SDL_assert(m_renderState);
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
				// create the "new" material with the instanced description
				RefCountedPtr<Graphics::Material> mat(r->CreateMaterial(mdesc));
				// copy over all of the other details
				mat->texture0 = it.material->texture0;
				mat->texture1 = it.material->texture1;
				mat->texture2 = it.material->texture2;
				mat->texture3 = it.material->texture3;
				mat->texture4 = it.material->texture4;
				mat->texture5 = it.material->texture5;
				mat->texture6 = it.material->texture5;
				mat->heatGradient = it.material->heatGradient;
				mat->diffuse = it.material->diffuse;
				mat->specular = it.material->specular;
				mat->emissive = it.material->emissive;
				mat->shininess = it.material->shininess;
				mat->specialParameter0 = it.material->specialParameter0;
				m_instanceMaterials.push_back(mat);
			}
		}

		// process each mesh
		int i = 0;
		for (auto &it : m_meshes) {
			// finally render using the instance material
			r->DrawBufferIndexedInstanced(it.vertexBuffer.Get(), it.indexBuffer.Get(), m_renderState, m_instanceMaterials[i].Get(), m_instBuffer.Get());
			++i;
		}
	}

	typedef std::vector<std::pair<std::string, RefCountedPtr<Graphics::Material>>> MaterialContainer;
	void StaticGeometry::Save(NodeDatabase &db)
	{
		PROFILE_SCOPED()
		Node::Save(db);
		db.wr->Int32(m_blendMode);
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

		sg->m_blendMode = static_cast<Graphics::BlendMode>(rd.Int32());
		sg->m_boundingBox.min = rd.Vector3d();
		sg->m_boundingBox.max = rd.Vector3d();

		Graphics::RenderStateDesc rsd;
		rsd.blendMode = sg->m_blendMode;
		rsd.depthWrite = rsd.blendMode == Graphics::BLEND_SOLID;
		sg->SetRenderState(sg->GetRenderer()->CreateRenderState(rsd));

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
		m_meshes.push_back(m);
	}

	StaticGeometry::Mesh &StaticGeometry::GetMeshAt(unsigned int i)
	{
		return m_meshes.at(i);
	}

	void StaticGeometry::DrawBoundingBox(const Aabb &bb)
	{
		const vector3f min(bb.min.x, bb.min.y, bb.min.z);
		const vector3f max(bb.max.x, bb.max.y, bb.max.z);
		const vector3f fbl(min.x, min.y, min.z); //front bottom left
		const vector3f fbr(max.x, min.y, min.z); //front bottom right
		const vector3f ftl(min.x, max.y, min.z); //front top left
		const vector3f ftr(max.x, max.y, min.z); //front top right
		const vector3f rtl(min.x, max.y, max.z); //rear top left
		const vector3f rtr(max.x, max.y, max.z); //rear top right
		const vector3f rbl(min.x, min.y, max.z); //rear bottom left
		const vector3f rbr(max.x, min.y, max.z); //rear bottom right

		const Color c(Color::WHITE);

		std::unique_ptr<Graphics::VertexArray> vts(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE));

		//Front face
		vts->Add(ftr, c); //3
		vts->Add(fbr, c); //1
		vts->Add(fbl, c); //0

		vts->Add(fbl, c); //0
		vts->Add(ftl, c); //2
		vts->Add(ftr, c); //3

		//Rear face
		vts->Add(rbr, c); //7
		vts->Add(rtr, c); //5
		vts->Add(rbl, c); //6

		vts->Add(rbl, c); //6
		vts->Add(rtr, c); //5
		vts->Add(rtl, c); //4

		//Top face
		vts->Add(rtl, c); //4
		vts->Add(rtr, c); //5
		vts->Add(ftr, c); //3

		vts->Add(ftr, c); //3
		vts->Add(ftl, c); //2
		vts->Add(rtl, c); //4

		//bottom face
		vts->Add(fbr, c); //1
		vts->Add(rbr, c); //7
		vts->Add(rbl, c); //6

		vts->Add(rbl, c); //6
		vts->Add(fbl, c); //0
		vts->Add(fbr, c); //1

		//left face
		vts->Add(fbl, c); //0
		vts->Add(rbl, c); //6
		vts->Add(rtl, c); //4

		vts->Add(rtl, c); //4
		vts->Add(ftl, c); //2
		vts->Add(fbl, c); //0

		//right face
		vts->Add(rtr, c); //5
		vts->Add(rbr, c); //7
		vts->Add(fbr, c); //1

		vts->Add(fbr, c); //1
		vts->Add(ftr, c); //3
		vts->Add(rtr, c); //5

		Graphics::Renderer *r = GetRenderer();

		Graphics::RenderStateDesc rsd;
		rsd.cullMode = Graphics::CULL_NONE;

		RefCountedPtr<Graphics::VertexBuffer> vb;
		//create buffer and upload data
		Graphics::VertexBufferDesc vbd;
		vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
		vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
		vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
		vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
		vbd.numVertices = vts->GetNumVerts();
		vbd.usage = Graphics::BUFFER_USAGE_STATIC;
		vb.Reset(m_renderer->CreateVertexBuffer(vbd));
		vb->Populate(*vts);

		r->SetWireFrameMode(true);
		r->DrawBuffer(vb.Get(), r->CreateRenderState(rsd), Graphics::vtxColorMaterial);
		r->SetWireFrameMode(false);
	}

} // namespace SceneGraph
