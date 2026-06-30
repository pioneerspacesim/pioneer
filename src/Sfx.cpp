// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "Sfx.h"

#include "Body.h"
#include "Camera.h"
#include "FileSystem.h"
#include "Frame.h"
#include "Game.h"
#include "GameSaveError.h"
#include "Json.h"
#include "JsonUtils.h"
#include "MathUtil.h"
#include "ModelBody.h"
#include "Pi.h"
#include "Player.h"
#include "Space.h"
#include "matrix3x3.h"
#include "matrix4x4.h"

#include "core/IniConfig.h"
#include "graphics/Drawables.h"
#include "graphics/Graphics.h"
#include "graphics/Material.h"
#include "graphics/RenderState.h"
#include "graphics/Renderer.h"
#include "graphics/TextureBuilder.h"
#include "graphics/Types.h"
#include "graphics/VertexArray.h"
#include "graphics/VertexBuffer.h"

#include "profiler/Profiler.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <map>
#include <vector>

using namespace Graphics;

namespace {
	constexpr uint64_t EXHAUST_EMITTER_PTR_SORT_TAG = UINT64_C(0x8000000000000000);

	inline uint64_t ExhaustStreamEmitterSortKey(const Sfx &s)
	{
		if (s.m_exhaustEmitter)
			return EXHAUST_EMITTER_PTR_SORT_TAG | static_cast<uint64_t>(reinterpret_cast<uintptr_t>(s.m_exhaustEmitter));
		return static_cast<uint64_t>(s.m_exhaustSavedEmitterBodyIdx);
	}

	struct ExhaustJetStreamKey {
		uint64_t emitterKey;
		Uint16 jetIndex;

		bool operator<(const ExhaustJetStreamKey &other) const
		{
			if (emitterKey != other.emitterKey)
				return emitterKey < other.emitterKey;
			return jetIndex < other.jetIndex;
		}
	};

	// Reused each exhaust draw, cleared at the start of every pass.
	std::map<ExhaustJetStreamKey, std::vector<Sfx *>> s_exhaustByJet;

	// GPU instance record for instanced exhaust quads (40-byte stride, binding 1).
	struct ExhaustInstanceData {
		vector3f center;
		float size;
		vector3f jetVectorCam;
		float backboneCamZ;
		float stretchScale;
		Color color;
	};
	static_assert(sizeof(ExhaustInstanceData) == 40);

	Graphics::VertexFormatDesc BuildExhaustInstancedVertexFormat()
	{
		// Instance-only format, quad corners come from gl_VertexID in the shader.
		Graphics::VertexFormatDesc vtxFormat = {};
		vtxFormat.attribs[0] = { Graphics::ATTRIB_FORMAT_FLOAT4, 6, 0, 0 };
		vtxFormat.attribs[1] = { Graphics::ATTRIB_FORMAT_FLOAT4, 7, 0, 16 };
		vtxFormat.attribs[2] = { Graphics::ATTRIB_FORMAT_FLOAT, 8, 0, 32 };
		vtxFormat.attribs[3] = { Graphics::ATTRIB_FORMAT_UBYTE4, 9, 0, 36 };
		vtxFormat.bindings[0] = { sizeof(ExhaustInstanceData), true, Graphics::ATTRIB_RATE_INSTANCE };
		assert(vtxFormat.ValidateDesc() == Graphics::InvalidVertexFormatReason::OK);
		return vtxFormat;
	}

	float SizeToPixels(int screenHeight, const vector3f &trans, const float size)
	{
		//some hand-tweaked scaling, to make the lights seem larger from distance (final size is in pixels)
		// gl_PointSize = pixels_per_radian * point_diameter / distance( camera, pointcenter );
		// FIXME: this should reference a camera object instead of querying the window height
		const float pixrad = Clamp(screenHeight / trans.Length(), 0.1f, 50.0f);
		return (size * Graphics::GetFovFactor()) * pixrad;
	}

	float GetParticleSize(int screenHeight, SFX_TYPE t, const vector3f &pos, const Sfx &inst)
	{
		switch (t) {
		case TYPE_NONE: assert(false);
		case TYPE_EXPLOSION:
			return SizeToPixels(screenHeight, pos, inst.m_speed);
		case TYPE_DAMAGE:
			return SizeToPixels(screenHeight, pos, 20.f);
		case TYPE_SMOKE:
			return Clamp(SizeToPixels(screenHeight, pos, (inst.m_speed * inst.m_age)), 0.1f, 50.0f);
		case TYPE_EXHAUST:
			return 0.f; // billboards use explicit vertex quads in RenderAll
		default:
			return 0.f;
		}
	}

