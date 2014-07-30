// Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SFX_H
#define _SFX_H

#include "Body.h"
#include "Serializer.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"

class Frame;
namespace Graphics {
	class Renderer;
	namespace Drawables {
		class Sphere3D;
	}
}

class Sfx {
public:
	enum TYPE { TYPE_NONE, TYPE_EXPLOSION, TYPE_DAMAGE, TYPE_SMOKE };

	static void Add(const Body *, TYPE);
	static void AddExplosion(Body *, TYPE);
	static void AddThrustSmoke(const Body *b, TYPE, float speed, vector3d adjustpos);
	static void TimeStepAll(const float timeStep, Frame *f);
	static void RenderAll(Graphics::Renderer *r, Frame *f, const Frame *camFrame);
	static void Serialize(Serializer::Writer &wr, const Frame *f);
	static void Unserialize(Serializer::Reader &rd, Frame *f);

	Sfx();
	void SetPosition(const vector3d &p);
	vector3d GetPosition() const { return m_pos; }

	//create shared models
	static void Init(Graphics::Renderer *r);
	static void Uninit();
	static std::unique_ptr<Graphics::Material> damageParticle;
	static std::unique_ptr<Graphics::Material> ecmParticle;
	static std::unique_ptr<Graphics::Material> smokeParticle;
	static std::unique_ptr<Graphics::Material> explosionParticle;
	static Graphics::RenderState *alphaState;
	static Graphics::RenderState *additiveAlphaState;
	static Graphics::RenderState *alphaOneState;

private:
	static Sfx *AllocSfxInFrame(Frame *f);
	static const Uint32 NUM_EXPLOSION_TEXTURES = 32;
	static Graphics::Texture* explosionTextures[NUM_EXPLOSION_TEXTURES];

	void Render(Graphics::Renderer *r, const matrix4x4d &transform);
	void TimeStepUpdate(const float timeStep);
	void Save(Serializer::Writer &wr);
	void Load(Serializer::Reader &rd);

	vector3d m_pos;
	vector3d m_vel;
	float m_age;
	float m_speed;
	enum TYPE m_type;
};

#endif /* _SFX_H */
