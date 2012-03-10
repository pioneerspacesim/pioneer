#include "libs.h"
#include "Pi.h"
#include "Sfx.h"
#include "Frame.h"
#include "StarSystem.h"
#include "Space.h"
#include "Pi.h"
#include "graphics/Material.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"

using namespace Graphics;

static const std::string damageTextureFilename(PIONEER_DATA_DIR"/textures/smoke.png");

#define MAX_SFX_PER_FRAME 1024
Graphics::Drawables::Sphere3D *Sfx::shieldEffect = 0;
Graphics::Drawables::Sphere3D *Sfx::explosionEffect = 0;

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

void Sfx::Render(Renderer *renderer, const matrix4x4d &ftransform)
{
	vector3d fpos = ftransform * GetPosition();

	switch (m_type) {
		case TYPE_NONE: break;
		case TYPE_EXPLOSION: {
			//Explosion effect: A quick flash of three concentric coloured spheres. A bit retro.
			glPushMatrix();
			matrix4x4f trans = trans.Identity();
			trans.Translate(fpos.x, fpos.y, fpos.z);
			RefCountedPtr<Material> exmat = Sfx::explosionEffect->GetMaterial();
			exmat->diffuse = Color(1.f, 1.f, 0.5f, 1.f);
			renderer->SetTransform(trans * matrix4x4f::ScaleMatrix(1000*m_age));
			Sfx::explosionEffect->Draw(renderer);
			renderer->SetBlendMode(BLEND_ALPHA);
			exmat->diffuse = Color(1.f, 0.5f, 0.f, 0.66f);
			renderer->SetTransform(trans * matrix4x4f::ScaleMatrix(1500*m_age));
			Sfx::explosionEffect->Draw(renderer);
			exmat->diffuse = Color(1.f, 0.f, 0.f, 0.33f);
			renderer->SetTransform(trans * matrix4x4f::ScaleMatrix(2000*m_age));
			Sfx::explosionEffect->Draw(renderer);
			glPopMatrix();
			break;
		}
		case TYPE_DAMAGE:
			vector3f pos(&fpos.x);
			//XXX no need to recreate material every time
			Material mat;
			mat.texture0 = Graphics::TextureBuilder::Billboard(damageTextureFilename).GetOrCreateTexture(Pi::renderer, "billboard");
			mat.unlit = true;
			mat.diffuse = Color(1.f, 1.f, 0.f, 1.0f-(m_age/2.0f));
			renderer->DrawPointSprites(1, &pos, &mat, 20.f);
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

void Sfx::RenderAll(Renderer *renderer, const Frame *f, const Frame *camFrame)
{
	if (f->m_sfx) {
		matrix4x4d ftran;
		Frame::GetFrameTransform(f, camFrame, ftran);

		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].Render(renderer, ftran);
			}
		}
	}
	
	for (std::list<Frame*>::const_iterator i = f->m_children.begin();
			i != f->m_children.end(); ++i) {
		RenderAll(renderer, *i, camFrame);
	}
}

void Sfx::Init()
{
	//these are identical, but keeping separate anyway
	RefCountedPtr<Graphics::Material> smat(new Graphics::Material);
	smat->unlit = true;
	shieldEffect = new Graphics::Drawables::Sphere3D(smat, 2);
	RefCountedPtr<Graphics::Material> emat(new Graphics::Material);
	emat->unlit = true;
	explosionEffect = new Graphics::Drawables::Sphere3D(emat, 2);
}

void Sfx::Uninit()
{
	delete shieldEffect; shieldEffect = 0;
	delete explosionEffect; explosionEffect = 0;
}
