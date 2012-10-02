// Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ObjectViewerView.h"
#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"
#include "GeoSphere.h"
#include "terrain/Terrain.h"
#include "Planet.h"
#include "graphics/Light.h"
#include "graphics/Renderer.h"

#if WITH_OBJECTVIEWER

ObjectViewerView::ObjectViewerView(): View()
{
	SetTransparency(true);
	viewingDist = 1000.0f;
	m_camRot = matrix4x4d::Identity();

	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-66-Gui::Screen::GetFontHeight());

	m_vbox = new Gui::VBox();
	Add(m_vbox, 580, 2);

	m_vbox->PackEnd(new Gui::Label("Mass (earths):"));
	m_sbodyMass = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyMass);

	m_vbox->PackEnd(new Gui::Label("Radius (earths):"));
	m_sbodyRadius = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyRadius);

	m_vbox->PackEnd(new Gui::Label("Integer seed:"));
	m_sbodySeed = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodySeed);

	m_vbox->PackEnd(new Gui::Label("Volatile gases (>= 0):"));
	m_sbodyVolatileGas = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyVolatileGas);

	m_vbox->PackEnd(new Gui::Label("Volatile liquid (0-1):"));
	m_sbodyVolatileLiquid = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyVolatileLiquid);

	m_vbox->PackEnd(new Gui::Label("Volatile ices (0-1):"));
	m_sbodyVolatileIces = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyVolatileIces);

	m_vbox->PackEnd(new Gui::Label("Life (0-1):"));
	m_sbodyLife = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyLife);

	m_vbox->PackEnd(new Gui::Label("Volcanicity (0-1):"));
	m_sbodyVolcanicity = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyVolcanicity);

	m_vbox->PackEnd(new Gui::Label("Crust metallicity (0-1):"));
	m_sbodyMetallicity = new Gui::TextEntry();
	m_vbox->PackEnd(m_sbodyMetallicity);

	Gui::LabelButton *b = new Gui::LabelButton(new Gui::Label("Change planet terrain type"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnChangeTerrain));
	m_vbox->PackEnd(b);

	Gui::HBox *hbox = new Gui::HBox();

	b = new Gui::LabelButton(new Gui::Label("Prev Seed"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnPrevSeed));
	hbox->PackEnd(b);

	b = new Gui::LabelButton(new Gui::Label("Random Seed"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnRandomSeed));
	hbox->PackEnd(b);

	b = new Gui::LabelButton(new Gui::Label("Next Seed"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnNextSeed));
	hbox->PackEnd(b);

	m_vbox->PackEnd(hbox);
}

void ObjectViewerView::Draw3D()
{
	m_renderer->ClearScreen();
	float znear, zfar;
	m_renderer->GetNearFarRange(znear, zfar);
	m_renderer->SetPerspectiveProjection(75.f, Pi::GetScrAspect(), znear, zfar);
	m_renderer->SetTransform(matrix4x4f::Identity());

	Graphics::Light light;
	light.SetType(Graphics::Light::LIGHT_DIRECTIONAL);

	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		int m[2];
		Pi::GetMouseMotion(m);
		m_camRot = matrix4x4d::RotateXMatrix(-0.002*m[1]) *
				matrix4x4d::RotateYMatrix(-0.002*m[0]) * m_camRot;
	}

	Body *body = Pi::player->GetNavTarget();
	if (body) {
		if (body->IsType(Object::STAR))
			light.SetPosition(vector3f(0.f));
		else {
			light.SetPosition(vector3f(0.577f));
		}
		m_renderer->SetLights(1, &light);

		body->Render(m_renderer, 0, vector3d(0,0,-viewingDist), m_camRot);
	}
}

void ObjectViewerView::OnSwitchTo()
{
	m_camRot = matrix4x4d::Identity();
}