	void ApplyExhaustGroundThreshold(Sfx &s)
	{
		if (!(s.m_exhaustGroundRadius > 1.0))
			return;

		if (s.m_exhaustDustKick)
			return;	// Already hit the ground

		const vector3d particlePos = s.m_pos + vector3d(s.m_plumeOffset);
		const double radialDistSqr = particlePos.LengthSqr();
		const double groundRadius = s.m_exhaustGroundRadius;
		if (radialDistSqr > groundRadius * groundRadius)
			return;

		// Randomly cull some particles right at exhaust->dust transition, because the
		// dust clouds are large and diffuse and don't need nearly as many particles.
		// The number of particles to cull depends on the thruster power (stored in m_speed).
		if (Pi::rng.Double() > std::max(s.m_speed, SfxParams::EXHAUST_DUST_LOWEST_NON_CULL_PROB)) {
			s.m_type = TYPE_NONE;
			return;
		}

		const double rl = std::sqrt(std::max(radialDistSqr, 1e-24));
		const vector3d rad = particlePos / rl;
		const vector3d surface = rad * groundRadius;

		const vector3d ref(std::abs(rad.y) < 0.92 ? vector3d(0., 1., 0.) : vector3d(1., 0., 0.));
		vector3d east = rad.Cross(ref).NormalizedSafe();
		vector3d north = east.Cross(rad).NormalizedSafe();
		const double ang = Pi::rng.Double(0., 2.0 * M_PI);
		const vector3d tang = (east * std::cos(ang) + north * std::sin(ang)).NormalizedSafe();
		const double tanJ = Pi::rng.Double(0.35, 1.0);
		const double radJ = Pi::rng.Double(0.55, 1.0);

		s.m_pos = surface;
		s.m_plumeOffset = vector3f(0.f);
		s.m_vel = vector3d(0.);
		s.m_plumeOffsetVel =
			vector3f(rad) * (SfxParams::EXHAUST_DUST_RADIAL_KICK_SPEED * s.m_speed * float(radJ)) +
			vector3f(tang) * (SfxParams::EXHAUST_DUST_TANGENT_KICK_SPEED * s.m_speed * float(tanJ));

		s.m_exhaustDustKick = true;
	}
}

std::unique_ptr<Graphics::Material> SfxManager::damageParticle;
std::unique_ptr<Graphics::Material> SfxManager::ecmParticle;
std::unique_ptr<Graphics::Material> SfxManager::smokeParticle;
std::unique_ptr<Graphics::Material> SfxManager::exhaustParticle;
std::unique_ptr<Graphics::Material> SfxManager::explosionParticle;
SfxManager::MaterialData SfxManager::m_materialData[TYPE_NONE];

Sfx::Sfx(const vector3d &pos, const vector3d &vel, const float speed, const SFX_TYPE type) :
	m_pos(pos),
	m_vel(vel),
	m_posAtStepStart(pos),
	m_age(0.0f),
	m_speed(speed),
	m_type(type),
	m_seed(float(Pi::rng.Double())),
	m_lifetime(SfxParams::EXHAUST_LIFETIME),
	m_exhaustSuppressStreakElongation(false),
	m_exhaustDustKick(false),
	m_exhaustJetIndex(0),
	m_plumeOffset(0.f),
	m_plumeOffsetVel(0.f),
	m_exhaustEmitter(nullptr),
	m_exhaustSavedEmitterBodyIdx(Sfx::INVALID_EXHAUST_SAVED_BODY_IDX),
	m_opacityScale(1.0f),
	m_exhaustDustTint(0, 0, 0, 0),
	m_exhaustGroundRadius(0.0),
	m_dragScale(1.0f),
	m_windVel(vector3f::Zero)
{
}

