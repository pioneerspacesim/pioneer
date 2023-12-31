// Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Label3D.h"
#include "NodeVisitor.h"

#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

#include "profiler/Profiler.h"

namespace SceneGraph {

	Label3D::Label3D(Graphics::Renderer *r, RefCountedPtr<Text::DistanceFieldFont> font) :
		Node(r, NODE_TRANSPARENT),
		m_font(font)
	{
		Graphics::MaterialDescriptor matdesc;
		matdesc.textures = 1;
		matdesc.alphaTest = true;
		matdesc.lighting = true;

		Graphics::RenderStateDesc rsd;
		rsd.depthWrite = false;
		rsd.blendMode = Graphics::BLEND_ALPHA;

		m_geometry.reset(font->CreateVertexArray());
		m_material.Reset(r->CreateMaterial("label", matdesc, rsd));
		m_material->SetTexture("texture0"_hash, font->GetTexture());
		m_material->diffuse = Color::WHITE;
		m_material->emissive = Color(38, 38, 38);
		m_material->specular = Color::WHITE;
	}

	Label3D::Label3D(const Label3D &label, NodeCopyCache *cache) :
		Node(label, cache),
		m_material(label.m_material),
		m_font(label.m_font)
	{
		m_geometry.reset(m_font->CreateVertexArray());
	}

	Node *Label3D::Clone(NodeCopyCache *cache)
	{
		return new Label3D(*this, cache);
	}

	void Label3D::SetText(const std::string &text)
	{
		//regenerate geometry
		m_geometry->Clear();
		if (!text.empty()) {
			m_font->GetGeometry(*m_geometry, text, vector2f(0.f));

			// Happens if none of the characters in the string have glyphs in the SDF font.
			// Most noticeably, this means text consisting of entirely Cyrillic
			// or Chinese characters will vanish when rendered on a Label3D.
			if (m_geometry->IsEmpty()) {
				return;
			}

			//create buffer and upload data
			m_textMesh.reset(m_renderer->CreateMeshObjectFromArray(m_geometry.get()));
		}
	}

	void Label3D::Render(const matrix4x4f &trans, const RenderData *rd)
	{
		PROFILE_SCOPED()
		if (m_textMesh.get()) {
			Graphics::Renderer *r = GetRenderer();
			r->SetTransform(trans);
			r->DrawMesh(m_textMesh.get(), m_material.Get());
		}
	}

	void Label3D::Accept(NodeVisitor &nv)
	{
		nv.ApplyLabel(*this);
	}

} // namespace SceneGraph
