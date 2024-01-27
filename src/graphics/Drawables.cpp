// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Drawables.h"

#include "Aabb.h"
#include "MathUtil.h"
#include "Texture.h"
#include "TextureBuilder.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Types.h"
#include "graphics/VertexBuffer.h"
#include "profiler/Profiler.h"

namespace Graphics {

	namespace Drawables {

		/*
		Circle::Circle(Renderer *renderer, const float radius, const Color &c, RenderState *state) :
			m_color(c)
		{
			PROFILE_SCOPED()
			m_renderState = state;
			VertexArray vertices(ATTRIB_POSITION);
			for (float theta = 0; theta < 2 * float(M_PI); theta += 0.05f * float(M_PI)) {
				vertices.Add(vector3f(radius * sin(theta), radius * cos(theta), 0));
			}
			SetupVertexBuffer(vertices, renderer);
		}
		Circle::Circle(Renderer *renderer, const float radius, const float x, const float y, const float z, const Color &c, RenderState *state) :
			m_color(c)
		{
			PROFILE_SCOPED()
			m_renderState = state;
			VertexArray vertices(ATTRIB_POSITION);
			for (float theta = 0; theta < 2 * float(M_PI); theta += 0.05f * float(M_PI)) {
				vertices.Add(vector3f(radius * sin(theta) + x, radius * cos(theta) + y, z));
			}
			SetupVertexBuffer(vertices, renderer);
		}
		Circle::Circle(Renderer *renderer, const float radius, const vector3f &center, const Color &c, RenderState *state) :
			m_color(c)
		{
			PROFILE_SCOPED()
			m_renderState = state;
			VertexArray vertices(ATTRIB_POSITION);
			for (float theta = 0; theta < 2 * float(M_PI); theta += 0.05f * float(M_PI)) {
				vertices.Add(vector3f(radius * sin(theta) + center.x, radius * cos(theta) + center.y, center.z));
			}
			SetupVertexBuffer(vertices, renderer);
		}

		void Circle::Draw(Renderer *renderer)
		{
			PROFILE_SCOPED()
			m_material->diffuse = m_color;
			renderer->DrawBuffer(m_vertexBuffer.Get(), m_renderState, m_material.Get(), PrimitiveType::LINE_LOOP);
		}

		void Circle::SetupVertexBuffer(const Graphics::VertexArray &vertices, Graphics::Renderer *r)
		{
			PROFILE_SCOPED()
			MaterialDescriptor desc;
			m_material.Reset(r->CreateMaterial(desc));

			//Create vtx & index buffers and copy data
			VertexBufferDesc vbd;
			vbd.attrib[0].semantic = ATTRIB_POSITION;
			vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
			vbd.numVertices = vertices.GetNumVerts();
			vbd.usage = BUFFER_USAGE_STATIC;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			m_vertexBuffer->Populate(vertices);
		}
		*/

		//------------------------------------------------------------

		Disk::Disk(Graphics::Renderer *r, const int edges, const float rad)
		{
			PROFILE_SCOPED()

			VertexArray vertices(ATTRIB_POSITION | ATTRIB_UV0);

			vertices.Add(vector3f(0.f, 0.f, 0.f), vector2(0.5f, 0.5f));
			const float edgeStep = DEG2RAD(360.0f) / float(edges);
			for (int i = edges; i >= 0; i--) {
				float x = sinf(i * edgeStep);
				float y = cosf(i * edgeStep);
				vertices.Add(
					vector3f(x * rad, y * rad, 0.f),
					vector2f(0.5f + x * 0.5f, 0.5f + y * 0.5f));
			}

			m_diskMesh.reset(r->CreateMeshObjectFromArray(&vertices));
		}

		void Disk::Draw(Renderer *r, Material *mat)
		{
			PROFILE_SCOPED()
			r->DrawMesh(m_diskMesh.get(), mat);
		}

		//------------------------------------------------------------

