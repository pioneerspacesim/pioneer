// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SFX_H
#define _SFX_H

#include "FrameId.h"
#include "JsonFwd.h"
#include "graphics/Material.h"

#include <SDL_stdinc.h>

#include <deque>

class Body;
class Frame;
class Space;

namespace Graphics {
	class Renderer;
} // namespace Graphics

enum SFX_TYPE {
	TYPE_EXPLOSION = 1,
	TYPE_DAMAGE,
	TYPE_SMOKE,
	TYPE_EXHAUST,
	TYPE_NONE
};

namespace SfxParams {
	inline constexpr float EXHAUST_MAX_PLAYER_DISTANCE = 5000.0f;
	inline constexpr float EXHAUST_INITIAL_VELOCITY = 320.0f;
	inline constexpr float EXHAUST_INITIAL_SPREAD = 0.1f;
	inline constexpr float EXHAUST_MAX_SPREAD = 100.0f;
	inline constexpr float EXHAUST_LIFETIME = 10.0f;
	inline constexpr float EXHAUST_WIND_SPEED = 32.0f;
	inline constexpr float EXHAUST_PARTICLES_PER_SEC = 120.0f;
	// Normalized reaction power ([0,1] from joystick / autopilot) needed before spawning plume
	inline constexpr float EXHAUST_MIN_REACTION_POWER = 0.06f;
}

struct Sfx {
	Sfx() = delete;
	Sfx(const vector3d &pos, const vector3d &vel, const float speed, const SFX_TYPE type);
	Sfx(const Json &jsonObj);

	const vector3d &GetPosition() const { return m_pos; }

	float AgeBlend() const;
	void SetPosition(const vector3d &p);

	void TimeStepUpdate(const float timeStep);
	void SaveToJson(Json &jsonObj, const Space *space);

	vector3d m_pos;
	vector3d m_vel;
	vector3d m_backbonePos;
	vector3d m_backboneVel;
	// Start-of-current-physics-tick backbone (TYPE_EXHAUST); used for render interp like DynamicBody::m_oldPos.
	vector3d m_backboneAtStepStart;
	vector3d m_plumeOffset;
	vector3d m_plumeOffsetVel;
	float m_age;
	float m_speed;
	float m_seed;
	float m_lifetime;
	float m_dragScale;
	float m_opacityScale;
	vector3d m_windVel;
	enum SFX_TYPE m_type;

	// Atmospheric exhaust: streak direction chains consecutive spawns per thruster only.
	// Runtime grouping uses emitting Body pointer (stable for object lifetime); savegames store
	// Space body index separately because pointers are not serialized.
	const Body *m_exhaustEmitter;
	Uint32 m_exhaustSavedEmitterBodyIdx;
	Uint16 m_exhaustJetIndex;
	Uint32 m_exhaustBirthSeq; // monotonic per-frame SfxManager for sort order within a stream
	// If true, billboard streak does not use backbone delta vs previous particle (new thrust pulse opener).
	bool m_exhaustSuppressStreakElongation;
	static constexpr Uint32 INVALID_EXHAUST_SAVED_BODY_IDX = Uint32(0xffffffffu);
};

class SfxManager {
public:
	friend struct Sfx;

	static void Add(const Body *, SFX_TYPE);
	static void AddExplosion(Body *);
	static void AddThrustSmoke(const Body *b, float speed, const vector3d &adjustpos);
	static void AddExhaust(const Body *b, Uint16 exhaustJetIndex, bool exhaustSuppressStreakElongation, const vector3d &backboneAdjustPos, const vector3d &backboneVel, const vector3d &plumeOffset, const vector3d &plumeOffsetVel, float intensity, float dragScale, float opacityScale, const vector3d &windVel);
	static void TimeStepAll(const float timeStep, FrameId f);
	static void RenderAll(Graphics::Renderer *r, FrameId f, const FrameId camFrame);
	static void ToJson(Json &jsonObj, const FrameId f, const Space *space);
	static void FromJson(const Json &jsonObj, FrameId f);

	//create shared models
	static void Init(Graphics::Renderer *r);
	static void Uninit();
	static std::unique_ptr<Graphics::Material> damageParticle;
	static std::unique_ptr<Graphics::Material> ecmParticle;
	static std::unique_ptr<Graphics::Material> smokeParticle;
	static std::unique_ptr<Graphics::Material> exhaustParticle;
	static std::unique_ptr<Graphics::Material> explosionParticle;

	SfxManager();

	size_t GetNumberInstances(const SFX_TYPE t) const { return m_instances[t].size(); }
	void Cleanup();

private:
	// types
	struct MaterialData {
		MaterialData() :
			effect(Graphics::EFFECT_BILLBOARD),
			num_textures(1),
			num_imgs_wide(1),
			coord_downscale(1.0f) {}
		Graphics::EffectType effect;
		Uint32 num_textures;
		int num_imgs_wide;
		float coord_downscale;
	};

	Sfx &GetInstanceByIndex(const SFX_TYPE t, const size_t i) { return m_instances[t][i]; }
	void AddInstance(Sfx &inst) { m_instances[inst.m_type].push_back(inst); }

	// methods
	static SfxManager *AllocSfxInFrame(FrameId f);
	static vector2f CalculateOffset(const enum SFX_TYPE, const Sfx &);
	static bool SplitMaterialData(const std::string &spec, MaterialData &output);

	// static members
	static MaterialData m_materialData[TYPE_NONE];

	// members
	// per-frame
	std::deque<Sfx> m_instances[TYPE_NONE];
	Uint32 m_nextExhaustBirthSeq = 1;
};

#endif /* _SFX_H */