Sfx::Sfx(const Json &jsonObj)
{
	try {
		Json sfxObj = jsonObj["sfx"];

		m_pos = sfxObj["pos"];
		m_vel = sfxObj["vel"];
		m_posAtStepStart = sfxObj.value("posAtStepStart", m_pos);
		m_plumeOffset = vector3f(sfxObj.value("plumeOffset", vector3d::Zero));
		m_plumeOffsetVel = vector3f(sfxObj.value("plumeOffsetVel", vector3d::Zero));
		m_age = sfxObj["age"];
		m_seed = sfxObj.value("seed", float(Pi::rng.Double()));
		m_lifetime = sfxObj.value("lifetime", SfxParams::EXHAUST_LIFETIME);
		m_dragScale = sfxObj.value("dragScale", 1.0f);
		m_opacityScale = sfxObj.value("opacityScale", 1.0f);
		m_windVel = vector3f(sfxObj.value("windVel", vector3d::Zero));
		m_type = sfxObj["type"];
		m_exhaustEmitter = nullptr;
		m_exhaustSavedEmitterBodyIdx = sfxObj.value("exhaustEmitterBodyIdx", Sfx::INVALID_EXHAUST_SAVED_BODY_IDX);
		m_exhaustJetIndex = Uint16(sfxObj.value("exhaustJetIndex", 0));
		m_exhaustSuppressStreakElongation = sfxObj.value("exhaustSuppressStreakElongation", false);
		m_exhaustGroundRadius = sfxObj.value("exhaustGroundRadius", 0.0);
		m_exhaustDustKick = sfxObj.value("exhaustDustKick", false);
		m_exhaustDustTint = sfxObj.value("exhaustDustTint", Color(0, 0, 0, 0));
	} catch (Json::type_error &) {
		throw SavedGameCorruptException();
	}
}

void Sfx::SaveToJson(Json &jsonObj, const Space *space)
{
	Json sfxObj({}); // Create JSON object to contain sfx data.

	sfxObj["pos"] = m_pos;
	sfxObj["vel"] = m_vel;
	if (m_type == TYPE_EXHAUST) {
		sfxObj["posAtStepStart"] = m_posAtStepStart;
		sfxObj["plumeOffset"] = vector3d(m_plumeOffset);
		sfxObj["plumeOffsetVel"] = vector3d(m_plumeOffsetVel);
		sfxObj["lifetime"] = m_lifetime;
		sfxObj["dragScale"] = m_dragScale;
		sfxObj["opacityScale"] = m_opacityScale;
		sfxObj["windVel"] = vector3d(m_windVel);
		Uint32 bodyIdxForSave = m_exhaustSavedEmitterBodyIdx;
		if (m_exhaustEmitter && space && space->IsBodyIndexValid())
			bodyIdxForSave = space->GetIndexForBody(m_exhaustEmitter);
		sfxObj["exhaustEmitterBodyIdx"] = bodyIdxForSave;
		sfxObj["exhaustJetIndex"] = m_exhaustJetIndex;
		sfxObj["exhaustSuppressStreakElongation"] = m_exhaustSuppressStreakElongation;
		sfxObj["exhaustGroundRadius"] = m_exhaustGroundRadius;
		sfxObj["exhaustDustKick"] = m_exhaustDustKick;
		sfxObj["exhaustDustTint"] = m_exhaustDustTint;
	}
	sfxObj["age"] = m_age;
	sfxObj["seed"] = m_seed;
	sfxObj["type"] = m_type;

	jsonObj["sfx"] = sfxObj; // Add sfx object to supplied object.
}

void Sfx::SetPosition(const vector3d &p)
{
	m_pos = p;
}

void Sfx::TimeStepUpdate(const float timeStep)
{
	PROFILE_SCOPED()
	m_age += timeStep;
	if (m_type != TYPE_EXHAUST)
		m_pos += m_vel * double(timeStep);

	switch (m_type) {
	case TYPE_EXPLOSION:
		if (m_age > 3.2) m_type = TYPE_NONE;
		break;
	case TYPE_DAMAGE:
		if (m_age > 2.0) m_type = TYPE_NONE;
		break;
	case TYPE_SMOKE:
		if (m_age > 8.0) m_type = TYPE_NONE;
		break;
	case TYPE_EXHAUST:
		{
			m_posAtStepStart = m_pos;

			const double drag = Clamp(0.9 * double(m_dragScale) * double(timeStep), 0.0, 0.85);
			// Backbone is influenced by wind, the plume offset isn't, so we're not applying the wind twice.
			m_vel += (vector3d(m_windVel) - m_vel) * drag;
			m_plumeOffsetVel -= m_plumeOffsetVel * drag * 0.5;

			m_pos += m_vel * double(timeStep);
			if (m_age > SfxParams::EXHAUST_TIME_BEFORE_SPREAD)
				m_plumeOffset += m_plumeOffsetVel * timeStep;
			ApplyExhaustGroundThreshold(*this);
		}
		if (m_age > m_lifetime) m_type = TYPE_NONE;
		break;
	case TYPE_NONE: break;
	}
}