		/*
		Line3D::Line3D() :
			m_refreshVertexBuffer(true),
			m_width(2.0f),
			m_va(new Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE, 2))
		{
			PROFILE_SCOPED()
			assert(m_va.get());
			// XXX bug in Radeon drivers will cause crash in glLineWidth if width >= 3
			m_va->Add(vector3f(0.f), Color::BLANK);
			m_va->Add(vector3f(0.f), Color::WHITE);
		}

		Line3D::Line3D(const Line3D &b) :
			Line3D()
		{
			PROFILE_SCOPED()
			m_refreshVertexBuffer = (b.m_refreshVertexBuffer);
			m_width = (b.m_width);
			m_material = (b.m_material);
			m_vertexBuffer = (b.m_vertexBuffer);
			(*m_va.get()) = (*b.m_va.get());
		}

		void Line3D::SetStart(const vector3f &s)
		{
			PROFILE_SCOPED()
			if (!m_va->position[0].ExactlyEqual(s)) {
				m_va->Set(0, s);
				Dirty();
			}
		}

		void Line3D::SetEnd(const vector3f &e)
		{
			PROFILE_SCOPED()
			if (!m_va->position[1].ExactlyEqual(e)) {
				m_va->Set(1, e);
				Dirty();
			}
		}

		void Line3D::SetColor(const Color &c)
		{
			PROFILE_SCOPED()
			if (!(m_va->diffuse[0] == c)) {
				m_va->Set(0, m_va->position[0], c);
				m_va->Set(1, m_va->position[1], c * 0.5);
				Dirty();
			}
		}

		void Line3D::Draw(Renderer *r, RenderState *rs)
		{
			PROFILE_SCOPED()
			if (m_va->GetNumVerts() == 0)
				return;

			if (!m_vertexBuffer.Valid()) {
				CreateVertexBuffer(r, 2);
			}
			if (m_refreshVertexBuffer) {
				assert(m_va.get());
				m_refreshVertexBuffer = false;
				m_vertexBuffer->Populate(*m_va);
			}
			// XXX would be nicer to draw this as a textured triangle strip
			// can't guarantee linewidth support
			// glLineWidth(m_width);
			r->DrawBuffer(m_vertexBuffer.Get(), rs, m_material.Get(), Graphics::LINE_SINGLE);
			// glLineWidth(1.f);
		}

		void Line3D::CreateVertexBuffer(Graphics::Renderer *r, const Uint32 size)
		{
			PROFILE_SCOPED()
			Graphics::MaterialDescriptor desc;
			desc.vertexColors = true;
			m_material.Reset(r->CreateMaterial(desc));

			Graphics::VertexBufferDesc vbd;
			vbd.attrib[0].semantic = Graphics::ATTRIB_POSITION;
			vbd.attrib[0].format = Graphics::ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = Graphics::ATTRIB_DIFFUSE;
			vbd.attrib[1].format = Graphics::ATTRIB_FORMAT_UBYTE4;
			vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;
			vbd.numVertices = size;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			assert(m_vertexBuffer.Valid());
		}

		void Line3D::Dirty()
		{
			m_refreshVertexBuffer = true;
		}
		*/

		//------------------------------------------------------------

		Lines::Lines() :
			m_refreshVertexBuffer(true),
			m_va(new VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE))
		{
			PROFILE_SCOPED()
			// XXX bug in Radeon drivers will cause crash in glLineWidth if width >= 3
		}

		void Lines::SetData(const Uint32 vertCount, const vector3f *vertices, const Color &color)
		{
			PROFILE_SCOPED()
			assert(vertices);

			// somethings changed so even if the number of verts is constant the data must be uploaded
			m_refreshVertexBuffer = true;

			// if the number of vert mismatches then clear the current vertex buffer
			if (m_lineMesh.Valid() && m_lineMesh->GetVertexBuffer()->GetCapacity() < vertCount) {
				// a new one will be created when it is drawn
				m_lineMesh.Reset();
			}

			// populate the VertexArray
			m_va->Clear(vertCount);
			for (Uint32 i = 0; i < vertCount; i++) {
				m_va->Add(vertices[i], color);
			}
		}

		void Lines::SetData(const Uint32 vertCount, const vector3f *vertices, const Color *colors)
		{
			PROFILE_SCOPED()
			assert(vertices);

			// somethings changed so even if the number of verts is constant the data must be uploaded
			m_refreshVertexBuffer = true;

			// if the number of vert mismatches then clear the current vertex buffer
			if (m_lineMesh.Valid() && m_lineMesh->GetVertexBuffer()->GetCapacity() < vertCount) {
				// a new one will be created when it is drawn
				m_lineMesh.Reset();
			}

			// populate the VertexArray
			m_va->Clear(vertCount);
			for (Uint32 i = 0; i < vertCount; i++) {
				m_va->Add(vertices[i], colors[i]);
			}
		}

		void Lines::Draw(Renderer *r, Material *mat)
		{
			PROFILE_SCOPED()
			if (m_va->IsEmpty())
				return;

			/*
			if (!m_lineMesh.Valid()) {
				Graphics::VertexBufferDesc vbd = VertexBufferDesc::FromAttribSet(m_va->GetAttributeSet());
				vbd.usage = Graphics::BUFFER_USAGE_DYNAMIC;
				vbd.numVertices = m_va->GetNumVerts() * 2; // ask for twice as many as we need to reduce buffer thrashing
				m_lineMesh.Reset(r->CreateMeshObject(r->CreateVertexBuffer(vbd)));
				m_refreshVertexBuffer = true;
			}

			if (m_refreshVertexBuffer) {
				m_refreshVertexBuffer = false;
				m_lineMesh->GetVertexBuffer()->Populate(*m_va);
			}
			*/

			// XXX would be nicer to draw this as a textured triangle strip
			// can't guarantee linewidth support
			// glLineWidth(m_width);
			// r->DrawMesh(m_lineMesh.Get(), mat);
			r->DrawBuffer(m_va.get(), mat);
			// glLineWidth(1.f);
		}

		//------------------------------------------------------------
		PointSprites::PointSprites() :
			m_refreshVertexBuffer(true),
			m_va(new VertexArray(ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_DIFFUSE))
		{
		}