void ObjectViewerView::Update()
{
	if (Pi::KeyState(SDLK_EQUALS)) viewingDist *= 0.99f;
	if (Pi::KeyState(SDLK_MINUS)) viewingDist *= 1.01f;
	viewingDist = Clamp(viewingDist, 10.0f, 1e12f);

	char buf[128];
	Body *body = Pi::player->GetNavTarget();
	if(body && (body != lastTarget)) {
		// Reset view distance for new target.
		viewingDist = body->GetBoundingRadius() * 2.0f;
		lastTarget = body;

		if (body->IsType(Object::TERRAINBODY)) {
			TerrainBody *tbody = static_cast<TerrainBody*>(body);
			const SystemBody *sbody = tbody->GetSystemBody();
			m_sbodyVolatileGas->SetText(stringf("%0{f.3}", sbody->m_volatileGas.ToFloat()));
			m_sbodyVolatileLiquid->SetText(stringf("%0{f.3}", sbody->m_volatileLiquid.ToFloat()));
			m_sbodyVolatileIces->SetText(stringf("%0{f.3}", sbody->m_volatileIces.ToFloat()));
			m_sbodyLife->SetText(stringf("%0{f.3}", sbody->m_life.ToFloat()));
			m_sbodyVolcanicity->SetText(stringf("%0{f.3}", sbody->m_volcanicity.ToFloat()));
			m_sbodyMetallicity->SetText(stringf("%0{f.3}", sbody->m_metallicity.ToFloat()));
			m_sbodySeed->SetText(stringf("%0{i}", int(sbody->seed)));
			m_sbodyMass->SetText(stringf("%0{f}", sbody->mass.ToFloat()));
			m_sbodyRadius->SetText(stringf("%0{f}", sbody->radius.ToFloat()));
		}
	}
	snprintf(buf, sizeof(buf), "View dist: %s     Object: %s", format_distance(viewingDist).c_str(), (body ? body->GetLabel().c_str() : "<none>"));
	m_infoLabel->SetText(buf);

	if (body && body->IsType(Object::TERRAINBODY)) m_vbox->ShowAll();
	else m_vbox->HideAll();
}

void ObjectViewerView::OnChangeTerrain()
{
	const fixed volatileGas = fixed(65536.0*atof(m_sbodyVolatileGas->GetText().c_str()), 65536);
	const fixed volatileLiquid = fixed(65536.0*atof(m_sbodyVolatileLiquid->GetText().c_str()), 65536);
	const fixed volatileIces = fixed(65536.0*atof(m_sbodyVolatileIces->GetText().c_str()), 65536);
	const fixed life = fixed(65536.0*atof(m_sbodyLife->GetText().c_str()), 65536);
	const fixed volcanicity = fixed(65536.0*atof(m_sbodyVolcanicity->GetText().c_str()), 65536);
	const fixed metallicity = fixed(65536.0*atof(m_sbodyMetallicity->GetText().c_str()), 65536);
	const fixed mass = fixed(65536.0*atof(m_sbodyMass->GetText().c_str()), 65536);
	const fixed radius = fixed(65536.0*atof(m_sbodyRadius->GetText().c_str()), 65536);

	// XXX this is horrendous, but probably safe for the moment. all bodies,
	// terrain, whatever else holds a const pointer to the same toplevel
	// sbody. one day objectviewer should be far more contained and not
	// actually modify the space
	Body *body = Pi::player->GetNavTarget();
	SystemBody *sbody = const_cast<SystemBody*>(body->GetSystemBody());

	sbody->seed = atoi(m_sbodySeed->GetText().c_str());
	sbody->radius = radius;
	sbody->mass = mass;
	sbody->m_metallicity = metallicity;
	sbody->m_volatileGas = volatileGas;
	sbody->m_volatileLiquid = volatileLiquid;
	sbody->m_volatileIces = volatileIces;
	sbody->m_volcanicity = volcanicity;
	sbody->m_life = life;

	// force reload
	static_cast<TerrainBody*>(body)->GetGeoSphere()->OnChangeDetailLevel();
}

void ObjectViewerView::OnRandomSeed()
{
	m_sbodySeed->SetText(stringf("%0{i}", int(Pi::rng.Int32())));
	OnChangeTerrain();
}

void ObjectViewerView::OnNextSeed()
{
	m_sbodySeed->SetText(stringf("%0{i}", atoi(m_sbodySeed->GetText().c_str()) + 1));
	OnChangeTerrain();
}

void ObjectViewerView::OnPrevSeed()
{
	m_sbodySeed->SetText(stringf("%0{i}", atoi(m_sbodySeed->GetText().c_str()) - 1));
	OnChangeTerrain();
}

#endif
