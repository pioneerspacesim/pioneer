// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sfx.h"
#include "Frame.h"
#include "galaxy/StarSystem.h"
#include "libs.h"
#include "Pi.h"
#include "Ship.h"
#include "Space.h"
#include "StringF.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"

using namespace Graphics;

static const int MAX_SFX_PER_FRAME = 1024;

std::unique_ptr<Graphics::Material> Sfx::damageParticle;
std::unique_ptr<Graphics::Material> Sfx::ecmParticle;
std::unique_ptr<Graphics::Material> Sfx::smokeParticle;
std::unique_ptr<Graphics::Material> Sfx::explosionParticle;
Graphics::RenderState *Sfx::alphaState = nullptr;
Graphics::RenderState *Sfx::additiveAlphaState = nullptr;
Graphics::RenderState *Sfx::alphaOneState = nullptr;

Graphics::Texture* Sfx::explosionTextures[Sfx::NUM_EXPLOSION_TEXTURES];

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

void Sfx::SetPosition(const vector3d &p)
{
	m_pos = p;
}

void Sfx::TimeStepUpdate(const float timeStep)
{
	PROFILE_SCOPED()
	m_age += timeStep;
	m_pos += m_vel * double(timeStep);

	switch (m_type) {
		case TYPE_EXPLOSION:
			//if (m_age > 0.5) m_type = TYPE_NONE;
			if (m_age > 3.2) m_type = TYPE_NONE;
			break;
		case TYPE_DAMAGE:
			if (m_age > 2.0) m_type = TYPE_NONE;
			break;
		case TYPE_SMOKE:
			if (m_age > 8.0)
				m_type = TYPE_NONE;
			break;
		case TYPE_NONE: break;
	}
}

void Sfx::Render(Renderer *renderer, const matrix4x4d &ftransform)
{
	PROFILE_SCOPED()
	vector3d fpos = ftransform * GetPosition();
	vector3f pos(&fpos.x);

	switch (m_type) 
	{
		case TYPE_NONE: break;
		case TYPE_EXPLOSION: 
		{
			renderer->SetTransform(matrix4x4d::Translation(fpos));
			const int spriteframe = Clamp( Uint32(m_age*20.0f), Uint32(0), NUM_EXPLOSION_TEXTURES-1 );
			assert(explosionTextures[spriteframe]);
			explosionParticle->texture0 = explosionTextures[spriteframe];
			//face camera
			renderer->SetTransform(matrix4x4f::Identity());
			renderer->DrawPointSprites(1, &pos, alphaOneState, explosionParticle.get(), m_speed);
			break;
		} 
		case TYPE_DAMAGE: 
		{
			renderer->SetTransform(matrix4x4d::Translation(fpos));
			damageParticle->diffuse = Color(255, 255, 0, (1.0f-(m_age/2.0f))*255);
			renderer->DrawPointSprites(1, &pos, additiveAlphaState, damageParticle.get(), 20.f);
			break;
		} 
		case TYPE_SMOKE: 
		{
			float var = Pi::rng.Double()*0.05f; //slightly variation to trail color
			if (m_age < 0.5) { //start trail
				smokeParticle->diffuse = Color((0.75f-var)*255, (0.75f-var)*255, (0.75f-var)*255, (m_age*0.5-(m_age/2.0f))*255);
			} else { //end trail
				smokeParticle->diffuse = Color((0.75-var)*255, (0.75f-var)*255, (0.75f-var)*255, Clamp(0.5*0.5-(m_age/16.0),0.0,1.0)*255);
			}

			renderer->SetTransform(matrix4x4d::Translation(fpos));

			damageParticle->diffuse*=0.05;
			renderer->DrawPointSprites(1, &pos, alphaState, smokeParticle.get(), (m_speed*m_age));
			break;
		}
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

void Sfx::AddExplosion(Body *b, TYPE t)
{
	Sfx *sfx = AllocSfxInFrame(b->GetFrame());
	if (!sfx) return;

	sfx->m_type = t;
	sfx->m_age = 0;
	sfx->SetPosition(b->GetPosition());
	sfx->m_vel = b->GetVelocity();
	if (b->IsType(Object::SHIP)) {
		Ship *s = static_cast<Ship*>(b);
		sfx->m_speed = s->GetAabb().radius*8.0;
	}
}


void Sfx::AddThrustSmoke(const Body *b, TYPE t, const float speed, vector3d adjustpos)
{
	Sfx *sfx = AllocSfxInFrame(b->GetFrame());
	if (!sfx) return;

	sfx->m_type = t;
	sfx->m_age = 0;
	sfx->m_speed = speed;
	vector3d npos = b->GetPosition();
	sfx->SetPosition(npos+adjustpos);
	sfx->m_vel = vector3d(0,0,0);
}

void Sfx::TimeStepAll(const float timeStep, Frame *f)
{
	PROFILE_SCOPED()
	if (f->m_sfx) {
		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].TimeStepUpdate(timeStep);
			}
		}
	}

	for (Frame* kid : f->GetChildren()) {
		TimeStepAll(timeStep, kid);
	}
}