		void PointSprites::SetData(const int count, const vector3f *positions, const Color *colours, const float *sizes)
		{
			PROFILE_SCOPED()
			if (count < 1)
				return;

			assert(positions);

			m_va->Clear(count);

			for (int i = 0; i < count; i++) {
				vector3f vSize(sizes[i]);
				m_va->Add(positions[i], colours[i], vSize);
			}

			m_refreshVertexBuffer = true;
		}

		void PointSprites::SetData(const int count, std::vector<vector3f> &&positions, std::vector<Color> &&colors, std::vector<float> &&sizes)
		{
			PROFILE_SCOPED();
			if (count < 1)
				return;

			m_va->Clear();
			m_va->position = std::move(positions);
			m_va->diffuse = std::move(colors);

			m_va->normal.reserve(count);
			for (int i = 0; i < count; i++) {
				vector3f vSize(sizes[i]);
				m_va->normal.push_back(vSize);
			}
		}

		void PointSprites::Draw(Renderer *r, Material *mat)
		{
			PROFILE_SCOPED()
			assert(r->GetMaterialRenderState(mat).primitiveType == Graphics::POINTS);

			if (m_va->GetNumVerts() == 0)
				return;

			if (!m_pointData.Valid()) {
				m_pointData.Reset(r->CreateMeshObjectFromArray(m_va.get()));
				m_refreshVertexBuffer = false;
			}

			if (m_refreshVertexBuffer) {
				m_refreshVertexBuffer = false;
				m_pointData->GetVertexBuffer()->Populate(*m_va);
			}

			r->DrawMesh(m_pointData.Get(), mat);
		}

		//------------------------------------------------------------

		Points::Points() :
			m_refreshVertexBuffer(true),
			m_va(new VertexArray(ATTRIB_POSITION | ATTRIB_DIFFUSE))
		{
			PROFILE_SCOPED()
		}

		void Points::SetData(Renderer *r, const int count, const vector3f *positions, const matrix4x4f &trans, const Color &color, const float size)
		{
			PROFILE_SCOPED()
			if (count < 1)
				return;

			assert(positions);
			const unsigned int total = (count * 6);

			m_va->Clear(total);

			matrix4x4f rot(trans);
			rot.ClearToRotOnly();
			rot = rot.Inverse();

			const float sz = 0.5f * size;
			const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
			const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
			const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
			const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);

			//do two-triangle quads. Could also do indexed surfaces.
			//PiGL renderer should use actual point sprites
			//(see history of Render.cpp for point code remnants)
			for (int i = 0; i < count; i++) {
				const vector3f &pos = positions[i];

				m_va->Add(pos + rotv4, color); //top left
				m_va->Add(pos + rotv3, color); //bottom left
				m_va->Add(pos + rotv1, color); //top right

				m_va->Add(pos + rotv1, color); //top right
				m_va->Add(pos + rotv3, color); //bottom left
				m_va->Add(pos + rotv2, color); //bottom right
			}

			// if the number of vert mismatches then clear the current vertex buffer
			if (m_pointMesh.Valid() && m_pointMesh->GetVertexBuffer()->GetCapacity() < total) {
				// a new one will be created when it is drawn
				m_pointMesh.Reset();
			}

