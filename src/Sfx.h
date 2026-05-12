// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _SFX_H
#define _SFX_H

#include "Color.h"
#include "FrameId.h"
#include "JsonFwd.h"
#include "graphics/Material.h"

#include <SDL_stdinc.h>

#include <deque>

class Body;
class Camera;
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
	inline constexpr float EXHAUST_TIME_BEFORE_SPREAD = 0.1f;
	inline constexpr float EXHAUST_MAX_SPREAD = 100.0f;
	inline constexpr float EXHAUST_DUST_SIZE = 200.0f;
	inline constexpr float EXHAUST_LIFETIME = 10.0f;
	inline constexpr float EXHAUST_WIND_SPEED = 32.0f;
	inline constexpr float EXHAUST_PARTICLES_PER_SHIP_PER_SEC = 1800.0f;
	inline constexpr float EXHAUST_MIN_REACTION_POWER = 0.02f;
	inline constexpr float EXHAUST_STREAM_TIMESTEP_CAP = 0.03f;
	inline constexpr float EXHAUST_DUST_RADIAL_KICK_SPEED = 24.0f;
	inline constexpr float EXHAUST_DUST_TANGENT_KICK_SPEED = 38.0f;
	inline constexpr float EXHAUST_DUST_LOWEST_NON_CULL_PROB = 0.05f;	// Keep at least one in 20 particles
	inline constexpr float EXHAUST_LOG_SCALE = 40.0f;
	inline constexpr float EXHAUST_ANGULAR_FACTOR = 0.2f;
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
	// Thruster exhaust jets have a "backbone" which is the centre of the stream. This is used
	// to track how stretched out each particle should be, and in which direction.
	vector3d m_backbonePos;
	vector3d m_backboneVel;
	vector3d m_backboneAtStepStart;
	// Each particle then has a perpendicular offset from the backbone, which grows over time
	// and gives the exhaust plume its conical shape
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
	Uint32 m_exhaustBirthSeq; // For sort order within a stream
	// If true this particle should not streak towards the previous one, usually this is because it's the first particle in a new thruster pulse
	bool m_exhaustSuppressStreakElongation;
	double m_exhaustGroundRadius; // The height of the ground below this exhaust, calculated from the ground below the ship at spawn time
	bool m_exhaustDustKick; // After the exhaust hits the ground, it can turn into a dust cloud particle
	Color m_exhaustDustTint; // Dust tint and mix amount, calculated from the ground below the ship at spawn time.
	static constexpr Uint32 INVALID_EXHAUST_SAVED_BODY_IDX = Uint32(0xffffffffu);
};

class SfxManager {
public:
	friend struct Sfx;

	static void Add(const Body *, SFX_TYPE);
	static void AddExplosion(Body *);
	static void AddThrustSmoke(const Body *b, float speed, const vector3d &adjustpos);
	static void AddExhaust(const Body *b, Uint16 exhaustJetIndex, bool exhaustSuppressStreakElongation, const vector3d &backboneAdjustPos, const vector3d &backboneVel, const vector3d &plumeOffset, const vector3d &plumeOffsetVel, float intensity, float dragScale, float opacityScale, const vector3d &windVel, double groundRadius, const Color &dustTint);
	static void TimeStepAll(const float timeStep, FrameId f);
	static void RenderAll(Graphics::Renderer *r, FrameId f, FrameId camFrame, const Camera *camera = nullptr, float exhaustIllumMul = 1.f);
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
