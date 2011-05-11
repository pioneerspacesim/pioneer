#include "libs.h"
#include "Gui.h"
#include "Pi.h"
#include "GalacticView.h"
#include "SystemInfoView.h"
#include "Player.h"
#include "Serializer.h"
#include "SectorView.h"
#include "Sector.h"
#include "Galaxy.h"
#include "Render.h"
#include "perlin.h"
		
GalacticView::GalacticView()
{
	const SDL_Surface *s = Galaxy::GetGalaxyBitmap();
	glEnable(GL_TEXTURE_2D);
	glGenTextures (1, &m_texture);
	glBindTexture (GL_TEXTURE_2D, m_texture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, s->w, s->h, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, s->pixels);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glDisable(GL_TEXTURE_2D);

	SetTransparency(true);
	m_zoom = 1.0f;
	m_rot_x = m_rot_z = 0.0f;

	m_zoomInButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_in_f7.png");
	//m_zoomInButton->SetShortcut(SDLK_F6, KMOD_NONE);
	m_zoomInButton->SetToolTip("Zoom in");
	Add(m_zoomInButton, 700, 5);
	
	m_zoomOutButton = new Gui::ImageButton(PIONEER_DATA_DIR "/icons/zoom_out_f8.png");
	//m_zoomOutButton->SetShortcut(SDLK_F7, KMOD_NONE);
	m_zoomOutButton->SetToolTip("Zoom out");
	Add(m_zoomOutButton, 732, 5);
	
	m_scaleReadout = new Gui::Label("");
	Add(m_scaleReadout, 500.0f, 10.0f);

	m_labels = new Gui::LabelSet();
	Add(m_labels, 0, 0);

	m_onMouseButtonDown = 
		Pi::onMouseButtonDown.connect(sigc::mem_fun(this, &GalacticView::MouseButtonDown));
}

GalacticView::~GalacticView()
{
	glDeleteTextures(1, &m_texture);
	m_onMouseButtonDown.disconnect();
}

void GalacticView::Save(Serializer::Writer &wr)
{
}

void GalacticView::Load(Serializer::Reader &rd)
{
}

/*void GalacticView::OnClickSystemInfo()
{
	Pi::SetView(Pi::systemInfoView);
}*/

struct galaclabel_t {
	const char *label;
	vector3d pos;
} s_labels[] = {
	{ "Norma arm", vector3d(0.0,-0.3,0.0) },
	{ "Persius arm", vector3d(0.57,0.0,0.0) },
	{ "Outer arm", vector3d(0.65,0.4,0.0) },
	{ "Sagittarius arm", vector3d(-.3,0.2,0.0) },
	{ "Scutum-Centaurus arm", vector3d(-.45,-0.45,0.0) },
	{ 0 }
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
			m_labels->Add(s_labels[i].label, sigc::ptr_fun(&dummy), (float)pos.x, (float)pos.y);
		}
	}

	Gui::Screen::LeaveOrtho();
	glDisable(GL_LIGHTING);			// what
}

float densityfunc(const vector3f &v)
{
	return 
		(0.5f + 0.5f*fabs(noise(10.0f*v))) * (
		// galactic disk
		0.25f * std::max(1.0f - vector3f(v.x, v.y, v.z * 20.0).Length(), 0.0f) +
		// galactic core
		0.75f * std::max(1.0f - vector3f(v.x*4.0, v.y*4.0, v.z * 6.0).Length(), 0.0f) +
		// galactic halo
		0.02f * std::max(1.0f - vector3f(v.x, v.y, v.z * 1.5f).Length(), 0.0f)
		);
}

bool findSphereEyeRayEntryDistance(vector3d sphereCenter, vector3d eyeDir, float radius, float &entryDist, float &exitDist)
{
	vector3d v = -sphereCenter;
	float b = -v.Dot(eyeDir);
	float det = (b * b) - v.Dot(v) + (radius * radius);
	bool retval = false;
	if (det > 0.0f) {
		det = sqrt(det);
		float i1 = b - det;
		float i2 = b + det;
		if (i2 > 0.0f) {
			entryDist = std::max(i1, 0.0f);
			exitDist = i2;
			retval = true;
		}
	}
	return retval;
}

