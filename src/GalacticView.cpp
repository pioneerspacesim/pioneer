// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "libs.h"
#include "gui/Gui.h"
#include "Pi.h"
#include "GalacticView.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "SectorView.h"
#include "galaxy/Sector.h"
#include "galaxy/Galaxy.h"
#include "Lang.h"
#include "StringF.h"
#include "AnimationCurves.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"

using namespace Graphics;
static const float ZOOM_IN_SPEED = 2;
static const float ZOOM_OUT_SPEED = 1.f/ZOOM_IN_SPEED;
static const float WHEEL_SENSITIVITY = .2f;		// Should be a variable in user settings.

GalacticView::GalacticView() : UIView(),
	m_quad(Graphics::TextureBuilder::UI("galaxy.bmp").CreateTexture(Gui::Screen::GetRenderer()))
{

	SetTransparency(true);
	m_zoom = 1.0f;
	m_zoomTo = m_zoom;

	m_zoomInButton = new Gui::ImageButton("icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	m_zoomInButton->SetRenderDimensions(30, 22);
	Add(m_zoomInButton, 700, 5);

	m_zoomOutButton = new Gui::ImageButton("icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	m_zoomOutButton->SetRenderDimensions(30, 22);
	Add(m_zoomOutButton, 732, 5);

	m_scaleReadout = new Gui::Label("");
	Add(m_scaleReadout, 500.0f, 10.0f);

	Gui::Screen::PushFont("OverlayFont");
	m_labels = new Gui::LabelSet();
	Add(m_labels, 0, 0);
	Gui::Screen::PopFont();

	m_onMouseWheelCon =
		Pi::onMouseWheel.connect(sigc::mem_fun(this, &GalacticView::MouseWheel));

	Graphics::RenderStateDesc rsd;
	rsd.depthTest  = false;
	rsd.depthWrite = false;
	m_renderState = Gui::Screen::GetRenderer()->CreateRenderState(rsd);
}

GalacticView::~GalacticView()
{
	m_onMouseWheelCon.disconnect();
}

void GalacticView::Save(Serializer::Writer &wr)
{
}

void GalacticView::Load(Serializer::Reader &rd)
{
}


struct galaclabel_t {
	const char *label;
	vector3d pos;
} s_labels[] = {
	{ Lang::NORMA_ARM, vector3d(0.0,-0.3,0.0) },
	{ Lang::PERSEUS_ARM, vector3d(0.57,0.0,0.0) },
	{ Lang::OUTER_ARM, vector3d(0.65,0.4,0.0) },
	{ Lang::SAGITTARIUS_ARM, vector3d(-.3,0.2,0.0) },
	{ Lang::SCUTUM_CENTAURUS_ARM, vector3d(-.45,-0.45,0.0) },
	{ 0, vector3d(0.0, 0.0, 0.0) }
};

static void dummy() {}

void GalacticView::PutLabels(vector3d offset)
{
	Gui::Screen::EnterOrtho();

	for (int i=0; s_labels[i].label; i++) {
		vector3d p = m_zoom * (s_labels[i].pos + offset);
		vector3d pos;
		if (Gui::Screen::Project(p, pos)) {
			m_labels->Add(s_labels[i].label, sigc::ptr_fun(&dummy), float(pos.x), float(pos.y));
		}
	}

	Gui::Screen::LeaveOrtho();
}


void GalacticView::Draw3D()
{
	PROFILE_SCOPED()
	vector3f pos = Pi::sectorView->GetPosition();
	float offset_x = (pos.x*Sector::SIZE + Pi::GetGalaxy()->SOL_OFFSET_X)/Pi::GetGalaxy()->GALAXY_RADIUS;
	float offset_y = (-pos.y*Sector::SIZE + Pi::GetGalaxy()->SOL_OFFSET_Y)/Pi::GetGalaxy()->GALAXY_RADIUS;

	const float aspect = m_renderer->GetDisplayAspect();
	m_renderer->SetOrthographicProjection(-aspect, aspect, 1.f, -1.f, -1.f, 1.f);
	m_renderer->ClearScreen();

	//apply zoom
	m_renderer->SetTransform(
		matrix4x4f::Identity() *
		matrix4x4f::ScaleMatrix(m_zoom, m_zoom, 0.f) *
		matrix4x4f::Translation(-offset_x, -offset_y, 0.f));

	// galaxy image
	m_quad.Draw(m_renderer, vector2f(-1.0f), vector2f(2.0f));

	// "you are here" dot
	//Color green(0, 255, 0, 255);
	vector3f offs(offset_x, offset_y, 0.f);
	m_renderer->DrawPoints(1, &offs, &Color::GREEN, m_renderState, 3.f);

	// scale at the top
	m_renderer->SetTransform(matrix4x4f::Identity());
	//Color white(255);
	const vector2f vts[] = {
		vector2f(-0.25f,-0.93f),
		vector2f(-0.25f,-0.94f),
		vector2f(0.25f,-0.94f),
		vector2f(0.25f,-0.93f)
	};
	m_renderer->DrawLines2D(4, vts, Color::WHITE, m_renderState, LINE_STRIP);

	m_labels->Clear();
	PutLabels(-vector3d(offset_x, offset_y, 0.0));

	UIView::Draw3D();
}

void GalacticView::Update()
{
	const float frameTime = Pi::GetFrameTime();

	if (m_zoomInButton->IsPressed()) m_zoomTo *= pow(ZOOM_IN_SPEED * Pi::GetMoveSpeedShiftModifier(), frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoomTo *= pow(ZOOM_OUT_SPEED / Pi::GetMoveSpeedShiftModifier(), frameTime);
	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
		if (Pi::KeyState(SDLK_EQUALS)) m_zoomTo *= pow(ZOOM_IN_SPEED * Pi::GetMoveSpeedShiftModifier(), frameTime);
		if (Pi::KeyState(SDLK_MINUS)) m_zoomTo *= pow(ZOOM_OUT_SPEED / Pi::GetMoveSpeedShiftModifier(), frameTime);
	}
	m_zoomTo = Clamp(m_zoomTo, 0.5f, 100.0f);
	m_zoom = Clamp(m_zoom, 0.5f, 100.0f);
	AnimationCurves::Approach(m_zoom, m_zoomTo, frameTime);

	m_scaleReadout->SetText(stringf(Lang::INT_LY, formatarg("scale", int(0.5*Pi::GetGalaxy()->GALAXY_RADIUS/m_zoom))));

	UIView::Update();
}

void GalacticView::MouseWheel(bool up)
{
	if (this == Pi::GetView()) {
        if (!up)
			m_zoomTo *= ((ZOOM_OUT_SPEED-1) * WHEEL_SENSITIVITY+1) / Pi::GetMoveSpeedShiftModifier();
		else
			m_zoomTo *= ((ZOOM_IN_SPEED-1) * WHEEL_SENSITIVITY+1) * Pi::GetMoveSpeedShiftModifier();
	}
}

