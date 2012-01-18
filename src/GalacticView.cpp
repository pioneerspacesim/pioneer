#include "libs.h"
#include "gui/Gui.h"
#include "Pi.h"
#include "GalacticView.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "SectorView.h"
#include "Sector.h"
#include "Galaxy.h"
#include "Lang.h"
#include "StringF.h"

GalacticView::GalacticView()
{
	m_texture.Reset(new UITexture(Galaxy::GetGalaxyBitmap()));

	SetTransparency(true);
	m_zoom = 1.0f;

	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in.png");
	m_zoomInButton->SetToolTip(Lang::ZOOM_IN);
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out.png");
	m_zoomOutButton->SetToolTip(Lang::ZOOM_OUT);
	Add(m_zoomOutButton, 732, 5);
	
	m_scaleReadout = new Gui::Label("");
	Add(m_scaleReadout, 500.0f, 10.0f);

	Gui::Screen::PushFont("OverlayFont");
	m_labels = new Gui::LabelSet();
	Add(m_labels, 0, 0);
	Gui::Screen::PopFont();

	m_onMouseButtonDown = 
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &GalacticView::MouseButtonDown));
}

GalacticView::~GalacticView()
{
	m_onMouseButtonDown.disconnect();
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
	glColor3f(1,1,1);
	
	for (int i=0; s_labels[i].label; i++) {
		vector3d p = m_zoom * (s_labels[i].pos + offset);
		vector3d pos;
		if (Gui::Screen::Project(p, pos)) {
			m_labels->Add(s_labels[i].label, sigc::ptr_fun(&dummy), float(pos.x), float(pos.y));
		}
	}

	Gui::Screen::LeaveOrtho();
	glDisable(GL_LIGHTING);			// what
}


void GalacticView::Draw3D()
{
	vector3f pos = Pi::sectorView->GetPosition();
	float offset_x = (pos.x*Sector::SIZE + Galaxy::SOL_OFFSET_X)/Galaxy::GALAXY_RADIUS;
	float offset_y = (-pos.y*Sector::SIZE + Galaxy::SOL_OFFSET_Y)/Galaxy::GALAXY_RADIUS;

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(-Pi::GetScrAspect(), Pi::GetScrAspect(), 1.0, -1.0, -1, 1);
	
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glClearColor(0,0,0,0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);

	glEnable(GL_TEXTURE_2D);
	m_texture->Bind();
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
	
	glScalef(m_zoom, m_zoom, 0.0f);
	glTranslatef(-offset_x, -offset_y, 0.0f);

	glBegin(GL_QUADS);
		float w = 1.0;
		float h = 1.0;
		glTexCoord2f(0,h);
		glVertex2f(-1.0,1.0);
		glTexCoord2f(w,h);
		glVertex2f(1.0,1.0);
		glTexCoord2f(w,0);
		glVertex2f(1.0,-1.0);
		glTexCoord2f(0,0);
		glVertex2f(-1.0,-1.0);
	glEnd();

	m_texture->Unbind();
	glDisable(GL_TEXTURE_2D);

	glColor3f(0.0,1.0,0.0);
	glPointSize(3.0);
	glBegin(GL_POINTS);
		glVertex2f(offset_x, offset_y);
	glEnd();
	
	glLoadIdentity();
	glColor3f(1,1,1);
	glPointSize(1.0);
	glBegin(GL_LINE_STRIP);
		glVertex2f(-0.25,-0.93);
		glVertex2f(-0.25,-0.94);
		glVertex2f(0.25,-0.94);
		glVertex2f(0.25,-0.93);
	glEnd();

	m_labels->Clear();
	PutLabels(-vector3d(offset_x, offset_y, 0.0));

	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}
	
void GalacticView::Update()
{
	const float frameTime = Pi::GetFrameTime();
	
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(4.0f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(0.25f, frameTime);
	// XXX ugly hack checking for console here
	if (!Pi::IsConsoleActive()) {
		if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(4.0f, frameTime);
		if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(0.25f, frameTime);
	}
	m_zoom = Clamp(m_zoom, 0.5f, 100.0f);

	m_scaleReadout->SetText(stringf(Lang::INT_LY, formatarg("scale", int(0.5*Galaxy::GALAXY_RADIUS/m_zoom))));
}

void GalacticView::MouseButtonDown(int button, int x, int y)
{
	const float ft = Pi::GetFrameTime();
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN)) 
			m_zoom *= pow(0.25f, ft);
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP)) 
			m_zoom *= pow(4.0f, ft);
}

