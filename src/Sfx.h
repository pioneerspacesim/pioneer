// Copyright © 2008-2016 Pioneer Developers. See AUTHORS.txt for details
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
}

const enum SFX_TYPE { TYPE_EXPLOSION=1, TYPE_DAMAGE, TYPE_SMOKE, TYPE_NONE };
static const Uint32 NUM_EXPLOSION_TEXTURES = 32;

class Sfx {
public:
	
	friend class SfxManager;
	Sfx();
	Sfx(vector3d &pos, vector3d &vel, float speed, SFX_TYPE type);
	void SetPosition(const vector3d &p);
	const vector3d& GetPosition() const { return m_pos; }

private:

	void Render(Graphics::Renderer *r, const matrix4x4d &transform);
	void TimeStepUpdate(const float timeStep);
	void SaveToJson(Json::Value &jsonObj);
	void LoadFromJson(const Json::Value &jsonObj);

	vector3d m_pos;
	vector3d m_vel;
	float m_age;
	float m_speed;
	enum SFX_TYPE m_type;
};


class SfxManager {
public:
	friend class Sfx;

	static void Add(const Body *, SFX_TYPE);
	static void AddExplosion(Body *);
	static void AddThrustSmoke(const Body *b, float speed, const vector3d &adjustpos);
	static void TimeStepAll(const float timeStep, Frame *f);
	static void RenderAll(Graphics::Renderer *r, Frame *f, const Frame *camFrame);
	static void ToJson(Json::Value &jsonObj, const Frame *f);
	static void FromJson(const Json::Value &jsonObj, Frame *f);

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

	size_t GetNumberInstances(const SFX_TYPE t) const { return m_instances[t].size(); }
	Sfx& GetInstanceByIndex(const SFX_TYPE t, const size_t i) { return m_instances[t][i]; }
	void AddInstance(Sfx &inst) { return m_instances[inst.m_type].push_back(inst); }
	void Cleanup();

private:
	static SfxManager *AllocSfxInFrame(Frame *f);
	static Graphics::Texture* explosionTextures[NUM_EXPLOSION_TEXTURES];

	// per-frame
	std::deque<Sfx> m_instances[TYPE_NONE];
};

#endif /* _SFX_H */
