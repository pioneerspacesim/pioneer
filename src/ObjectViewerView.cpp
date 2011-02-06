#include "ObjectViewerView.h"
#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"
#include "GeoSphere.h"
#include "GeoSphereStyle.h"
#include "Planet.h"

ObjectViewerView::ObjectViewerView(): View()
{
	SetTransparency(true);
	viewingDist = 1000.0f;
	m_camRot = matrix4x4d::Identity();
	
	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-66-Gui::Screen::GetFontHeight());

	Gui::Label *l = new Gui::Label("Terrain type:");
	Add(l, 600, 2);
	m_geosphereTerrainStyle = new Gui::TextEntry();
	Add(m_geosphereTerrainStyle, 700, 2);

	l = new Gui::Label("Coloring type:");
	Add(l, 600, 32);
	m_geosphereColorStyle = new Gui::TextEntry();
	Add(m_geosphereColorStyle, 700, 32);

	Gui::LabelButton *b = new Gui::LabelButton(new Gui::Label("Change planet terrain type"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnChangeGeoSphereStyle));
	Add(b, 600, 64);
}

void ObjectViewerView::Draw3D()
{
	static float rot;
	rot += 0.1;
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	float znear, zfar;
	Pi::worldView->GetNearFarClipPlane(&znear, &zfar);
	float fracH = znear / Pi::GetScrAspect();
	glFrustum(-znear, znear, -fracH, fracH, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_LIGHT0);

	Render::State::SetZnearZfar(znear, zfar);

	if (Pi::MouseButtonState(3)) {
		int m[2];
		Pi::GetMouseMotion(m);
		m_camRot = matrix4x4d::RotateXMatrix(-0.002*m[1]) *
				matrix4x4d::RotateYMatrix(-0.002*m[0]) * m_camRot;
	}
		
	float lightPos[4] = { .577, .577, .577, 0 };
	glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
	Body *body = Pi::player->GetNavTarget();
	if (body) {
		body->Render(vector3d(0,0,-viewingDist), m_camRot);
	}
}

void ObjectViewerView::OnSwitchTo()
{
	m_camRot = matrix4x4d::Identity();
}

void ObjectViewerView::Update()
{
	if (Pi::KeyState(SDLK_EQUALS)) viewingDist *= 0.99;
	if (Pi::KeyState(SDLK_MINUS)) viewingDist *= 1.01;
	viewingDist = Clamp(viewingDist, 10.0f, 1e12f);

	char buf[128];
	Body *body = Pi::player->GetNavTarget();
	if(body && (body != lastTarget)) {
		// Reset view distance for new target.
		viewingDist = body->GetBoundingRadius() * 2.0f;
		lastTarget = body;

		if (body->IsType(Object::PLANET)) {
			Planet *planet = static_cast<Planet*>(body);
			GeoSphere *gs = planet->m_geosphere;
			m_geosphereTerrainStyle->SetText(stringf(64, "%d", (int)gs->m_style.m_terrainType));
			m_geosphereColorStyle->SetText(stringf(64, "%d", (int)gs->m_style.m_colorType));

		}
	}
	snprintf(buf, sizeof(buf), "View dist: %s     Object: %s", format_distance(viewingDist).c_str(), (body ? body->GetLabel().c_str() : "<none>"));
	m_infoLabel->SetText(buf);
}

void ObjectViewerView::OnChangeGeoSphereStyle()
{
	int terrain_style = atoi(m_geosphereTerrainStyle->GetText().c_str());
	int color_style = atoi(m_geosphereColorStyle->GetText().c_str());

	terrain_style = Clamp(terrain_style, 0, (int)GeoSphereStyle::TERRAIN_MAX);
	color_style = Clamp(color_style, 0, (int)GeoSphereStyle::COLOR_MAX);

	Body *body = Pi::player->GetNavTarget();
	if (body->IsType(Object::PLANET)) {
		Planet *planet = static_cast<Planet*>(body);
		GeoSphere *gs = planet->m_geosphere;
		gs->m_style.m_terrainType = (GeoSphereStyle::TerrainType)terrain_style;
		gs->m_style.m_colorType = (GeoSphereStyle::ColorType)color_style;
		// force rebuild
		gs->OnChangeDetailLevel();
	}
}