void GalacticView::Draw3D()
{
	int secx, secy;
	Pi::sectorView->GetSector(&secx, &secy);
	float offset_x = (secx*Sector::SIZE + Galaxy::SOL_OFFSET_X)/Galaxy::GALAXY_RADIUS;
	float offset_y = (-secy*Sector::SIZE + Galaxy::SOL_OFFSET_Y)/Galaxy::GALAXY_RADIUS;

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

#define GAL_SIZE 256
	Uint8 *crap = new Uint8[GAL_SIZE*GAL_SIZE];
	vector3d campos(0.0,0.0,5.0);
	vector3d topleft(-1.0,1.0,3.0);
	vector3d topright(1.0,1.0,3.0);
	vector3d botleft(-1.0,-1.0,3.0);
	matrix4x4d rot;
	rot = matrix4x4d::RotateXMatrix(m_rot_x*0.01f);
	rot = rot * matrix4x4d::RotateZMatrix(m_rot_z*0.01f);
	for (int y=0; y<GAL_SIZE; y++) {
		for (int x=0; x<GAL_SIZE; x++) {

			vector3d eyeray = topleft +
				         (topright-topleft) * (x / (GAL_SIZE-1.0f)) +
					 (botleft-topleft) * (y / (GAL_SIZE-1.0f));
			eyeray = eyeray.Normalized();
			float entryDist, exitDist;
			if (!findSphereEyeRayEntryDistance(campos, eyeray, 1.0f, entryDist, exitDist)) {
				crap[y*GAL_SIZE + x] = 0;
			} else {
				float d = 0;
				float p = 0;
				for (int i=0; i<30; i++) {
					const vector3d pt = (campos - eyeray*entryDist - eyeray*(p*(exitDist - entryDist))) * rot;
					d += densityfunc(pt);
					p += 1.0f/30.0f;
				}
				d = d / (d + 1.0f);
				d = Clamp(d, 0.0f, 1.0f);
				crap[y*GAL_SIZE + x] = (Uint8)(d * 255.0f);
			}
		}
	}

	GLuint tex;
		glEnable (GL_TEXTURE_2D);
		glGenTextures (1, &tex);
		glBindTexture (GL_TEXTURE_2D, tex);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, GAL_SIZE, GAL_SIZE, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, crap);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

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
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &tex);



//	glDrawPixels(GAL_SIZE, GAL_SIZE, GL_LUMINANCE, GL_UNSIGNED_BYTE, (const void*)crap);
	delete[] crap;
#if 0
	//glScalef(m_zoom, m_zoom, 0.0f);
	glTranslatef(0.0, 0.0, -15.0f);
	
	glRotatef(m_rot_x, 1, 0, 0);
	glRotatef(m_rot_z, 0, 0, 1);
	
	static GLuint tex;
        if (!tex) tex = util_load_tex_rgba("data/textures/smoke.png");
	glEnable(GL_BLEND);
	glDisable(GL_LIGHTING);
	glDepthMask(GL_FALSE);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameterf (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);	
	std::vector<vector3f> verts;
	std::vector<float> dens;
	for (float z = -6.0f; z <= 6.0f; z += 0.1f) {
		for (float x = -6.0f; x <= 6.0f; x += 0.25f) {
			for (float y = -6.0f; y <= 6.0f; y += 0.25f) {
				vector3f v(x,y,z);
				float density = densityfunc(v);
				if (density == 0.0f) continue;
				density *= 0.5f;
				float col[4] = { 1.0f, 1.0f, 1.0f, density };
				verts.push_back(v);
				dens.push_back(density);
				//Render::PutPointSprites(1, &v, 1.5f, col, tex);
			}
		}
	}
	{
		// quad billboards
		matrix4x4f rot;
		glGetFloatv(GL_MODELVIEW_MATRIX, &rot[0]);
		rot.ClearToRotOnly();
		rot = rot.InverseOf();

		const float sz = 0.5f*verts.size();
		const vector3f rotv1 = rot * vector3f(sz, sz, 0.0f);
		const vector3f rotv2 = rot * vector3f(sz, -sz, 0.0f);
		const vector3f rotv3 = rot * vector3f(-sz, -sz, 0.0f);
		const vector3f rotv4 = rot * vector3f(-sz, sz, 0.0f);
		float col[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

		glBegin(GL_QUADS);
		for (int i=0; i<verts.size(); i++) {
			vector3f pos = verts[i];
			vector3f vert;

			col[3] = dens[i];
			glColor4fv(col);

			vert = pos+rotv4;
			glTexCoord2f(0.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv3;
			glTexCoord2f(0.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv2;
			glTexCoord2f(1.0f,1.0f);
			glVertex3f(vert.x, vert.y, vert.z);
			
			vert = pos+rotv1;
			glTexCoord2f(1.0f,0.0f);
			glVertex3f(vert.x, vert.y, vert.z);
		}
		glEnd();
	}

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
#endif
	glEnable(GL_LIGHTING);
	glEnable(GL_DEPTH_TEST);
}
	
void GalacticView::Update()
{
	const float frameTime = Pi::GetFrameTime();
	
	if (m_zoomInButton->IsPressed()) m_zoom *= pow(4.0f, frameTime);
	if (m_zoomOutButton->IsPressed()) m_zoom *= pow(0.25f, frameTime);
	if (Pi::KeyState(SDLK_EQUALS)) m_zoom *= pow(4.0f, frameTime);
	if (Pi::KeyState(SDLK_MINUS)) m_zoom *= pow(0.25f, frameTime);
	m_zoom = Clamp(m_zoom, 0.5f, 100.0f);
	
	if (Pi::MouseButtonState(3)) {
		int motion[2];
		Pi::GetMouseMotion(motion);
		m_rot_x += motion[1];
		m_rot_z += motion[0];
	}

	m_scaleReadout->SetText(stringf(128, "%d ly", (int)(0.5*Galaxy::GALAXY_RADIUS/m_zoom)));
}

void GalacticView::MouseButtonDown(int button, int x, int y)
{
	const float ft = Pi::GetFrameTime();
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELDOWN)) 
			m_zoom *= pow(0.25f, ft);
	if (Pi::MouseButtonState(SDL_BUTTON_WHEELUP)) 
			m_zoom *= pow(4.0f, ft);
}

