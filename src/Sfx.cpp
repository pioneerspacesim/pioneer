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

#include "profiler/Profiler.h"

#include <algorithm>
#include <cmath>
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

	inline bool SameExhaustJetStream(const Sfx &a, const Sfx &b)
	{
		if (a.m_exhaustJetIndex != b.m_exhaustJetIndex)
			return false;
		if (a.m_exhaustEmitter || b.m_exhaustEmitter)
			return a.m_exhaustEmitter == b.m_exhaustEmitter;
		return a.m_exhaustSavedEmitterBodyIdx == b.m_exhaustSavedEmitterBodyIdx;
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

		const vector3d particlePos = s.m_backbonePos + s.m_plumeOffset;
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

		s.m_backbonePos = surface;
		s.m_plumeOffset = vector3d(0.);
		s.m_backboneVel = vector3d(0.);
		s.m_plumeOffsetVel =
			rad * double(SfxParams::EXHAUST_DUST_RADIAL_KICK_SPEED * s.m_speed * float(radJ)) +
			tang * double(SfxParams::EXHAUST_DUST_TANGENT_KICK_SPEED * s.m_speed * float(tanJ));

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
	m_backbonePos(pos),
	m_backboneVel(vel),
	m_backboneAtStepStart(pos),
	m_plumeOffset(vector3d::Zero),
	m_plumeOffsetVel(vector3d::Zero),
	m_age(0.0f),
	m_speed(speed),
	m_seed(float(Pi::rng.Double())),
	m_lifetime(SfxParams::EXHAUST_LIFETIME),
	m_dragScale(1.0f),
	m_opacityScale(1.0f),
	m_windVel(vector3d::Zero),
	m_type(type),
	m_exhaustEmitter(nullptr),
	m_exhaustSavedEmitterBodyIdx(Sfx::INVALID_EXHAUST_SAVED_BODY_IDX),
	m_exhaustJetIndex(0),
	m_exhaustBirthSeq(0),
	m_exhaustSuppressStreakElongation(false),
	m_exhaustGroundRadius(0.0),
	m_exhaustDustKick(false),
	m_exhaustDustTint(0, 0, 0, 0)
{
	if (type == TYPE_EXHAUST)
		m_lifetime = SfxParams::EXHAUST_LIFETIME * float(std::pow(Pi::rng.Double(0.5, 1.0), 2.0));
}

Sfx::Sfx(const Json &jsonObj)
{
	try {
		Json sfxObj = jsonObj["sfx"];

		m_pos = sfxObj["pos"];
		m_vel = sfxObj["vel"];
		m_backbonePos = sfxObj.value("backbonePos", m_pos);
		m_backboneVel = sfxObj.value("backboneVel", m_vel);
		m_backboneAtStepStart = sfxObj.value("backboneAtStepStart", m_backbonePos);
		m_plumeOffset = sfxObj.value("plumeOffset", vector3d::Zero);
		m_plumeOffsetVel = sfxObj.value("plumeOffsetVel", vector3d::Zero);
		m_age = sfxObj["age"];
		m_seed = sfxObj.value("seed", float(Pi::rng.Double()));
		m_lifetime = sfxObj.value("lifetime", SfxParams::EXHAUST_LIFETIME);
		m_dragScale = sfxObj.value("dragScale", 1.0f);
		m_opacityScale = sfxObj.value("opacityScale", 1.0f);
		m_windVel = sfxObj.value("windVel", vector3d::Zero);
		m_type = sfxObj["type"];
		m_exhaustEmitter = nullptr;
		m_exhaustSavedEmitterBodyIdx = sfxObj.value("exhaustEmitterBodyIdx", Sfx::INVALID_EXHAUST_SAVED_BODY_IDX);
		m_exhaustJetIndex = Uint16(sfxObj.value("exhaustJetIndex", 0));
		m_exhaustBirthSeq = sfxObj.value("exhaustBirthSeq", Uint32(0));
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
		sfxObj["backbonePos"] = m_backbonePos;
		sfxObj["backboneVel"] = m_backboneVel;
		sfxObj["backboneAtStepStart"] = m_backboneAtStepStart;
		sfxObj["plumeOffset"] = m_plumeOffset;
		sfxObj["plumeOffsetVel"] = m_plumeOffsetVel;
		sfxObj["lifetime"] = m_lifetime;
		sfxObj["dragScale"] = m_dragScale;
		sfxObj["opacityScale"] = m_opacityScale;
		sfxObj["windVel"] = m_windVel;
		Uint32 bodyIdxForSave = m_exhaustSavedEmitterBodyIdx;
		if (m_exhaustEmitter && space && space->IsBodyIndexValid())
			bodyIdxForSave = space->GetIndexForBody(m_exhaustEmitter);
		sfxObj["exhaustEmitterBodyIdx"] = bodyIdxForSave;
		sfxObj["exhaustJetIndex"] = m_exhaustJetIndex;
		sfxObj["exhaustBirthSeq"] = m_exhaustBirthSeq;
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
			m_backboneAtStepStart = m_backbonePos;

			const double drag = Clamp(0.9 * double(m_dragScale) * double(timeStep), 0.0, 0.85);
			// Backbone follows the coherent central jet flow.
			m_backboneVel += (m_windVel - m_backboneVel) * drag;

			m_backbonePos += m_backboneVel * double(timeStep);
			if (m_age > SfxParams::EXHAUST_TIME_BEFORE_SPREAD)
				m_plumeOffset += m_plumeOffsetVel * double(timeStep);
			ApplyExhaustGroundThreshold(*this);

			// Keep legacy fields coherent for non-exhaust code paths.
			m_pos = m_backbonePos + m_plumeOffset;
			m_vel = m_backboneVel + m_plumeOffsetVel;
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

SfxManager::SfxManager() :
	m_nextExhaustBirthSeq(1)
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

void SfxManager::AddExhaust(const Body *b, const Uint16 exhaustJetIndex, const bool exhaustSuppressStreakElongation, const vector3d &startPos, const vector3d &backboneVel, const vector3d &plumeOffset, const vector3d &plumeOffsetVel, const float intensity, const float dragScale, const float opacityScale, const vector3d &windVel, const double groundRadius, const Color &dustTint)
{
	SfxManager *sfxman = AllocSfxInFrame(b->GetFrame());
	if (!sfxman) return;

	const vector3d pos = startPos + plumeOffset;
	Sfx sfx(pos, backboneVel, intensity, TYPE_EXHAUST);
	sfx.m_backbonePos = startPos;
	sfx.m_backboneVel = backboneVel;
	sfx.m_plumeOffset = plumeOffset;
	sfx.m_plumeOffsetVel = plumeOffsetVel;
	sfx.m_vel = sfx.m_backboneVel + sfx.m_plumeOffsetVel;
	sfx.m_dragScale = dragScale;
	sfx.m_opacityScale = opacityScale;
	sfx.m_windVel = windVel;
	sfx.m_backboneAtStepStart = sfx.m_backbonePos;
	sfx.m_exhaustJetIndex = exhaustJetIndex;
	sfx.m_exhaustEmitter = b;
	sfx.m_exhaustSavedEmitterBodyIdx = Sfx::INVALID_EXHAUST_SAVED_BODY_IDX;
	sfx.m_exhaustSuppressStreakElongation = exhaustSuppressStreakElongation;
	sfx.m_exhaustBirthSeq = sfxman->m_nextExhaustBirthSeq++;
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
				Graphics::VertexArray exhaustArray(
					Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0,
					numInstances * 6);
				std::vector<size_t> exhaustOrder(numInstances);
				for (size_t i = 0; i < numInstances; ++i)
					exhaustOrder[i] = i;
				std::sort(exhaustOrder.begin(), exhaustOrder.end(), [&](size_t ia, size_t ib) {
					const Sfx &a = f->m_sfx->GetInstanceByIndex(TYPE_EXHAUST, ia);
					const Sfx &b = f->m_sfx->GetInstanceByIndex(TYPE_EXHAUST, ib);
					const uint64_t keyA = ExhaustStreamEmitterSortKey(a);
					const uint64_t keyB = ExhaustStreamEmitterSortKey(b);
					if (keyA != keyB)
						return keyA < keyB;
					if (a.m_exhaustJetIndex != b.m_exhaustJetIndex)
						return a.m_exhaustJetIndex < b.m_exhaustJetIndex;
					if (a.m_exhaustBirthSeq != b.m_exhaustBirthSeq)
						return a.m_exhaustBirthSeq < b.m_exhaustBirthSeq;
					return ia < ib;
				});

				const Sfx *prevStreakParticle = nullptr;
				vector3d prevInterpBackboneCam = vector3d::Zero;

				for (const size_t i : exhaustOrder) {
					Sfx &inst(f->m_sfx->GetInstanceByIndex(TYPE_EXHAUST, i));

					assert(inst.m_type == t);

					// Backbone stores explicit start/end for this physics step.
					vector3d interpBackbone = inst.m_backboneAtStepStart * (1.0 - alpha) + inst.m_backbonePos * alpha;

					// Plume offset is a small local perturbation; derive its start-of-step estimate
					// from end-of-step offset and current plume offset velocity.
					const vector3d interpOffset = inst.m_plumeOffset - inst.m_plumeOffsetVel * ((1.0 - alpha) * timeStep);
					const vector3d interpPos = interpBackbone + interpOffset;
					const vector3f pos(ftran * interpPos);

					const float ageNorm = Clamp(inst.m_age / std::max(inst.m_lifetime, 0.001f), 0.0f, 1.0f);
					const vector3d interpBackboneCam = ftran * interpBackbone;

					const bool prevWasDust = prevStreakParticle && prevStreakParticle->m_exhaustDustKick;
					const bool sameJetStream = prevStreakParticle && SameExhaustJetStream(*prevStreakParticle, inst);
					const bool elongateTowardPrev = sameJetStream && !inst.m_exhaustSuppressStreakElongation;

					// The elongation effectively draws particles stretched from their current location back towards
					// the previous particle in the stream. But since the particles are soft ellipses we need to 
					// lengthen the jet vector first, to get a nice unbroken streaky stream.
					const vector3f jetVectorCam = vector3f((interpBackboneCam - prevInterpBackboneCam) * 4.0);

					prevInterpBackboneCam = interpBackboneCam;
					prevStreakParticle = &inst;

					if (!elongateTowardPrev && !inst.m_exhaustDustKick) continue;
					if (!sameJetStream && !inst.m_exhaustDustKick) continue;
					if (prevWasDust && !inst.m_exhaustDustKick) continue;

					// Camera-space view direction from particle to camera origin.
					const vector3f viewDir = (-pos).NormalizedSafe();

					// Attenuate elongation when difference in Z values is relatively large
					float attenuation = 1.0;
					const float prevZ = std::abs(interpBackboneCam.z - jetVectorCam.z);
					const float curZ = std::abs(interpBackboneCam.z);
					if ((prevZ > 0.1f) && (curZ > 0.1f))
						attenuation = std::min(1.0f, curZ / prevZ);

					// Billboard plane basis: V aligns with jet axis projection onto the view plane.
					// This approximates an ellipsoid projection:
					// - looking down axis => round
					// - side-on => elongated.
					const vector3f vProj = jetVectorCam - viewDir * jetVectorCam.Dot(viewDir);
					const vector3f axisV = vProj.NormalizedSafe();
					const vector3f axisU = viewDir.Cross(axisV).NormalizedSafe();

					const float speedMag = jetVectorCam.Dot(axisV);
					const float apparentStretch = (prevWasDust || inst.m_exhaustDustKick ? 0.0 : speedMag * attenuation);

					float size = std::max(0.01f, float(inst.m_plumeOffset.Length()));
					if (inst.m_exhaustDustKick)
						size = SfxParams::EXHAUST_DUST_SIZE * ageNorm * std::max(inst.m_speed, 0.2f);

					// Two UV triangles, the V-axis is aligned to the jet backbone axis.
					const vector3f u = axisU * size;
					const vector3f v = axisV * size;
					const vector3f v_add = axisV * apparentStretch;

					const vector3f p0 = pos - u - v - v_add;
					const vector3f p1 = pos + u - v - v_add;
					const vector3f p2 = pos + u + v;
					const vector3f p3 = pos - u + v;

					const float ageFade = powf(1.0f - ageNorm, 2.8f);

					Color particleColor;
					if (inst.m_exhaustDustKick) {
						// This is a dust cloud particle
						particleColor = inst.m_exhaustDustTint;
						const Uint8 opacityA = Uint8((float)particleColor.a * ageFade);
						particleColor.a = opacityA;
					} else {
						// This is an exhaust plume particle
						const float opacityTimesAge = Clamp(inst.m_opacityScale * ageFade, 0.f, 1.f);
						const Uint8 opacityA = Uint8(opacityTimesAge * 255.f);
						particleColor = Color(255, 255, 255, opacityA);
					}

					particleColor.r = Uint8(Clamp(int(float(particleColor.r) * exhaustIllumMul), 0, 255));
					particleColor.g = Uint8(Clamp(int(float(particleColor.g) * exhaustIllumMul), 0, 255));
					particleColor.b = Uint8(Clamp(int(float(particleColor.b) * exhaustIllumMul), 0, 255));

					exhaustArray.Add(p0, particleColor, vector2f(0.f, 0.f));
					exhaustArray.Add(p1, particleColor, vector2f(1.f, 0.f));
					exhaustArray.Add(p2, particleColor, vector2f(1.f, 1.f));
					exhaustArray.Add(p2, particleColor, vector2f(1.f, 1.f));
					exhaustArray.Add(p3, particleColor, vector2f(0.f, 1.f));
					exhaustArray.Add(p0, particleColor, vector2f(0.f, 0.f));
				}

				renderer->SetTransform(matrix4x4f::Identity);
				renderer->DrawBuffer(&exhaustArray, material);

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
	exhaustAlphaState.primitiveType = Graphics::TRIANGLES;
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
	const auto exhaustVfmt = Graphics::VertexFormatDesc::FromAttribSet(
		Graphics::ATTRIB_POSITION | Graphics::ATTRIB_DIFFUSE | Graphics::ATTRIB_UV0);
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
