// Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyMap.h"
#include "Game.h"
#include "LabelOverlay.h"
#include "Pi.h"
#include "galaxy/Galaxy.h"
#include "matrix4x4.h"

using namespace UI;

namespace {

	static const char GALAXY_IMAGE_FILE[] = "galaxy_colour.png";

} // anonymous namespace

namespace GameUI {

	GalaxyMap::GalaxyMap(Context *context) :
		OverlayStack(context),
		m_baseImage(nullptr),
		m_labelOverlay(nullptr),
		m_zoom(1.0f),
		m_displayScale(0.0f),
		m_centreSector(0.0f, 0.0f)
	{
		m_baseImage = context->Image(
			std::string(GALAXY_IMAGE_FILE),
			UI::Widget::EXPAND_WIDTH | UI::Widget::EXPAND_HEIGHT);
		m_baseImage->SetPreserveAspect(true);
		AddLayer(m_baseImage);

		m_labelOverlay = new GameUI::LabelOverlay(context);
		AddLayer(m_labelOverlay);

		GetContext()->RequestLayout();
	}

	UI::Point GalaxyMap::PreferredSize()
	{
		return m_baseImage->PreferredSize();
	}

	void GalaxyMap::Update()
	{
		const Galaxy *galaxy = Pi::game->GetGalaxy().Get();
		const float inv_sector_size = 1.0 / Sector::SIZE;
		const float radius_sectors = galaxy->GALAXY_RADIUS * inv_sector_size;

		const UI::Point widget_size = m_baseImage->GetSize();
		const float widget_aspect = float(widget_size.x) / float(widget_size.y);
		vector2f viewport_sectors(radius_sectors, radius_sectors);
		float display_scale;
		if (widget_size.x > widget_size.y) {
			viewport_sectors.x *= widget_aspect;
			display_scale = radius_sectors / float(widget_size.y);
		} else {
			viewport_sectors.y /= widget_aspect;
			display_scale = radius_sectors / float(widget_size.x);
		}
		display_scale *= 2.0f / m_zoom;
		if (!is_equal_exact(m_displayScale, display_scale)) {
			m_displayScale = display_scale;
			onDisplayScaleChanged.emit(m_displayScale);
		}

		const vector2f sol_offset(galaxy->SOL_OFFSET_X * inv_sector_size,
			galaxy->SOL_OFFSET_Y * inv_sector_size);

		const Graphics::Frustum frustum(
			matrix4x4d::Translation(-m_centreSector.x, -m_centreSector.y, 0.0),
			matrix4x4d::ScaleMatrix(m_zoom) *
				matrix4x4d::OrthoFrustum(-viewport_sectors.x, viewport_sectors.x, // left, right
					viewport_sectors.y, -viewport_sectors.y, // top, bottom
					-1.0, 1.0));
		m_labelOverlay->SetView(frustum);

		m_baseImage->SetTransform(m_zoom, vector2f((m_centreSector.x + sol_offset.x) / radius_sectors, (m_centreSector.y + sol_offset.y) / radius_sectors));

		OverlayStack::Update();
	}

	void GalaxyMap::ClearLabels()
	{
		m_labelOverlay->Clear();
	}

	GalaxyMap *GalaxyMap::AddAreaLabel(const vector2f &at, const std::string &text)
	{
		const vector3f at3(at, 0.0f);
		LabelOverlay::Marker *m = m_labelOverlay->AddMarker(text, at3);
		m->color = Color4ub(255, 255, 255, 255);
		m->style = LabelOverlay::MARKER_NONE;
		m->textAnchor = UI::Align::MIDDLE;
		return this;
	}

	GalaxyMap *GalaxyMap::AddPointLabel(const vector2f &at, const std::string &text)
	{
		const vector3f at3(at, 0.0f);
		LabelOverlay::Marker *m = m_labelOverlay->AddMarker(text, at3);
		m->color = Color4ub(0, 255, 0, 255);
		m->style = LabelOverlay::MARKER_DOT;
		m->textAnchor = UI::Align::LEFT;
		return this;
	}

	GalaxyMap *GalaxyMap::SetZoom(float v)
	{
		m_zoom = Clamp(v, 0.5f, 20.0f);
		return this;
	}

	GalaxyMap *GalaxyMap::SetCentreSector(const vector2f &at)
	{
		m_centreSector = at;
		return this;
	}

} // namespace GameUI