float Sfx::AgeBlend() const
{
	switch (m_type) {
	case TYPE_EXPLOSION: return (3.2 - m_age) / 3.2;
	case TYPE_DAMAGE: return (2.0 - m_age) / 2.0;
	case TYPE_SMOKE: return (8.0 - m_age) / 8.0;
	case TYPE_EXHAUST: return (m_lifetime - m_age) / std::max(m_lifetime, 0.001f);
	case TYPE_NONE: return 0.0f;
	}
	return 0.0f;
}

SfxManager::SfxManager()
{
	for (size_t t = 0; t < TYPE_NONE; t++) {
		m_instances[t].clear();
	}
}

void SfxManager::ToJson(Json &jsonObj, const FrameId fId, const Space *space)
{
	Frame *f = Frame::GetFrame(fId);
	Json sfxArray = Json::array(); // Create JSON array to contain sfx data.

	if (f->m_sfx) {
		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			for (size_t i = 0; i < f->m_sfx->GetNumberInstances(SFX_TYPE(t)); i++) {
				Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));
				if (inst.m_type != TYPE_NONE) {
					Json sfxArrayEl({}); // Create JSON object to contain sfx element.
					inst.SaveToJson(sfxArrayEl, space);
					sfxArray.push_back(sfxArrayEl); // Append sfx object to array.
				}
			}
		}
	}

	jsonObj["sfx_array"] = sfxArray; // Add sfx array to supplied object.
}

void SfxManager::FromJson(const Json &jsonObj, FrameId fId)
{
	Json sfxArray = jsonObj["sfx_array"].get<Json::array_t>();

	Frame *f = Frame::GetFrame(fId);

	if (sfxArray.size()) f->m_sfx.reset(new SfxManager);
	for (unsigned int i = 0; i < sfxArray.size(); ++i) {
		Sfx inst(sfxArray[i]);
		f->m_sfx->AddInstance(inst);
	}
}

SfxManager *SfxManager::AllocSfxInFrame(FrameId fId)
{
	Frame *f = Frame::GetFrame(fId);

	if (!f->m_sfx) {
		f->m_sfx.reset(new SfxManager);
	}

	return f->m_sfx.get();
}

void SfxManager::Add(const Body *b, SFX_TYPE t)
{
	assert(t != TYPE_NONE);
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;
	vector3d vel(b->GetVelocity() + 200.0 * vector3d(Pi::rng.Double() - 0.5, Pi::rng.Double() - 0.5, Pi::rng.Double() - 0.5));
	Sfx sfx(b->GetPosition(), vel, 200, t);
	sfxman->AddInstance(sfx);
}

void SfxManager::AddExplosion(Body *b)
{
	if (!b) return;

	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	float speed = 200.0f;
	if (b->IsType(ObjectType::SHIP)) {
		ModelBody *mb = static_cast<ModelBody *>(b);
		speed = mb->GetAabb().radius * 8.0;
	}
	Sfx sfx(b->GetPosition(), b->GetVelocity(), speed, TYPE_EXPLOSION);
	sfxman->AddInstance(sfx);
}

void SfxManager::AddThrustSmoke(const Body *b, const float speed, const vector3d &adjustpos)
{
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	Sfx sfx(b->GetPosition() + adjustpos, vector3d::Zero, speed, TYPE_SMOKE);
	sfxman->AddInstance(sfx);
}

