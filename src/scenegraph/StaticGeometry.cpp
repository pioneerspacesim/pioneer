// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "StaticGeometry.h"

#include "BaseLoader.h"
#include "Model.h"
#include "NodeVisitor.h"
#include "Serializer.h"
#include "core/StringUtils.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "scenegraph/BinaryConverter.h"

#include "profiler/Profiler.h"

#include <cstring>

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
		return new StaticGeometry(*this, cache); // geometries cannot be shared if material overriding is supported. See Shields.cpp
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

	void StaticGeometry::RenderInstanced(const std::vector<matrix4x4f> &trans, const RenderData *rd)
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
			PROFILE_SCOPED_DESC("Copy Instance Data")

			// Copy the transforms into the buffer
			for (const matrix4x4f &mt : trans) {
				(*pBuffer) = mt;
				++pBuffer;
			}
			ib->Unmap();
			ib->SetInstanceCount(numTrans);
		}

		if (m_instanceMaterials.empty()) {
			// process each mesh
			for (auto &it : m_meshes) {
				// Due to the shader needing to change we have to get the material and force it to the instanced variant
				Graphics::MaterialDescriptor mdesc = it.material->GetDescriptor();
				mdesc.instanced = true;

				const Graphics::RenderStateDesc oldDesc = r->GetMaterialRenderState(it.material.Get());

				// Set up the vertex format descriptor for instanced rendering
				// This ideally should be done significantly before now (at load time, probably)
				Graphics::VertexFormatDesc vtxFormat = it.meshObject->GetFormat();
				vtxFormat.attribs[vtxFormat.GetNumAttribs()] = { Graphics::ATTRIB_FORMAT_MAT4x4, 6, 1, 0 };
				vtxFormat.bindings[1] = { sizeof(matrix4x4f), true, Graphics::ATTRIB_RATE_INSTANCE };

				// create the "new" material with the instanced description
				RefCountedPtr<Graphics::Material> mat(r->CloneMaterial(it.material.Get(), mdesc, oldDesc, vtxFormat));
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
			db.wr->Int32(mesh.semantics);

			// write the number of vertices
			db.wr->Int32(mesh.vertexBuffer->GetSize());

			//save positions, normals, uvs, and optional tangents interleaved (only known format now)
			const uint8_t *vtxPtr = mesh.vertexBuffer->Map<uint8_t>(Graphics::BUFFER_MAP_READ);
			db.wr->Raw(vtxPtr, mesh.vertexBuffer->GetSize() * mesh.vertexBuffer->GetStride());
			mesh.vertexBuffer->Unmap();

			//indices
			const Uint32 numIndices = mesh.indexBuffer->GetSize();
			db.wr->Int32(numIndices);

			//directly blit index data to disk; note that this implies host-endian ordering
			const Uint32 *indexPtr = mesh.indexBuffer->Map(Graphics::BUFFER_MAP_READ);
			db.wr->Raw(reinterpret_cast<const uint8_t *>(indexPtr), numIndices * sizeof(Uint32));
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

			//vertex buffer
			Graphics::VertexFormatDesc desc = Graphics::VertexFormatDesc::FromAttribSet(vtxFormat);

			// Read the number of vertices in the mesh
			Uint32 numVertices = db.rd->Int32();
			RefCountedPtr<Graphics::VertexBuffer> vtxBuffer(db.loader->GetRenderer()->CreateVertexBuffer(Graphics::BUFFER_USAGE_STATIC, numVertices, desc.bindings[0].stride));

			// Copy the vertex data into the buffer; note this implies host-only endianness.
			uint8_t *vtxPtr = vtxBuffer->Map<uint8_t>(BUFFER_MAP_WRITE);
			size_t vtxSize = numVertices * vtxBuffer->GetStride();

			std::memcpy(vtxPtr, db.rd->Raw(vtxSize), vtxSize);
			vtxBuffer->Unmap();


			//index buffer
			const Uint32 numIndices = db.rd->Int32();
			RefCountedPtr<Graphics::IndexBuffer> idxBuffer(db.loader->GetRenderer()->CreateIndexBuffer(numIndices, Graphics::BUFFER_USAGE_STATIC));

			Uint32 *idxPtr = idxBuffer->Map(BUFFER_MAP_WRITE);
			size_t idxSize = numIndices * sizeof(Uint32);

			std::memcpy(idxPtr, db.rd->Raw(idxSize), idxSize);
			idxBuffer->Unmap();

			sg->AddMesh(vtxFormat, vtxBuffer, idxBuffer, material);
		}
		return sg;
	}

	void StaticGeometry::AddMesh(
		Graphics::AttributeSet semantics,
		RefCountedPtr<Graphics::VertexBuffer> vb,
		RefCountedPtr<Graphics::IndexBuffer> ib,
		RefCountedPtr<Graphics::Material> mat)
	{
		Mesh m;
		m.vertexBuffer = vb;
		m.indexBuffer = ib;
		m.material = mat;
		m.meshObject.Reset(m_renderer->CreateMeshObject(Graphics::VertexFormatDesc::FromAttribSet(semantics), vb.Get(), ib.Get()));
		m.semantics = semantics;
		m_meshes.push_back(m);
	}

	StaticGeometry::Mesh &StaticGeometry::GetMeshAt(unsigned int i)
	{
		return m_meshes.at(i);
	}

} // namespace SceneGraph
