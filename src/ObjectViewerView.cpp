#include "ObjectViewerView.h"
#include "WorldView.h"
#include "Pi.h"
#include "Frame.h"
#include "Player.h"
#include "Space.h"
#include "GeoSphere.h"
#include "terrain/Terrain.h"
#include "Planet.h"

#if OBJECTVIEWER

ObjectViewerView::ObjectViewerView(): View()
{
	SetTransparency(true);
	viewingDist = 1000.0f;
	m_camRot = matrix4x4d::Identity();
	
	m_infoLabel = new Gui::Label("");
	Add(m_infoLabel, 2, Gui::Screen::GetHeight()-66-Gui::Screen::GetFontHeight());

	Gui::VBox *vbox = new Gui::VBox();
	Add(vbox, 580, 2);

	vbox->PackEnd(new Gui::Label("Mass (earths):"));
	m_sbodyMass = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyMass);

	vbox->PackEnd(new Gui::Label("Radius (earths):"));
	m_sbodyRadius = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyRadius);

	vbox->PackEnd(new Gui::Label("Integer seed:"));
	m_sbodySeed = new Gui::TextEntry();
	vbox->PackEnd(m_sbodySeed);

	vbox->PackEnd(new Gui::Label("Volatile gases (>= 0):"));
	m_sbodyVolatileGas = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyVolatileGas);

	vbox->PackEnd(new Gui::Label("Volatile liquid (0-1):"));
	m_sbodyVolatileLiquid = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyVolatileLiquid);

	vbox->PackEnd(new Gui::Label("Volatile ices (0-1):"));
	m_sbodyVolatileIces = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyVolatileIces);

	vbox->PackEnd(new Gui::Label("Life (0-1):"));
	m_sbodyLife = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyLife);

	vbox->PackEnd(new Gui::Label("Volcanicity (0-1):"));
	m_sbodyVolcanicity = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyVolcanicity);

	vbox->PackEnd(new Gui::Label("Crust metallicity (0-1):"));
	m_sbodyMetallicity = new Gui::TextEntry();
	vbox->PackEnd(m_sbodyMetallicity);

	Gui::LabelButton *b = new Gui::LabelButton(new Gui::Label("Change planet terrain type"));
	b->onClick.connect(sigc::mem_fun(this, &ObjectViewerView::OnChangeTerrain));
	vbox->PackEnd(b);
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
	Render::GetNearFarClipPlane(znear, zfar);
	float fracH = znear / Pi::GetScrAspect();
	glFrustum(-znear, znear, -fracH, fracH, znear, zfar);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glEnable(GL_LIGHT0);

	Render::State::SetZnearZfar(znear, zfar);

	if (Pi::MouseButtonState(SDL_BUTTON_RIGHT)) {
		int m[2];
		Pi::GetMouseMotion(m);
		m_camRot = matrix4x4d::RotateXMatrix(-0.002*m[1]) *
				matrix4x4d::RotateYMatrix(-0.002*m[0]) * m_camRot;
	}
		
	Body *body = Pi::player->GetNavTarget();
	if (body) {
		float lightPos[4];
		if (body->IsType(Object::STAR))
			lightPos[0] = lightPos[1] = lightPos[2] = lightPos[3] = 0;
		else {
			lightPos[0] = lightPos[1] = lightPos[2] = 0.577f;
			lightPos[3] = 0;
		}
		glLightfv(GL_LIGHT0, GL_POSITION, lightPos);
	
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
			const SBody *sbody = planet->GetSBody();
			m_sbodyVolatileGas->SetText(stringf("%0{f.3}", sbody->m_volatileGas.ToFloat()));
			m_sbodyVolatileLiquid->SetText(stringf("%0{f.3}", sbody->m_volatileLiquid.ToFloat()));
			m_sbodyVolatileIces->SetText(stringf("%0{f.3}", sbody->m_volatileIces.ToFloat()));
			m_sbodyLife->SetText(stringf("%0{f.3}", sbody->m_life.ToFloat()));
			m_sbodyVolcanicity->SetText(stringf("%0{f.3}", sbody->m_volcanicity.ToFloat()));
			m_sbodyMetallicity->SetText(stringf("%0{f.3}", sbody->m_metallicity.ToFloat()));
			m_sbodySeed->SetText(stringf("%0{u}", sbody->seed));
			m_sbodyMass->SetText(stringf("%0{f}", sbody->mass.ToFloat()));
			m_sbodyRadius->SetText(stringf("%0{f}", sbody->radius.ToFloat()));
		}
	}
	snprintf(buf, sizeof(buf), "View dist: %s     Object: %s", format_distance(viewingDist).c_str(), (body ? body->GetLabel().c_str() : "<none>"));
	m_infoLabel->SetText(buf);
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
	SBody *sbody = const_cast<SBody*>(body->GetSBody());

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
	if (body->IsType(Object::TERRAINBODY))
		static_cast<TerrainBody*>(body)->GetGeoSphere()->OnChangeDetailLevel();
}

#endif