void SfxManager::AddExhaust(const Body *b, const Uint16 exhaustJetIndex, const bool exhaustSuppressStreakElongation, const vector3d &startPos, const vector3d &backboneVel, const vector3f &plumeOffset, const vector3f &plumeOffsetVel, const float intensity, const float dragScale, const float opacityScale, const vector3f &windVel, const double groundRadius, const Color &dustTint)
{
	PROFILE_SCOPED()

	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	Sfx sfx(startPos, backboneVel, intensity, TYPE_EXHAUST);
	sfx.m_plumeOffset = plumeOffset;
	sfx.m_plumeOffsetVel = plumeOffsetVel;
	sfx.m_dragScale = dragScale;
	sfx.m_opacityScale = opacityScale;
	sfx.m_windVel = windVel;
	sfx.m_posAtStepStart = startPos;
	sfx.m_exhaustJetIndex = exhaustJetIndex;
	sfx.m_exhaustEmitter = b;
	sfx.m_exhaustSavedEmitterBodyIdx = Sfx::INVALID_EXHAUST_SAVED_BODY_IDX;
	sfx.m_exhaustSuppressStreakElongation = exhaustSuppressStreakElongation;

	// Give each exhaust particle a random lifetime, except for the "marker" particles which only exist to pinpoint
	// the start of an exhaust pulse. Those stay alive as long as possible because if they disappear early then the
	// next particle may try to streak back to an earlier one instead of stopping where it should.
	sfx.m_lifetime = SfxParams::EXHAUST_LIFETIME + 0.5f;
	if (!exhaustSuppressStreakElongation)
		sfx.m_lifetime = SfxParams::EXHAUST_LIFETIME * float(std::pow(Pi::rng.Double(0.5, 1.0), 2.0));

	sfx.m_exhaustGroundRadius = groundRadius;
	sfx.m_exhaustDustKick = false;
	sfx.m_exhaustDustTint = dustTint;
	sfxman->AddInstance(sfx);
}

void SfxManager::TimeStepAll(const float timeStep, FrameId fId)
{
	PROFILE_SCOPED()

	Frame *f = Frame::GetFrame(fId);

	if (f->m_sfx) {
		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			for (size_t i = 0; i < f->m_sfx->GetNumberInstances(SFX_TYPE(t)); i++) {
				Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));
				inst.TimeStepUpdate(timeStep);
			}
		}
		f->m_sfx->Cleanup();
	}

	for (FrameId kid : f->GetChildren()) {
		TimeStepAll(timeStep, kid);
	}
}

void SfxManager::Cleanup()
{
	for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
		const size_t numInstances = GetNumberInstances(SFX_TYPE(t));
		if (!numInstances)
			continue;

		for (Sint64 i = Sint64(numInstances - 1); i >= 0; i--) {
			Sfx &inst(GetInstanceByIndex(SFX_TYPE(t), i));
			if (inst.m_type == TYPE_NONE) {
				m_instances[t].erase(m_instances[t].begin() + i);
			}
		}
	}
}

