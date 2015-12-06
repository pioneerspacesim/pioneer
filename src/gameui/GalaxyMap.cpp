// Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GalaxyMap.h"
#include "galaxy/Galaxy.h"
#include "Game.h"
#include "Pi.h"
#include "matrix4x4.h"

using namespace UI;

namespace {

static const char GALAXY_IMAGE_FILE[] = "galaxy_colour.png";

} // anonymous namespace

namespace GameUI {

GalaxyMap::GalaxyMap(Context *context):
	Single(context),
	m_baseImage(nullptr),
	m_zoom(1.0f),
	m_centreSector(0.0f, 0.0f)
{
	m_baseImage = context->Image(
		std::string(GALAXY_IMAGE_FILE),
		UI::Widget::EXPAND_WIDTH | UI::Widget::EXPAND_HEIGHT);
	m_baseImage->SetPreserveAspect(true);
	SetInnerWidget(m_baseImage);
	GetContext()->RequestLayout();
}

UI::Point GalaxyMap::PreferredSize() {
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
	if (widget_size.x > widget_size.y) {
		viewport_sectors.x *= widget_aspect;
	} else {
		viewport_sectors.y /= widget_aspect;
	}

	const vector2f sol_offset(galaxy->SOL_OFFSET_X * inv_sector_size,
	                          galaxy->SOL_OFFSET_Y * inv_sector_size);

	m_baseImage->SetTransform(m_zoom, vector2f(
		(m_centreSector.x + sol_offset.x) / radius_sectors,
		(m_centreSector.y + sol_offset.y) / radius_sectors));

	Single::Update();
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

}

