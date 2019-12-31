// Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Image.h"
#include "Context.h"
#include "graphics/TextureBuilder.h"

namespace UI {

	namespace {

		static Point CalcDisplayDimensions(const UI::Context *context, const Graphics::Texture *texture)
		{
			const auto image_size = texture->GetDescriptor().GetOriginalSize();
			const float scale = context->GetScale();
			return Point(scale * image_size.x, scale * image_size.y);
		}

	} // namespace

	Image::Image(Context *context, const std::string &filename, Uint32 sizeControlFlags) :
		Widget(context),
		m_centre(0.0f, 0.0f),
		m_scale(1.0f),
		m_preserveAspect(false),
		m_needsRefresh(false)
	{
		Graphics::TextureBuilder b = Graphics::TextureBuilder::UI(filename);
		m_texture.Reset(b.GetOrCreateTexture(GetContext()->GetRenderer(), "ui"));

		m_initialSize = CalcDisplayDimensions(GetContext(), m_texture.Get());

		Graphics::MaterialDescriptor material_desc;
		material_desc.textures = 1;
		m_material.Reset(GetContext()->GetRenderer()->CreateMaterial(material_desc));
		m_material->texture0 = m_texture.Get();

		SetSizeControlFlags(sizeControlFlags);
	}

	Point Image::PreferredSize()
	{
		return m_initialSize;
	}

	Image *Image::SetHeightLines(Uint32 lines)
	{
		m_needsRefresh = true;
		const Text::TextureFont *font = GetContext()->GetFont(GetFont()).Get();
		const float height = font->GetHeight() * lines;

		const vector2f sz = m_texture->GetDescriptor().GetOriginalSize();
		const float width = height * sz.x / sz.y;

		m_initialSize = UI::Point(width, height);
		GetContext()->RequestLayout();
		return this;
	}

	Image *Image::SetNaturalSize()
	{
		m_needsRefresh = true;
		m_initialSize = CalcDisplayDimensions(GetContext(), m_texture.Get());
		GetContext()->RequestLayout();
		return this;
	}

	void Image::SetTransform(float scale, const vector2f &centre)
	{
		if (!is_equal_exact(m_scale, scale) || !m_centre.ExactlyEqual(centre)) {
			m_needsRefresh = true;
			m_scale = scale;
			m_centre = centre;
		}
	}

	void Image::SetPreserveAspect(bool preserve_aspect)
	{
		m_needsRefresh = true;
		m_preserveAspect = preserve_aspect;
	}

	void Image::Draw()
	{
		Graphics::Renderer *r = GetContext()->GetRenderer();
		if (!m_quad || m_needsRefresh) {
			m_needsRefresh = false;
			const Point &offset = GetActiveOffset();
			const Point &area = GetActiveArea();
			const auto &descriptor = m_texture->GetDescriptor();

			const float half_sx = area.x * 0.5f;
			const float half_sy = area.y * 0.5f;

			float cx = offset.x + half_sx;
			float cy = offset.y + half_sy;
			float rx, ry;

			if (m_preserveAspect) {
				const vector2f sz = descriptor.GetOriginalSize();
				const float wantRatio = sz.x / sz.y;
				const float haveRatio = float(area.x) / float(area.y);
				if (wantRatio > haveRatio) {
					// limited by width
					rx = half_sx;
					ry = half_sx / wantRatio;
				} else {
					// limited by height
					rx = half_sy * wantRatio;
					ry = half_sy;
				}
			} else {
				rx = half_sx;
				ry = half_sy;
			}

			rx *= m_scale;
			ry *= m_scale;
			cx -= rx * m_centre.x;
			cy -= ry * m_centre.y;
			const vector2f texSize = descriptor.texSize;

			Graphics::VertexArray va(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_UV0);
			va.Add(vector3f(cx - rx, cy - ry, 0.0f), vector2f(0.0f, 0.0f));
			va.Add(vector3f(cx - rx, cy + ry, 0.0f), vector2f(0.0f, texSize.y));
			va.Add(vector3f(cx + rx, cy - ry, 0.0f), vector2f(texSize.x, 0.0f));
			va.Add(vector3f(cx + rx, cy + ry, 0.0f), vector2f(texSize.x, texSize.y));

			auto renderState = GetContext()->GetSkin().GetAlphaBlendState();
			m_quad.reset(new Graphics::Drawables::TexturedQuad(r, m_material, va, renderState));
		}
		m_quad->Draw(r, Color(Color::WHITE.r, Color::WHITE.g, Color::WHITE.b, GetContext()->GetOpacity() * Color::WHITE.a));
	}

} // namespace UI