void SfxManager::RenderAll(Renderer *renderer, FrameId fId, FrameId camFrameId, const Camera *camera, float exhaustIllumMul)
{
	PROFILE_SCOPED()

	Frame *f = Frame::GetFrame(fId);
	int screenHeight = renderer->GetWindowHeight();

	if (camera && Pi::game && Pi::player && !Pi::player->IsDead() && fId == Pi::game->GetSpace()->GetRootFrame()) {
		double amb = 0.0, direct = 1.0;
		camera->CalcLighting(Pi::player, amb, direct);
		exhaustIllumMul = Clamp(float(amb + direct), 0.08f, 1.0f);
	}
	if (f->m_sfx) {
		matrix4x4d ftran;
		// Use interpolated frame transform so SFX aligns with interpolated body rendering.
		const matrix3x3d forient = f->GetInterpOrientRelTo(camFrameId);
		const vector3d fpos = f->GetInterpPositionRelTo(camFrameId);
		ftran = forient;
		ftran.SetTranslate(fpos);

		for (size_t t = TYPE_EXPLOSION; t < TYPE_NONE; t++) {
			const size_t numInstances = f->m_sfx->GetNumberInstances(SFX_TYPE(t));
			if (!numInstances)
				continue;

			Graphics::Material *material = nullptr;
			switch (t) {
			case TYPE_NONE: assert(false); break;
			case TYPE_EXPLOSION:
				material = explosionParticle.get();
				break;
			case TYPE_DAMAGE:
				material = damageParticle.get();
				break;
			case TYPE_SMOKE:
				material = smokeParticle.get();
				break;
			case TYPE_EXHAUST:
				material = exhaustParticle.get();
				break;
			}

			const double timeStep = Pi::game ? double(Pi::game->GetTimeStep()) : 0.0;
			const double alpha = Pi::game ? Pi::GetGameTickAlpha() : 1.0;

			if (t == TYPE_EXHAUST) {
				const matrix3x3f frameRot(forient);

				// We need to draw the particles in order, so that we can have streaks in the exhaust jet - each particle
				// elongates back to the previous particle's position. So the particles need to be in "jet stream order".
				// However, they are already in order, because new particles are always added at the end, and existing
				// particles may disappear but never swap places. Except - if there is more than one jet (quite likely
				// given many ships have multiple thrusters grouped together) then the particles are all mixed up per jet.
				// So we now separate them into jet stream buckets by walking through the list.

				for (auto &bucket : s_exhaustByJet)
					bucket.second.clear();

				for (size_t i = 0; i < numInstances; ++i) {
					Sfx &inst(f->m_sfx->GetInstanceByIndex(TYPE_EXHAUST, i));
					assert(inst.m_type == t);
					const ExhaustJetStreamKey key{
						ExhaustStreamEmitterSortKey(inst),
						inst.m_exhaustJetIndex,
					};
					s_exhaustByJet[key].push_back(&inst);
				}

				for (auto it = s_exhaustByJet.begin(); it != s_exhaustByJet.end(); ) {
					if (it->second.empty())
						it = s_exhaustByJet.erase(it);
					else
						++it;
				}

				std::vector<ExhaustInstanceData> exhaustInstances;
				exhaustInstances.reserve(numInstances);

				for (const auto &jetBucket : s_exhaustByJet) {
					const Sfx *prevStreakParticle = nullptr;
					vector3f prevInterpBackboneCam = vector3f(0.f);

					for (Sfx *instPtr : jetBucket.second) {
						Sfx &inst(*instPtr);

						// Backbone stores explicit start/end for this physics step.
						vector3d interpBackbone = inst.m_posAtStepStart * (1.0 - alpha) + inst.m_pos * alpha;

						// Plume offset is a small local perturbation, composite in camera space after backbone transform.
						const vector3f interpOffset = inst.m_plumeOffset - inst.m_plumeOffsetVel * float((1.0 - alpha) * timeStep);
						const vector3f backboneCam(ftran * interpBackbone);
						const vector3f pos = backboneCam + frameRot * interpOffset;

						const float ageNorm = Clamp(inst.m_age / std::max(inst.m_lifetime, 0.001f), 0.0f, 1.0f);
						const vector3f interpBackboneCam = backboneCam;

						// The first particle in a jet does not get drawn - it is there to allow the next particle to stretch back
						// towards it. This also applies to the first particle in a thruster pulse - if the user repeatedly presses
						// and lets go of a thruster key, then we don't want each burst's initial particle to streak towards the end
						// of the previous burst, so we deliberately suppress those.
						// However - none of this applies to dust particles, which have no streaking.
						const bool prevWasDust = prevStreakParticle && prevStreakParticle->m_exhaustDustKick;
						const bool elongateTowardPrev = prevStreakParticle && !inst.m_exhaustSuppressStreakElongation;

						// The elongation effectively draws particles stretched from their current location back towards
						// the previous particle in the stream. But since the particles are soft ellipses we need to
						// lengthen the jet vector to make them overlap and get a nice unbroken streaky stream.
						const vector3f jetVectorCam = (interpBackboneCam - prevInterpBackboneCam) * 4.0;

						prevInterpBackboneCam = interpBackboneCam;
						prevStreakParticle = &inst;

						if (!elongateTowardPrev && !inst.m_exhaustDustKick) continue;
						if (prevWasDust && !inst.m_exhaustDustKick) continue;

						// Don't start drawing particles until they have started spreading. This ensures we don't have
						// any thin lines coming from the thruster nozzle, preferring a later "emerging contrail" effect.
						if (inst.m_age < SfxParams::EXHAUST_TIME_BEFORE_SPREAD) continue;

						float size = std::max(0.01f, float(inst.m_plumeOffset.Length()));
						if (inst.m_exhaustDustKick)
							size = SfxParams::EXHAUST_DUST_SIZE * ageNorm * std::max(inst.m_speed, 0.2f);

						const float ageFade = powf(1.0f - ageNorm, 2.8f);

						Color particleColor;
						if (inst.m_exhaustDustKick) {
							particleColor = inst.m_exhaustDustTint;
							const Uint8 opacityA = Uint8((float)particleColor.a * ageFade);
							particleColor.a = opacityA;
						} else {
							const float opacityTimesAge = Clamp(inst.m_opacityScale * ageFade, 0.f, 1.f);
							const Uint8 opacityA = Uint8(opacityTimesAge * 255.f);
							particleColor = Color(255, 255, 255, opacityA);
						}

						particleColor.r = Uint8(Clamp(int(float(particleColor.r) * exhaustIllumMul), 0, 255));
						particleColor.g = Uint8(Clamp(int(float(particleColor.g) * exhaustIllumMul), 0, 255));
						particleColor.b = Uint8(Clamp(int(float(particleColor.b) * exhaustIllumMul), 0, 255));

						const float stretchScale = (prevWasDust || inst.m_exhaustDustKick) ? 0.f : 1.f;

						exhaustInstances.push_back({
							pos,
							size,
							jetVectorCam,
							interpBackboneCam.z,
							stretchScale,
							particleColor,
						});
					}
				}

				if (!exhaustInstances.empty()) {
					const uint32_t numDrawn = uint32_t(exhaustInstances.size());
					Graphics::BufferBinding<Graphics::VertexBuffer> instanceBinding =
						renderer->CreateTempVertexBuffer(numDrawn, sizeof(ExhaustInstanceData));

					ExhaustInstanceData *instanceData =
						instanceBinding.buffer->MapRange<ExhaustInstanceData>(instanceBinding, Graphics::BUFFER_MAP_WRITE);
					if (instanceData) {
						std::memcpy(instanceData, exhaustInstances.data(), numDrawn * sizeof(ExhaustInstanceData));
						instanceBinding.buffer->UnmapRange(false);
					}

					renderer->SetTransform(matrix4x4f::Identity);
					renderer->Draw({ instanceBinding }, {}, material, 4, numDrawn);
				}

			} else {
				const double interpDt = alpha * timeStep;
				// NB - we're (ab)using the normal type to hold (uv coordinate offset value + point size)
				Graphics::VertexArray pointArray(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL, numInstances);

				for (size_t i = 0; i < numInstances; i++) {
					Sfx &inst(f->m_sfx->GetInstanceByIndex(SFX_TYPE(t), i));

					assert(inst.m_type == t);

					vector3d interpPos = inst.m_pos + inst.m_vel * interpDt;

					const vector3f pos(ftran * interpPos);

					// pack UV offset and particle size in normal attribute
					const vector2f offset = CalculateOffset(SFX_TYPE(t), inst);
					const float size = GetParticleSize(screenHeight, SFX_TYPE(t), pos, inst);
					pointArray.Add(pos, vector3f(offset, Clamp(size, 0.1f, FLT_MAX)));
				}

				renderer->SetTransform(matrix4x4f::Identity);
				renderer->DrawBuffer(&pointArray, material);
			}
		}
	}

	for (FrameId kid : f->GetChildren()) {
		RenderAll(renderer, kid, camFrameId, camera, exhaustIllumMul);
	}
}

