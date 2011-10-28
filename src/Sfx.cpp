#include "libs.h"
#include "Pi.h"
#include "Sfx.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "render/Render.h"

#define MAX_SFX_PER_FRAME 1024

Sfx::Sfx()
{
	m_type = TYPE_NONE;
}

void Sfx::Save(Serializer::Writer &wr)
{
	wr.Vector3d(m_pos);
	wr.Vector3d(m_vel);
	wr.Float(m_age);
	wr.Int32(m_type);
}

void Sfx::Load(Serializer::Reader &rd)
{
	m_pos = rd.Vector3d();
	m_vel = rd.Vector3d();
	m_age = rd.Float();
	m_type = static_cast<Sfx::TYPE>(rd.Int32());
}

void Sfx::Serialize(Serializer::Writer &wr, const Frame *f)
{
	// how many sfx turds are active in frame?
	int numActive = 0;
	if (f->m_sfx) {
		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) numActive++;
		}
	}
	wr.Int32(numActive);

	if (numActive) for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
		if (f->m_sfx[i].m_type != TYPE_NONE) {
			f->m_sfx[i].Save(wr);
		}
	}
}

void Sfx::Unserialize(Serializer::Reader &rd, Frame *f)
{
	int numActive = rd.Int32();
	if (numActive) {
		f->m_sfx = new Sfx[MAX_SFX_PER_FRAME];
		for (int i=0; i<numActive; i++) {
			f->m_sfx[i].Load(rd);
		}
	}
}

void Sfx::SetPosition(vector3d p)
{
	m_pos = p;
}

void Sfx::TimeStepUpdate(const float timeStep)
{
	m_age += timeStep;
	m_pos += m_vel * double(timeStep);

	switch (m_type) {
		case TYPE_EXPLOSION:
			if (m_age > 0.2) m_type = TYPE_NONE;
			break;
		case TYPE_DAMAGE:
			if (m_age > 2.0) m_type = TYPE_NONE;
			break;
		case TYPE_NONE: break;
	}
}

void Sfx::Render(const matrix4x4d &ftransform)
{
	static GLuint tex;
	float col[4];
	if (!tex) tex = util_load_tex_rgba(PIONEER_DATA_DIR"/textures/smoke.png");

	vector3d fpos = ftransform * GetPosition();

	switch (m_type) {
		case TYPE_NONE: break;
		case TYPE_EXPLOSION:
			glPushMatrix();
			glTranslatef(fpos.x, fpos.y, fpos.z);
			glPushAttrib(GL_LIGHTING_BIT | GL_COLOR_BUFFER_BIT);
			glDisable(GL_LIGHTING);
			glColor3f(1,1,0.5);
			gluSphere(Pi::gluQuadric, 1000*m_age, 20,20);
			glEnable(GL_BLEND);
			glColor4f(1,0.5,0,0.66);
			gluSphere(Pi::gluQuadric, 1500*m_age, 20,20);
			glColor4f(1,0,0,0.33);
			gluSphere(Pi::gluQuadric, 2000*m_age, 20,20);
			glPopAttrib();
			glPopMatrix();
			break;
		case TYPE_DAMAGE:
			col[0] = 1.0f;
			col[1] = 1.0f;
			col[2] = 0.0f;
			col[3] = 1.0f-(m_age/2.0f);
			vector3f pos(&fpos.x);
			glBindTexture(GL_TEXTURE_2D, tex);
			Render::PutPointSprites(1, &pos, 20.0f, col);
			break;
	}
}

Sfx *Sfx::AllocSfxInFrame(Frame *f)
{
	if (!f->m_sfx) {
		f->m_sfx = new Sfx[MAX_SFX_PER_FRAME];
	}

	for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
		if (f->m_sfx[i].m_type == TYPE_NONE) {
			return &f->m_sfx[i];
		}
	}
	return 0;
}

void Sfx::Add(const Body *b, TYPE t)
{
	Sfx *sfx = AllocSfxInFrame(b->GetFrame());
	if (!sfx) return;

	sfx->m_type = t;
	sfx->m_age = 0;
	sfx->SetPosition(b->GetPosition());
	sfx->m_vel = b->GetVelocity() + 200.0*vector3d(
			Pi::rng.Double()-0.5,
			Pi::rng.Double()-0.5,
			Pi::rng.Double()-0.5);
}

void Sfx::TimeStepAll(const float timeStep, Frame *f)
{
	if (f->m_sfx) {
		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].TimeStepUpdate(timeStep);
			}
		}
	}
	
	for (std::list<Frame*>::iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		TimeStepAll(timeStep, *i);
	}
}

void Sfx::RenderAll(const Frame *f, const Frame *camFrame)
{
	if (f->m_sfx) {
		matrix4x4d ftran;
		Frame::GetFrameTransform(f, camFrame, ftran);

		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].Render(ftran);
			}
		}
	}
	
	for (std::list<Frame*>::const_iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		RenderAll(*i, camFrame);
	}
}