void Sfx::RenderAll(Renderer *renderer, Frame *f, const Frame *camFrame)
{
	PROFILE_SCOPED()
	if (f->m_sfx) {
		matrix4x4d ftran;
		Frame::GetFrameTransform(f, camFrame, ftran);

		for (int i=0; i<MAX_SFX_PER_FRAME; i++) {
			if (f->m_sfx[i].m_type != TYPE_NONE) {
				f->m_sfx[i].Render(renderer, ftran);
			}
		}
	}

	for (Frame* kid : f->GetChildren()) {
		RenderAll(renderer, kid, camFrame);
	}
}

void Sfx::Init(Graphics::Renderer *r)
{
	//shared render states
	Graphics::RenderStateDesc rsd;
	rsd.blendMode = Graphics::BLEND_ALPHA;
	rsd.depthWrite = false;
	alphaState = r->CreateRenderState(rsd);
	rsd.blendMode = Graphics::BLEND_ALPHA_ONE;
	additiveAlphaState = r->CreateRenderState(rsd);
	rsd.depthWrite = true;
	alphaOneState = r->CreateRenderState(rsd);

	Graphics::MaterialDescriptor desc;
	RefCountedPtr<Graphics::Material> explosionMat(r->CreateMaterial(desc));

	desc.textures = 1;
	damageParticle.reset( r->CreateMaterial(desc) );
	damageParticle->texture0 = Graphics::TextureBuilder::Billboard("textures/smoke.png").GetOrCreateTexture(r, "billboard");
	ecmParticle.reset( r->CreateMaterial(desc) );
	ecmParticle->texture0 = Graphics::TextureBuilder::Billboard("textures/ecm.png").GetOrCreateTexture(r, "billboard");
	smokeParticle.reset( r->CreateMaterial(desc) );
	smokeParticle->texture0 = Graphics::TextureBuilder::Billboard("textures/smoke.png").GetOrCreateTexture(r, "billboard");
	explosionParticle.reset( r->CreateMaterial(desc) );
	explosionParticle->texture0 = Graphics::TextureBuilder::Billboard("textures/smoke.png").GetOrCreateTexture(r, "billboard");

	// NB: 0-31
	for( Uint32 i=0 ; i<NUM_EXPLOSION_TEXTURES ; i++ )
	{
		const std::string fname(stringf("textures/explosions/image%0.png", i));
		explosionTextures[i] = Graphics::TextureBuilder::Billboard(fname).GetOrCreateTexture(r, "billboard");
	}
}

void Sfx::Uninit()
{
	damageParticle.reset();
	ecmParticle.reset();
	smokeParticle.reset();
	explosionParticle.reset();
}