vector2f SfxManager::CalculateOffset(const enum SFX_TYPE type, const Sfx &inst)
{
	if (m_materialData[type].effect == Graphics::EFFECT_BILLBOARD_ATLAS) {
		const Uint32 max_texture = (m_materialData[type].num_textures);
		const int spriteframe = max_texture - (inst.AgeBlend() * max_texture); // count DOWN from the maximum texture to zero
		const Sint32 numImgsWide = m_materialData[type].num_imgs_wide;
		const int u = (spriteframe % numImgsWide); // % is the "modulo operator", the remainder of i / width;
		const int v = (spriteframe / numImgsWide); // where "/" is an integer division
		return vector2f(
			float(u) / float(numImgsWide),
			float(v) / float(numImgsWide));
	}
	return vector2f(0.0f);
}

bool SfxManager::SplitMaterialData(const std::string &spec, MaterialData &output)
{
	static const std::string delim(",");

	enum dataEntries {
		eEFFECT = 0,
		eNUM_IMGS_WIDE,
		eNUM_TEXTURES,
		eCOORD_DOWNSCALE
	};

	size_t i = 0, start = 0, end = 0;
	while (end != std::string::npos) {
		// get to the first non-delim char
		start = spec.find_first_not_of(delim, end);

		// read the end, no more to do
		if (start == std::string::npos)
			break;

		// find the end - next delim or end of string
		end = spec.find_first_of(delim, start);

		// extract the fragment and remember it
		switch (i) {
		case eEFFECT:
			output.effect = (spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start) == "billboard") ? Graphics::EFFECT_BILLBOARD : Graphics::EFFECT_BILLBOARD_ATLAS;
			break;
		case eNUM_IMGS_WIDE:
			output.num_imgs_wide = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
			break;
		case eNUM_TEXTURES:
			output.num_textures = atoi(spec.substr(start, (end == std::string::npos) ? std::string::npos : end - start).c_str());
			break;
		default:
		case eCOORD_DOWNSCALE:
			assert(false);
			break;
		}
		i++;
	}

	output.coord_downscale = 1.0f / float(output.num_imgs_wide);
	return i == eCOORD_DOWNSCALE;
}

