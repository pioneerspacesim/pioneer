#include "Background.h"
#include "perlin.h"
#include "Pi.h"
#include "StarSystem.h"
#include "Space.h"
#include "Frame.h"
#include "Player.h"
#include <vector>
#include "Game.h"
#include "render/Renderer.h"

namespace Background
{

Starfield::Starfield()
{
	//starfield is not created without a seed
	glGenBuffersARB(1, &m_vbo);
	m_shader = new Render::Shader("bgstars");
}

Starfield::Starfield(unsigned long seed)
{
	glGenBuffersARB(1, &m_vbo);
	Fill(seed);
	m_shader = new Render::Shader("bgstars");
}

Starfield::~Starfield()
{
	if (m_shader) delete m_shader;
	glDeleteBuffersARB(1, &m_vbo);
}

void Starfield::Fill(unsigned long seed)
{
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
		const float theta = float(rand.Double(0.0, 2.0*M_PI));
		const float u = float(rand.Double(-1.0, 1.0));

		m_stars[i].x = 1000.0f * sqrt(1.0f - u*u) * cos(theta);
		m_stars[i].y = 1000.0f * u;
		m_stars[i].z = 1000.0f * sqrt(1.0f - u*u) * sin(theta);
	}

	Render::BindArrayBuffer(m_vbo);
	glBufferDataARB(GL_ARRAY_BUFFER, sizeof(Vertex)*BG_STAR_MAX, m_stars, GL_STATIC_DRAW);
	Render::BindArrayBuffer(0);
}

void Starfield::Draw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);

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

	// XXX would be nice to get rid of the Pi:: stuff here
	if (!Pi::game || Pi::player->GetFlightState() != Ship::HYPERSPACE) {
		glBindBufferARB(GL_ARRAY_BUFFER, m_vbo);
		glVertexPointer(3, GL_FLOAT, sizeof(struct Vertex), 0);
		glColorPointer(3, GL_FLOAT, sizeof(struct Vertex), reinterpret_cast<void *>(3*sizeof(float)));
		glDrawArrays(GL_POINTS, 0, BG_STAR_MAX);
		glBindBufferARB(GL_ARRAY_BUFFER, 0);
	} else {
		/* HYPERSPACING!!!!!!!!!!!!!!!!!!! */
		/* all this jizz isn't really necessary, since the player will
		 * be in the root frame when hyperspacing... */
		matrix4x4d m, rot;
		Frame::GetFrameTransform(Pi::game->GetSpace()->GetRootFrame(), Pi::player->GetFrame(), m);
		m.ClearToRotOnly();
		Pi::player->GetRotMatrix(rot);
		m = rot.InverseOf() * m;
		vector3d pz(m[2], m[6], m[10]);

		// roughly, the multiplier gets smaller as the duration gets larger.
		// the time-looking bits in this are completely arbitrary - I figured
		// it out by tweaking the numbers until it looked sort of right
		double mult = 0.0015 / (Pi::player->GetHyperspaceDuration() / (60.0*60.0*24.0*7.0));

		double hyperspaceProgress = Pi::game->GetHyperspaceProgress();

		float *vtx = new float[BG_STAR_MAX*12];
		for (int i=0; i<BG_STAR_MAX; i++) {
			
			vector3f v(m_stars[i].x, m_stars[i].y, m_stars[i].z);
			v += vector3f(pz*hyperspaceProgress*mult);

			vtx[i*12] = m_stars[i].x + v.x;
			vtx[i*12+1] = m_stars[i].y + v.y;
			vtx[i*12+2] = m_stars[i].z + v.z;

			vtx[i*12+3] = m_stars[i].r;// * noise(v.x);
			vtx[i*12+4] = m_stars[i].g;// * noise(v.y);
			vtx[i*12+5] = m_stars[i].b;// * noise(v.z);

			vtx[i*12+6] = v.x;
			vtx[i*12+7] = v.y;
			vtx[i*12+8] = v.z;

			vtx[i*12+9] = m_stars[i].r;// * noise(v.x);
			vtx[i*12+10] = m_stars[i].g;// * noise(v.y);
			vtx[i*12+11] = m_stars[i].b;// * noise(v.z);

			//glRotatef(0.00001*v.x, 1.0f, 0.0f, 0.0f); // rotate around x axis
			//glRotatef(-0.00001*v.y, 0.0f, 1.0f, 0.0f); // rotate around y axis
			//glRotatef(0.00001*v.z, 0.0f, 0.0f, 1.0f); // rotate around z axis
		}

		glVertexPointer(3, GL_FLOAT, 6*sizeof(float), vtx);
		glColorPointer(3, GL_FLOAT, 6*sizeof(float), vtx+3);
		glDrawArrays(GL_LINES, 0, BG_STAR_MAX*2);

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
	m_model = new BufferThing();
	m_model->numSurfaces = 2;
	m_model->surfaces = new Surface[2];

	//build milky way model in two strips (about 256 verts)
	//The model is built as a generic vertex array first. The renderer
	//will reprocess this into buffered format as it sees fit, the old
	//data can then be thrown away (or not. If the cache is emptied the model
	//needs to be regenerated somehow).

	VertexArray *bottom = new VertexArray();
	VertexArray *top = new VertexArray();
	m_model->surfaces[0].vertices = bottom;
	m_model->surfaces[1].vertices = top;

	const Color dark(0.f);
	const Color bright(0.05,0.05f, 0.05f, 0.05f);

	//bottom
	float theta;
	for (theta=0.0; theta < 2.0*M_PI; theta+=0.1) {
		bottom->Add(
				vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
				dark);
		bottom->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
	}
	theta = 2.0*M_PI;
	bottom->Add(
		vector3f(100.0f*sin(theta), float(-40.0 - 30.0*noise(sin(theta),1.0,cos(theta))), 100.0f*cos(theta)),
		dark);
	bottom->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	//top
	for (theta=0.0; theta < 2.0*M_PI; theta+=0.1) {
		top->Add(
			vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
			bright);
		top->Add(
			vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
			dark);
	}
	theta = 2.0*M_PI;
	top->Add(
		vector3f(100.0f*sin(theta), float(5.0*noise(sin(theta),0.0,cos(theta))), 100.0f*cos(theta)),
		bright);
	top->Add(
		vector3f(100.0f*sin(theta), float(40.0 + 30.0*noise(sin(theta),-1.0,cos(theta))), 100.0f*cos(theta)),
		dark);

	assert(m_model->surfaces[0].vertices->GetNumVerts() > 0);
	assert(m_model->surfaces[1].vertices->GetNumVerts() > 0);
}

MilkyWay::~MilkyWay()
{
	//XXX release bufferthing (or let renderer do it?)
	delete m_model;
}

void MilkyWay::Draw()
{
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_LIGHTING);
	assert(m_model != 0);
	Pi::renderer->DrawBufferThing(m_model);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
}

Container::Container()
{
}

Container::Container(unsigned long seed)
{
	Refresh(seed);
};

void Container::Refresh(unsigned long seed)
{
	// redo starfield, milkyway stays normal for now
	m_starField.Fill(seed);
}

void Container::Draw(const matrix4x4d &transform) const
{
	glPushMatrix();
	glMultMatrixd(&transform[0]);
	const_cast<MilkyWay&>(m_milkyWay).Draw();
	glPushMatrix();
	// squeeze the starfield a bit to get more density near horizon
	glScalef(1.f, 0.4f, 1.f);
	const_cast<Starfield&>(m_starField).Draw();
	glPopMatrix();
	glPopMatrix();
}

}; //namespace Background