			m_refreshVertexBuffer = true;
		}

		void Points::SetData(Renderer *r, const int count, const vector3f *positions, const Color *color, const matrix4x4f &trans, const float size)
		{
			PROFILE_SCOPED()
			if (count < 1)
				return;

			assert(positions);
			const unsigned int total = (count * 6);

			m_va->Clear(total);

			matrix4x4f rot(trans);
			rot.ClearToRotOnly();
			rot = rot.Inverse();

			const float sz = 0.5f * size;
			const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
			const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);

			//do two-triangle quads. Could also do indexed surfaces.
			//PiGL renderer should use actual point sprites
			//(see history of Render.cpp for point code remnants)
			for (int i = 0; i < count; i++) {
				const vector3f &pos = positions[i];

				m_va->Add(pos - rotv2, color[i]); //top left
				m_va->Add(pos - rotv1, color[i]); //bottom left
				m_va->Add(pos + rotv1, color[i]); //top right

				m_va->Add(pos + rotv1, color[i]); //top right
				m_va->Add(pos - rotv1, color[i]); //bottom left
				m_va->Add(pos + rotv2, color[i]); //bottom right
			}

			// if the number of vert mismatches then clear the current vertex buffer
			if (m_pointMesh.Valid() && m_pointMesh->GetVertexBuffer()->GetCapacity() < total) {
				// a new one will be created when it is drawn
				m_pointMesh.Reset();
			}

			m_refreshVertexBuffer = true;
		}

		void Points::Draw(Renderer *r, Material *mat)
		{
			PROFILE_SCOPED()
			if (m_va->GetNumVerts() == 0)
				return;

			if (!m_pointMesh.Valid()) {
				m_pointMesh.Reset(r->CreateMeshObjectFromArray(m_va.get()));
				// the vertex buffer will have already been populated during creation
				m_refreshVertexBuffer = false;
			}

			if (m_refreshVertexBuffer) {
				m_refreshVertexBuffer = false;
				m_pointMesh->GetVertexBuffer()->Populate(*m_va);
			}

			r->DrawMesh(m_pointMesh.Get(), mat);
		}

		//------------------------------------------------------------

		static const float ICOSX = 0.525731112119133f;
		static const float ICOSZ = 0.850650808352039f;

		static const vector3f icosahedron_vertices[12] = {
			vector3f(-ICOSX, 0.0, ICOSZ), vector3f(ICOSX, 0.0, ICOSZ), vector3f(-ICOSX, 0.0, -ICOSZ), vector3f(ICOSX, 0.0, -ICOSZ),
			vector3f(0.0, ICOSZ, ICOSX), vector3f(0.0, ICOSZ, -ICOSX), vector3f(0.0, -ICOSZ, ICOSX), vector3f(0.0, -ICOSZ, -ICOSX),
			vector3f(ICOSZ, ICOSX, 0.0), vector3f(-ICOSZ, ICOSX, 0.0), vector3f(ICOSZ, -ICOSX, 0.0), vector3f(-ICOSZ, -ICOSX, 0.0)
		};

		static const int icosahedron_faces[20][3] = {
			{ 0, 4, 1 }, { 0, 9, 4 }, { 9, 5, 4 }, { 4, 5, 8 }, { 4, 8, 1 },
			{ 8, 10, 1 }, { 8, 3, 10 }, { 5, 3, 8 }, { 5, 2, 3 }, { 2, 7, 3 },
			{ 7, 10, 3 }, { 7, 6, 10 }, { 7, 11, 6 }, { 11, 0, 6 }, { 0, 1, 6 },
			{ 6, 1, 10 }, { 9, 0, 11 }, { 9, 11, 2 }, { 9, 2, 5 }, { 7, 2, 11 }
		};

		Graphics::MeshObject *Icosphere::Generate(Graphics::Renderer *r, int subdivs, float scale, AttributeSet attribs)
		{
			subdivs = Clamp(subdivs, 0, 10);
			scale = fabs(scale);
			matrix4x4f trans = matrix4x4f::Identity();
			trans.Scale(scale, scale, scale);

			//reserve some data - ATTRIB_POSITION | ATTRIB_NORMAL | ATTRIB_UV0
			VertexArray vts(attribs, (subdivs * subdivs) * 20 * 3);
			std::vector<Uint32> indices;

			//initial vertices
			int vi[12];
			for (int i = 0; i < 12; i++) {
				const vector3f &v = icosahedron_vertices[i];
				vi[i] = AddVertex(vts, trans * v, v);
			}

			//subdivide
			for (int i = 0; i < 20; i++) {
				Subdivide(vts, indices, trans, icosahedron_vertices[icosahedron_faces[i][0]],
					icosahedron_vertices[icosahedron_faces[i][1]],
					icosahedron_vertices[icosahedron_faces[i][2]],
					vi[icosahedron_faces[i][0]],
					vi[icosahedron_faces[i][1]],
					vi[icosahedron_faces[i][2]],
					subdivs);
			}

			//Create vtx & index buffers and copy data
			Graphics::IndexBuffer *indexBuffer = r->CreateIndexBuffer(indices.size(), BUFFER_USAGE_STATIC);
			Uint32 *idxPtr = indexBuffer->Map(Graphics::BUFFER_MAP_WRITE);
			for (auto it : indices) {
				*idxPtr = it;
				idxPtr++;
			}
			indexBuffer->Unmap();

			return r->CreateMeshObjectFromArray(&vts, indexBuffer);
		}

		int Icosphere::AddVertex(VertexArray &vts, const vector3f &v, const vector3f &n)
		{
			PROFILE_SCOPED()
			vts.position.push_back(v);
			if (vts.HasAttrib(ATTRIB_NORMAL)) {
				vts.normal.push_back(n);
			}
			if (vts.HasAttrib(ATTRIB_UV0)) {
				//http://www.mvps.org/directx/articles/spheremap.htm
				vts.uv0.push_back(vector2f(asinf(n.x) / M_PI + 0.5f, asinf(n.y) / M_PI + 0.5f));
			}
			return vts.GetNumVerts() - 1;
		}

		void Icosphere::AddTriangle(std::vector<Uint32> &indices, int i1, int i2, int i3)
		{
			PROFILE_SCOPED()
			indices.push_back(i1);
			indices.push_back(i2);
			indices.push_back(i3);
		}

		void Icosphere::Subdivide(VertexArray &vts, std::vector<Uint32> &indices,
			const matrix4x4f &trans, const vector3f &v1, const vector3f &v2, const vector3f &v3,
			const int i1, const int i2, const int i3, int depth)
		{
			PROFILE_SCOPED()
			if (depth == 0) {
				AddTriangle(indices, i1, i3, i2);
				return;
			}

			const vector3f v12 = (v1 + v2).Normalized();
			const vector3f v23 = (v2 + v3).Normalized();
			const vector3f v31 = (v3 + v1).Normalized();
			const int i12 = AddVertex(vts, trans * v12, v12);
			const int i23 = AddVertex(vts, trans * v23, v23);
			const int i31 = AddVertex(vts, trans * v31, v31);
			Subdivide(vts, indices, trans, v1, v12, v31, i1, i12, i31, depth - 1);
			Subdivide(vts, indices, trans, v2, v23, v12, i2, i23, i12, depth - 1);
			Subdivide(vts, indices, trans, v3, v31, v23, i3, i31, i23, depth - 1);
			Subdivide(vts, indices, trans, v12, v23, v31, i12, i23, i31, depth - 1);
		}

		//------------------------------------------------------------

		Sphere3D::Sphere3D(Renderer *renderer, int subdivs, float scale, AttributeSet attribs)
		{
			PROFILE_SCOPED()
			assert(attribs.HasAttrib(ATTRIB_POSITION));

			m_sphereMesh.reset(Icosphere::Generate(renderer, subdivs, scale, attribs));
		}

		void Sphere3D::Draw(Renderer *r, Material *mat)
		{
			PROFILE_SCOPED()
			r->DrawMesh(m_sphereMesh.get(), mat);
		}

		//------------------------------------------------------------

		/*
		TexturedQuad::TexturedQuad(Graphics::Renderer *r, const std::string &filename)
		{
			PROFILE_SCOPED()

			Graphics::TextureBuilder texbuilder = Graphics::TextureBuilder::UI(filename);
			m_texture.Reset(texbuilder.GetOrCreateTexture(r, "ui"));
			Graphics::RenderStateDesc rsd;
			rsd.blendMode = Graphics::BLEND_ALPHA;
			rsd.depthTest = false;
			rsd.depthWrite = false;
			m_renderState = r->CreateRenderState(rsd);

			VertexArray vertices(ATTRIB_POSITION | ATTRIB_UV0);
			Graphics::MaterialDescriptor desc;
			desc.effect = Graphics::EFFECT_DEFAULT;
			desc.textures = 1;
			desc.lighting = false;
			desc.vertexColors = false;
			m_material.Reset(r->CreateMaterial(desc));
			m_material->texture0 = m_texture.Get();

			// these might need to be reversed
			const vector2f texSize = m_texture->GetDescriptor().texSize;
			const vector2f halfsz = 0.5f * m_texture->GetDescriptor().GetOriginalSize();

			vertices.Add(vector3f(-halfsz.x, -halfsz.y, 0.0f), vector2f(0.0f, texSize.y));
			vertices.Add(vector3f(-halfsz.x, halfsz.y, 0.0f), vector2f(0.0f, 0.0f));
			vertices.Add(vector3f(halfsz.x, -halfsz.y, 0.0f), vector2f(texSize.x, texSize.y));
			vertices.Add(vector3f(halfsz.x, halfsz.y, 0.0f), vector2f(texSize.x, 0.0f));

			//Create vtx & index buffers and copy data
			VertexBufferDesc vbd;
			vbd.attrib[0].semantic = ATTRIB_POSITION;
			vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = ATTRIB_UV0;
			vbd.attrib[1].format = ATTRIB_FORMAT_FLOAT2;
			vbd.numVertices = vertices.GetNumVerts();
			vbd.usage = BUFFER_USAGE_STATIC;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			m_vertexBuffer->Populate(vertices);
		}

		// a textured quad with reversed winding
		TexturedQuad::TexturedQuad(Graphics::Renderer *r, Graphics::Texture *texture, const vector2f &pos, const vector2f &size, RenderState *state) :
			m_texture(RefCountedPtr<Graphics::Texture>(texture))
		{
			PROFILE_SCOPED()
			assert(state);
			m_renderState = state;

			VertexArray vertices(ATTRIB_POSITION | ATTRIB_UV0);
			Graphics::MaterialDescriptor desc;
			desc.textures = 1;
			m_material.Reset(r->CreateMaterial(desc));
			m_material->texture0 = m_texture.Get();

			// these might need to be reversed
			const vector2f texPos = vector2f(0.0f);
			const vector2f texSize = m_texture->GetDescriptor().texSize;

			vertices.Add(vector3f(pos.x, pos.y, 0.0f), vector2f(texPos.x, texPos.y + texSize.y));
			vertices.Add(vector3f(pos.x, pos.y + size.y, 0.0f), vector2f(texPos.x, texPos.y));
			vertices.Add(vector3f(pos.x + size.x, pos.y, 0.0f), vector2f(texPos.x + texSize.x, texPos.y + texSize.y));
			vertices.Add(vector3f(pos.x + size.x, pos.y + size.y, 0.0f), vector2f(texPos.x + texSize.x, texPos.y));

			//Create vtx & index buffers and copy data
			VertexBufferDesc vbd;
			vbd.attrib[0].semantic = ATTRIB_POSITION;
			vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = ATTRIB_UV0;
			vbd.attrib[1].format = ATTRIB_FORMAT_FLOAT2;
			vbd.numVertices = vertices.GetNumVerts();
			vbd.usage = BUFFER_USAGE_STATIC;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			m_vertexBuffer->Populate(vertices);
		}

		TexturedQuad::TexturedQuad(Graphics::Renderer *r, RefCountedPtr<Graphics::Material> &material, const Graphics::VertexArray &va, RenderState *state) :
			m_material(material)
		{
			PROFILE_SCOPED()
			assert(state);
			m_renderState = state;

			//Create vtx & index buffers and copy data
			VertexBufferDesc vbd;

			Uint32 attribIdx = 0;
			assert(va.HasAttrib(ATTRIB_POSITION));
			vbd.attrib[attribIdx].semantic = ATTRIB_POSITION;
			vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
			++attribIdx;

			if (va.HasAttrib(ATTRIB_NORMAL)) {
				vbd.attrib[attribIdx].semantic = ATTRIB_NORMAL;
				vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
				++attribIdx;
			}
			if (va.HasAttrib(ATTRIB_DIFFUSE)) {
				vbd.attrib[attribIdx].semantic = ATTRIB_DIFFUSE;
				vbd.attrib[attribIdx].format = ATTRIB_FORMAT_UBYTE4;
				++attribIdx;
			}
			if (va.HasAttrib(ATTRIB_UV0)) {
				vbd.attrib[attribIdx].semantic = ATTRIB_UV0;
				vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT2;
				++attribIdx;
			}
			if (va.HasAttrib(ATTRIB_TANGENT)) {
				vbd.attrib[attribIdx].semantic = ATTRIB_TANGENT;
				vbd.attrib[attribIdx].format = ATTRIB_FORMAT_FLOAT3;
				++attribIdx;
			}

			vbd.numVertices = va.GetNumVerts();
			vbd.usage = BUFFER_USAGE_STATIC;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			m_vertexBuffer->Populate(va);
		}

		void TexturedQuad::Draw(Graphics::Renderer *r)
		{
			PROFILE_SCOPED()
			m_material->diffuse = Color::WHITE;
			r->DrawBuffer(m_vertexBuffer.Get(), m_renderState, m_material.Get(), TRIANGLE_STRIP);
		}

		void TexturedQuad::Draw(Graphics::Renderer *r, const Color4ub &tint)
		{
			PROFILE_SCOPED()
			m_material->diffuse = tint;
			r->DrawBuffer(m_vertexBuffer.Get(), m_renderState, m_material.Get(), TRIANGLE_STRIP);
		}
		*/

		//------------------------------------------------------------

		/*
		Rect::Rect(Graphics::Renderer *r, const vector2f &pos, const vector2f &size, const Color &c, RenderState *state, const bool bIsStatic) :
			m_renderState(state)
		{
			PROFILE_SCOPED()

			using namespace Graphics;
			VertexArray bgArr(ATTRIB_POSITION | ATTRIB_DIFFUSE, 4);
			Graphics::MaterialDescriptor desc;
			desc.vertexColors = true;
			m_material.Reset(r->CreateMaterial(desc));

			bgArr.Add(vector3f(pos.x, size.y, 0), c);
			bgArr.Add(vector3f(size.x, size.y, 0), c);
			bgArr.Add(vector3f(size.x, pos.y, 0), c);
			bgArr.Add(vector3f(pos.x, pos.y, 0), c);

			VertexBufferDesc vbd;
			vbd.attrib[0].semantic = ATTRIB_POSITION;
			vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = ATTRIB_DIFFUSE;
			vbd.attrib[1].format = ATTRIB_FORMAT_UBYTE4;
			vbd.numVertices = 4;
			vbd.usage = bIsStatic ? BUFFER_USAGE_STATIC : BUFFER_USAGE_DYNAMIC;

			// VertexBuffer
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));
			m_vertexBuffer->Populate(bgArr);
		}

		void Rect::Update(const vector2f &pos, const vector2f &size, const Color &c)
		{
			using namespace Graphics;
			VertexArray bgArr(ATTRIB_POSITION | ATTRIB_DIFFUSE, 4);

			bgArr.Add(vector3f(pos.x, size.y, 0), c);
			bgArr.Add(vector3f(size.x, size.y, 0), c);
			bgArr.Add(vector3f(size.x, pos.y, 0), c);
			bgArr.Add(vector3f(pos.x, pos.y, 0), c);

			m_vertexBuffer->Populate(bgArr);
		}

		void Rect::Draw(Graphics::Renderer *r)
		{
			PROFILE_SCOPED()
			r->DrawBuffer(m_vertexBuffer.Get(), m_renderState, m_material.Get(), TRIANGLE_FAN);
		}
		*/

		//------------------------------------------------------------

		/*
		RoundEdgedRect::RoundEdgedRect(Graphics::Renderer *r, const vector2f &size, const float rad, const Color &c, RenderState *state, const bool bIsStatic) :
			m_renderState(state)
		{
			PROFILE_SCOPED()

			using namespace Graphics;
			Graphics::MaterialDescriptor desc;
			desc.vertexColors = true;
			m_material.Reset(r->CreateMaterial(desc));

			VertexBufferDesc vbd;
			vbd.attrib[0].semantic = ATTRIB_POSITION;
			vbd.attrib[0].format = ATTRIB_FORMAT_FLOAT3;
			vbd.attrib[1].semantic = ATTRIB_DIFFUSE;
			vbd.attrib[1].format = ATTRIB_FORMAT_UBYTE4;
			vbd.numVertices = 4 * (STEPS + 1);
			vbd.usage = bIsStatic ? BUFFER_USAGE_STATIC : BUFFER_USAGE_DYNAMIC;
			m_vertexBuffer.Reset(r->CreateVertexBuffer(vbd));

			Update(size, rad, c);
		}

		void RoundEdgedRect::Update(const vector2f &size, float rad, const Color &c)
		{
			Graphics::VertexArray vts(Graphics::ATTRIB_POSITION | ATTRIB_DIFFUSE);

			if (rad > 0.5f * std::min(size.x, size.y))
				rad = 0.5f * std::min(size.x, size.y);

			// top left
			// bottom left
			for (int i = 0; i <= STEPS; i++) {
				float ang = M_PI * 0.5f * i / float(STEPS);
				vts.Add(vector3f(rad - rad * cos(ang), (size.y - rad) + rad * sin(ang), 0.f), c);
			}
			// bottom right
			for (int i = 0; i <= STEPS; i++) {
				float ang = M_PI * 0.5 + M_PI * 0.5f * i / float(STEPS);
				vts.Add(vector3f(size.x - rad - rad * cos(ang), (size.y - rad) + rad * sin(ang), 0.f), c);
			}
			// top right
			for (int i = 0; i <= STEPS; i++) {
				float ang = M_PI + M_PI * 0.5f * i / float(STEPS);
				vts.Add(vector3f((size.x - rad) - rad * cos(ang), rad + rad * sin(ang), 0.f), c);
			}

			// top right
			for (int i = 0; i <= STEPS; i++) {
				float ang = M_PI * 1.5 + M_PI * 0.5f * i / float(STEPS);
				vts.Add(vector3f(rad - rad * cos(ang), rad + rad * sin(ang), 0.f), c);
			}

			m_vertexBuffer->Populate(vts);
		}

		void RoundEdgedRect::Draw(Graphics::Renderer *r)
		{
			PROFILE_SCOPED()
			r->DrawBuffer(m_vertexBuffer.Get(), m_renderState, m_material.Get(), TRIANGLE_FAN);
		}
		*/

		//------------------------------------------------------------

		struct GridLines::GridData {
			Color4f thin_color;
			Color4f thick_color;

			vector3f grid_dir;
			float cell_size;

			vector2f grid_size;
			float line_width;
		};

		GridLines::GridLines(Graphics::Renderer *r) :
			m_minorColor(Color(160, 160, 160)),
			m_majorColor(Color(255, 255, 255)),
			m_lineWidth(2.0f)
		{
			Graphics::RenderStateDesc rsd = {};
			rsd.blendMode = Graphics::BLEND_ALPHA;
			rsd.cullMode = Graphics::CULL_NONE;
			rsd.depthWrite = false;

			m_gridMat.reset(r->CreateMaterial("grid", {}, rsd));
		}

		void GridLines::SetLineColors(Color minorLineColor, Color majorLineColor, float lineWidth)
		{
			m_minorColor = minorLineColor;
			m_majorColor = majorLineColor;
			m_lineWidth = std::min(lineWidth, 1.0f);
		}

		void GridLines::Draw(Graphics::Renderer *r, vector2f grid_size, float cell_size)
		{
			Graphics::VertexArray va = Graphics::VertexArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);

			// Generate two triangles for the grid plane
			va.Add(vector3f(grid_size.x, 0, -grid_size.y), vector2f(grid_size.x, -grid_size.y));
			va.Add(vector3f(-grid_size.x, 0, grid_size.y), vector2f(-grid_size.x, grid_size.y));
			va.Add(vector3f(-grid_size.x, 0, -grid_size.y), vector2f(-grid_size.x, -grid_size.y));
			va.Add(vector3f(grid_size.x, 0, -grid_size.y), vector2f(grid_size.x, -grid_size.y));
			va.Add(vector3f(grid_size.x, 0, grid_size.y), vector2f(grid_size.x, grid_size.y));
			va.Add(vector3f(-grid_size.x, 0, grid_size.y), vector2f(-grid_size.x, grid_size.y));

			GridData data = {};
			data.thin_color = m_minorColor.ToColor4f();
			data.thick_color = m_majorColor.ToColor4f();
			data.grid_dir = r->GetTransform().ApplyRotationOnly(vector3f(0, 1.0, 0));
			data.cell_size = cell_size;
			data.grid_size = grid_size;
			data.line_width = m_lineWidth;

			m_gridMat->SetBufferDynamic("GridData"_hash, &data);
			r->DrawBuffer(&va, m_gridMat.get());
		}

		//------------------------------------------------------------

		struct GridSphere::GridData {
			Color4f thin_color;
			Color4f thick_color;
			float line_spacing;
			float line_width;
		};

		GridSphere::GridSphere(Graphics::Renderer *r, uint32_t num_subdivs) :
			m_minorColor(Color(160, 160, 160)),
			m_majorColor(Color(255, 255, 255)),
			m_lineWidth(2.0f),
			m_numSubdivs(num_subdivs)
		{
			Graphics::RenderStateDesc rsd = {};
			rsd.blendMode = Graphics::BLEND_ALPHA;
			rsd.cullMode = Graphics::CULL_NONE;
			rsd.depthWrite = false;

			m_gridMat.reset(r->CreateMaterial("gridsphere", {}, rsd));
		}

		void GridSphere::SetLineColors(Color minorLineColor, Color majorLineColor, float lineWidth)
		{
			m_minorColor = minorLineColor;
			m_majorColor = majorLineColor;
			m_lineWidth = std::min(lineWidth, 1.0f);
		}

		void GridSphere::Draw(Graphics::Renderer *r, float lineSpacing)
		{
			if (!m_sphereMesh) {
				m_sphereMesh.reset(Icosphere::Generate(r, m_numSubdivs, 1.f, Graphics::ATTRIB_POSITION));
			}

			GridData data = {};
			data.thin_color = m_minorColor.ToColor4f();
			data.thick_color = m_majorColor.ToColor4f();
			data.line_spacing = lineSpacing;
			data.line_width = m_lineWidth;

			m_gridMat->SetBufferDynamic("GridData"_hash, &data);
			r->DrawMesh(m_sphereMesh.get(), m_gridMat.get());
		}

		//------------------------------------------------------------
		Axes3D::Axes3D(Graphics::Renderer *r)
		{
			PROFILE_SCOPED()

			Graphics::MaterialDescriptor desc;

			Graphics::RenderStateDesc rsd;
			rsd.cullMode = Graphics::CULL_NONE;
			rsd.depthWrite = false;
			rsd.primitiveType = Graphics::LINE_SINGLE;
			m_axesMat.Reset(r->CreateMaterial("vtxColor", desc, rsd));

			VertexArray vertices(ATTRIB_POSITION | ATTRIB_DIFFUSE);

			//Draw plain XYZ axes using the current transform
			static const vector3f vtsXYZ[] = {
				vector3f(0.f, 0.f, 0.f),
				vector3f(1.f, 0.f, 0.f),
				vector3f(0.f, 0.f, 0.f),
				vector3f(0.f, 1.f, 0.f),
				vector3f(0.f, 0.f, 0.f),
				vector3f(0.f, 0.f, 1.f),
			};
			static const Color colors[] = {
				Color::RED,
				Color::RED,
				Color::GREEN,
				Color::GREEN,
				Color::BLUE,
				Color::BLUE,
			};

			for (int i = 0; i < 6; i++) {
				vertices.Add(vtsXYZ[i], colors[i]);
			}

			//Create vtx buffer and copy data
			m_axesMesh.Reset(r->CreateMeshObjectFromArray(&vertices));
		}

		void Axes3D::Draw(Graphics::Renderer *r)
		{
			PROFILE_SCOPED()
			r->DrawMesh(m_axesMesh.Get(), m_axesMat.Get());
		}

		static Axes3D *s_axes = nullptr;
		Axes3D *GetAxes3DDrawable(Graphics::Renderer *r)
		{
			if (!s_axes) {
				s_axes = new Axes3D(r);
			}
			return s_axes;
		}

		void AABB::DrawVertices(Graphics::VertexArray &va, const matrix4x4f &transform, const Aabb &aabb, Color color)
		{
			const vector3f verts[16] = {
				transform * vector3f(aabb.min.x, aabb.min.y, aabb.min.z),
				transform * vector3f(aabb.max.x, aabb.min.y, aabb.min.z),
				transform * vector3f(aabb.max.x, aabb.max.y, aabb.min.z),
				transform * vector3f(aabb.min.x, aabb.max.y, aabb.min.z),
				transform * vector3f(aabb.min.x, aabb.min.y, aabb.min.z),
				transform * vector3f(aabb.min.x, aabb.min.y, aabb.max.z),
				transform * vector3f(aabb.max.x, aabb.min.y, aabb.max.z),
				transform * vector3f(aabb.max.x, aabb.min.y, aabb.min.z),

				transform * vector3f(aabb.max.x, aabb.max.y, aabb.max.z),
				transform * vector3f(aabb.min.x, aabb.max.y, aabb.max.z),
				transform * vector3f(aabb.min.x, aabb.min.y, aabb.max.z),
				transform * vector3f(aabb.max.x, aabb.min.y, aabb.max.z),
				transform * vector3f(aabb.max.x, aabb.max.y, aabb.max.z),
				transform * vector3f(aabb.max.x, aabb.max.y, aabb.min.z),
				transform * vector3f(aabb.min.x, aabb.max.y, aabb.min.z),
				transform * vector3f(aabb.min.x, aabb.max.y, aabb.max.z),
			};

			for (unsigned int i = 0; i < 7; i++) {
				va.Add(verts[i], color);
				va.Add(verts[i + 1], color);
			}

			for (unsigned int i = 8; i < 15; i++) {
				va.Add(verts[i], color);
				va.Add(verts[i + 1], color);
			}
		}

	} // namespace Drawables

} // namespace Graphics