void SfxManager::Init(Graphics::Renderer *r)
{
	PROFILE_SCOPED()
	IniConfig cfg;
	// set defaults in case they're missing from the file
	cfg.SetString("damageFile", "textures/smoke.png");
	cfg.SetString("smokeFile", "textures/smoke.png");
	cfg.SetString("explosionFile", "textures/explosions/explosions.png");

	cfg.SetString("damagePacking", "billboard,1,1");
	cfg.SetString("smokePacking", "billboard,1,1");
	cfg.SetString("explosionPacking", "atlas,6,32");
	// load
	cfg.Read(FileSystem::gameDataFiles, "textures/Sfx.ini");

	// shared render states
	Graphics::RenderStateDesc alphaState;
	alphaState.blendMode = Graphics::BLEND_ALPHA;
	alphaState.depthWrite = false;
	alphaState.primitiveType = Graphics::POINTS;

	Graphics::RenderStateDesc additiveAlphaState;
	additiveAlphaState.blendMode = Graphics::BLEND_ALPHA_ONE;
	additiveAlphaState.depthWrite = false;
	additiveAlphaState.primitiveType = Graphics::POINTS;

	Graphics::RenderStateDesc exhaustAlphaState = alphaState;
	exhaustAlphaState.primitiveType = Graphics::TRIANGLE_STRIP;
	exhaustAlphaState.cullMode = Graphics::CULL_NONE;

	Graphics::VertexFormatDesc vfmt = Graphics::VertexFormatDesc::FromAttribSet(Graphics::ATTRIB_POSITION | Graphics::ATTRIB_NORMAL);

	// materials
	Graphics::MaterialDescriptor desc;
	desc.textures = 1;

	// ECM effect is different, not managed by Sfx at all, should it be factored out?
	desc.effect = Graphics::EFFECT_BILLBOARD;
	ecmParticle.reset(r->CreateMaterial("billboards", desc, additiveAlphaState, vfmt));
	ecmParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard("textures/ecm.png").GetOrCreateTexture(r, "billboard"));

	// load material definition data
	SplitMaterialData(cfg.String("explosionPacking"), m_materialData[TYPE_EXPLOSION]);
	SplitMaterialData(cfg.String("damagePacking"), m_materialData[TYPE_DAMAGE]);
	SplitMaterialData(cfg.String("smokePacking"), m_materialData[TYPE_SMOKE]);
	m_materialData[TYPE_EXHAUST].effect = Graphics::EFFECT_BILLBOARD;

	desc.effect = m_materialData[TYPE_DAMAGE].effect;
	damageParticle.reset(r->CreateMaterial("billboards", desc, additiveAlphaState, vfmt));
	damageParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("damageFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		damageParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_DAMAGE].coord_downscale);

	desc.effect = m_materialData[TYPE_SMOKE].effect;
	smokeParticle.reset(r->CreateMaterial("billboards", desc, alphaState, vfmt));
	smokeParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("smokeFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		smokeParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_SMOKE].coord_downscale);

	Graphics::MaterialDescriptor exhaustDesc;
	exhaustDesc.textures = 0;
	exhaustDesc.effect = Graphics::EFFECT_BILLBOARD;
	const Graphics::VertexFormatDesc exhaustVfmt = BuildExhaustInstancedVertexFormat();
	exhaustParticle.reset(r->CreateMaterial("exhaust", exhaustDesc, exhaustAlphaState, exhaustVfmt));
	exhaustParticle->diffuse = Color(255, 255, 255, 255);

	desc.effect = m_materialData[TYPE_EXPLOSION].effect;
	explosionParticle.reset(r->CreateMaterial("billboards", desc, alphaState, vfmt));
	explosionParticle->SetTexture("texture0"_hash,
		Graphics::TextureBuilder::Billboard(cfg.String("explosionFile")).GetOrCreateTexture(r, "billboard"));
	if (desc.effect == Graphics::EFFECT_BILLBOARD_ATLAS)
		explosionParticle->SetPushConstant("coordDownScale"_hash,
			m_materialData[TYPE_EXPLOSION].coord_downscale);
}

void SfxManager::Uninit()
{
	damageParticle.reset();
	ecmParticle.reset();
	smokeParticle.reset();
	exhaustParticle.reset();
	explosionParticle.reset();
}
