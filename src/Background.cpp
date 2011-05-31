#include "Background.h"
#include "perlin.h"
#include "Pi.h"
#include "StarSystem.h"
#include "Space.h"
#include "Frame.h"
#include "Player.h"
#include <vector>

namespace Background
{

Starfield::Starfield() :
	m_shader(0)
{
	//This is needed because there is no system seed for the main menu
	unsigned long seed = Pi::IsGameStarted() ? Pi::currentSystem->m_seed : UNIVERSE_SEED;
	
	// Slight colour variation to stars based on seed
	MTRand rand(seed);

	//fill the array
	for (int i=0; i<BG_STAR_MAX; i++) {
		float col = float(rand.Double(0,1));

		col *= col * col * 3.0;
		col = (col > 0.725 ? 1.45-col : col);
		col = Clamp(col, 0.00f, 0.725f);

		if (i<6) {
			col = 0.9;
		} else if (i<21) {
			col = 0.85;
		} else if (i<46) {
			col = 0.8;
		}

		m_stars[i].r = rand.Double(col-0.05f,col);
		m_stars[i].g = rand.Double(col-0.1f,m_stars[i].r);
		m_stars[i].b = rand.Double(col-0.05f,col);

		// this is proper random distribution on a sphere's surface
		// XXX TODO
		// perhaps distribute stars to give greater density towards the galaxy's centre and in the galactic plane?
		const float theta = float(rand.Double(0.0, 2.0*M_PI));
		const float u = float(rand.Double(-1.0, 1.0));

		m_stars[i].x = 1000.0f * sqrt(1.0f - u*u) * cos(theta);
		m_stars[i].y = 1000.0f * u;
		m_stars[i].z = 1000.0f * sqrt(1.0f - u*u) * sin(theta);
	}

	if (USE_VBO) {
		glGenBuffersARB(1, &m_vbo);
		Render::BindArrayBuffer(m_vbo);
		glBufferDataARB(GL_ARRAY_BUFFER, sizeof(Vertex)*BG_STAR_MAX, m_stars, GL_STATIC_DRAW);
		Render::BindArrayBuffer(0);
	}
	m_shader = new Render::Shader("bgstars");
}

Starfield::~Starfield()
{
	if (m_shader) delete m_shader;
	if (USE_VBO)
		glDeleteBuffersARB(1, &m_vbo);
}

void Starfield::Draw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	double hyperspaceAnim = Space::GetHyperspaceAnim();

	if (Render::AreShadersEnabled()) {
		glError();
		Render::State::UseProgram(m_shader);
		glError();
		glEnable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	} else {
		glDisable(GL_POINT_SMOOTH);
		glPointSize(1.0f);
	}

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if (hyperspaceAnim == 0) {
		if (USE_VBO) {
			glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
			glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), 0);
			glColorPointer(3, GL_FLOAT, sizeof(struct Vertex), reinterpret_cast<void *>(3*sizeof(float)));
			glDrawArrays(GL_POINTS, 0, BG_STAR_MAX);
			glBindBufferARB(GL_ARRAY_BUFFER, 0);
		} else {
			glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), &m_stars[0].x);
			glColorPointer(3, GL_FLOAT, sizeof(struct Vertex), &m_stars[0].r);
			glDrawArrays(GL_POINTS, 0, BG_STAR_MAX);
		}
	} else {
		/* HYPERSPACING!!!!!!!!!!!!!!!!!!! */
		/* all this jizz isn't really necessary, since the player will
		 * be in the root frame when hyperspacing... */
		matrix4x4d m, rot;
		Frame::GetFrameTransform(Space::rootFrame, Pi::player->GetFrame(), m);
		m.ClearToRotOnly();
		Pi::player->GetRotMatrix(rot);
		m = rot.InverseOf() * m;
		vector3d pz(m[2], m[6], m[10]);

		// roughly, the multiplier gets smaller as the duration gets larger.
		// the time-looking bits in this are completely arbitrary - I figured
		// it out by tweaking the numbers until it looked sort of right
		double mult = 0.0015 / (Space::GetHyperspaceDuration() / (60.0*60.0*24.0*7.0));

		float *vtx = new float[BG_STAR_MAX*12];
		for (int i=0; i<BG_STAR_MAX; i++) {
			vtx[i*12] = m_stars[i].x;
			vtx[i*12+1] = m_stars[i].y;
			vtx[i*12+2] = m_stars[i].z;

			vtx[i*12+3] = m_stars[i].r * 0.5;
			vtx[i*12+4] = m_stars[i].g * 0.5;
			vtx[i*12+5] = m_stars[i].b * 0.5;

			vector3f v(m_stars[i].x, m_stars[i].y, m_stars[i].z);
			v += pz*hyperspaceAnim*mult;

			vtx[i*12+6] = v.x;
			vtx[i*12+7] = v.y;
			vtx[i*12+8] = v.z;

			vtx[i*12+9] = m_stars[i].r * 0.5;
			vtx[i*12+10] = m_stars[i].g * 0.5;
			vtx[i*12+11] = m_stars[i].b * 0.5;
		}

		glVertexPointer(3, GL_FLOAT, 6*sizeof(float), vtx);
		glColorPointer(3, GL_FLOAT, 6*sizeof(float), vtx+3);
		glDrawArrays(GL_LINES, 0, 2*BG_STAR_MAX);

		delete[] vtx;
	}

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);

	if (Render::AreShadersEnabled()) {
		Render::State::UseProgram(0);
		glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
		glDisable(GL_POINT_SMOOTH);
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
}



MilkyWay::MilkyWay()
{

}

MilkyWay::~MilkyWay()
{
	glDeleteBuffersARB(1, &m_vbo);
}

void MilkyWay::Draw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);

	if (0 /*USE_VBO*/) {

	} else {
		// might be nice to shove this crap in a vbo.
		float theta;
		// make it rotated a bit so star systems are not in the same
		// plane (could make it different per system...
		glBegin(GL_TRIANGLE_STRIP);
		for (theta=0.0; theta < 2.0*M_PI; theta+=0.1) {
			glColor3f(0.0,0.0,0.0);
			glVertex3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta));
			glColor3f(0.05,0.05,0.05);
			glVertex3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta));
		}
		theta = 2.0*M_PI;
		glColor3f(0.0,0.0,0.0);
		glVertex3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta));
		glColor3f(0.05,0.05,0.05);
		glVertex3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta));

		glEnd();
		glBegin(GL_TRIANGLE_STRIP);
		for (theta=0.0; theta < 2.0*M_PI; theta+=0.1) {
			glColor3f(0.05,0.05,0.05);
			glVertex3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta));
			glColor3f(0.0,0.0,0.0);
			glVertex3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta));
		}
		theta = 2.0*M_PI;
		glColor3f(0.05,0.05,0.05);
		glVertex3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta));
		glColor3f(0.0,0.0,0.0);
		glVertex3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta));
		glEnd();
	}

	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

}; //namespace Background
