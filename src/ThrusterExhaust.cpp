// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "ThrusterExhaust.h"

#include "Body.h"
#include "Pi.h"
#include "Sfx.h"
#include "scenegraph/MatrixTransform.h"
#include "scenegraph/Model.h"
#include "scenegraph/Thruster.h"
#include "ship/Propulsion.h"

#include <algorithm>
#include <cmath>

void ThrusterExhaustSpawner::RefreshMounts(SceneGraph::Model *model)
{
	m_mounts.clear();
	m_thrusters.clear();
	if (!model) return;

	std::vector<std::pair<SceneGraph::MatrixTransform *, SceneGraph::Thruster *>> tmp;
	model->GatherThrusterMounts(tmp);

	std::vector<float> thrusterScaleAvg;
	thrusterScaleAvg.reserve(tmp.size());
	float maxThrusterScale = 0.0f;
	for (const auto &pr : tmp) {
		const matrix4x4f M = pr.first->CalcGlobalTransform();
		const float sx = vector3f(M[0], M[1], M[2]).Length();
		const float sy = vector3f(M[4], M[5], M[6]).Length();
		const float sz = vector3f(M[8], M[9], M[10]).Length();
		const float scaleAvg = (sx + sy + sz) / 3.0f;
		thrusterScaleAvg.push_back(scaleAvg);
		maxThrusterScale = std::max(maxThrusterScale, scaleAvg);
	}
	m_mounts.reserve(tmp.size());
	m_thrusters.reserve(tmp.size());
	for (size_t i = 0; i < tmp.size(); ++i) {
		const auto &pr = tmp[i];
		m_mounts.push_back(pr.first);
		m_thrusters.push_back(pr.second);
		const float scaleAvg = thrusterScaleAvg[i];
		const float scaleProportional = (maxThrusterScale > 1e-6f) ? (scaleAvg / maxThrusterScale) : 1.0f;
		pr.second->SetVisualSizeInfo(scaleAvg, scaleProportional);
	}
	m_channels.assign(tmp.size(), ExhaustThrusterChannel{});
}

void ThrusterExhaustSpawner::ClearChannelState()
{
	for (auto &ch : m_channels) {
		ch.hasLastNozzle = false;
		ch.wasThrusterFiring = false;
	}
}

void ThrusterExhaustSpawner::Spawn(const Body *body, const Propulsion *propulsion, const float timeStep, const ExhaustEnvironment &env, const float particlesPerSecTotal)
{
	if (!body || !propulsion || m_mounts.empty()) return;

	if (m_mounts.size() != m_thrusters.size())
		return;
	if (m_channels.size() != m_mounts.size())
		m_channels.assign(m_mounts.size(), ExhaustThrusterChannel{});

	const vector3f linT = vector3f(propulsion->GetLinThrusterState());
	const vector3f angT = -vector3f(propulsion->GetAngThrusterState());

	// How hard linear thrusters are firing vs a ship-wide reference (larger of up / forward max thrust).
	const double maxRefThrust = std::max(propulsion->GetThrust(THRUSTER_UP), propulsion->GetThrust(THRUSTER_FORWARD));
	const vector3d actualLinThrust = propulsion->GetActualLinThrust();
	const vector3d actualAngThrust = propulsion->GetActualAngThrust();
	const float linThrustEffortScalar = actualLinThrust.Length() / maxRefThrust;
	const float angThrustEffortScalar = actualAngThrust.Length() * SfxParams::EXHAUST_ANGULAR_FACTOR / propulsion->GetAngThrustCap();

	const float atmosDragScale = float(Clamp(env.density / 1.225, 0.0, 2.0));
	const float noiseStrength = env.density * SfxParams::EXHAUST_NOISE_STRENGTH / 1.225;
	const matrix3x3d &bodyOrient = body->GetOrient();

	// Some ships have lots of small thrusters next to each other. Others have one big one.
	// We use the same number of particles per ship, since small adjacent streams will merge
	// visually anyway, and we this way we don't end up with too many particles on some ships
	// and not enough on others.
	const float particlesPerSecPerThruster = particlesPerSecTotal / m_mounts.size();

	for (size_t ti = 0; ti < m_mounts.size(); ++ti) {
		SceneGraph::MatrixTransform *mt = m_mounts[ti];
		SceneGraph::Thruster *thr = m_thrusters[ti];
		ExhaustThrusterChannel &ch = m_channels[ti];

		const matrix4x4f M = mt->CalcGlobalTransform();
		const vector3d nozzleLocal = vector3d(M.GetTranslate());
		const vector3d exhaustDirModel = vector3d(thr->GetDirection()).NormalizedSafe();
		const vector3d currentNozzleWorld = body->GetPosition() + bodyOrient * nozzleLocal;

		if (!ch.hasLastNozzle) {
			ch.lastNozzleWorld = currentNozzleWorld;
			ch.hasLastNozzle = true;
		}

		const float visualScaled = thr->GetVisualSizeProportional();
		// Get the amount that each thruster is firing this tick.
		// Scale by thruster visual size and overall thrust demand
		// Same combined reaction as model thruster flames
		const float reactionPower = thr->ComputeReactionPower(linT, angT);
		const float linOnly = thr->ComputeReactionPower(linT, vector3f(0.f));
		const float angOnly = thr->ComputeReactionPower(vector3f(0.f), angT);
		const float effortScalar = (angOnly >= linOnly) ? angThrustEffortScalar : linThrustEffortScalar;
		const float unscaledPower = reactionPower * visualScaled * effortScalar;
		// Use a log scale on the final thruster power so that tiny maneouvering thrusters are still visible compared to huge delta-V ones
		const float finalThrusterPower = std::log(1.0f + SfxParams::EXHAUST_LOG_SCALE * unscaledPower) / std::log(1.0f + SfxParams::EXHAUST_LOG_SCALE);

		const bool firing = finalThrusterPower >= SfxParams::EXHAUST_MIN_REACTION_POWER;
		const bool newPulseLeadingEdge = firing && !ch.wasThrusterFiring;

		vector3d exhaustDirWorld = bodyOrient * exhaustDirModel;
		if (exhaustDirWorld.LengthSqr() < 1e-12)
			exhaustDirWorld = bodyOrient.VectorZ();
		exhaustDirWorld = exhaustDirWorld.Normalized();
		const vector3d backboneWorldVel = body->GetVelocity() + exhaustDirWorld * double(SfxParams::EXHAUST_INITIAL_VELOCITY);

		// Where the jet backbone was at the start of this frame (previous nozzle + last frame's exhaust velocity).
		const vector3d lastBackbonePosWorld = ch.lastNozzleWorld + ch.lastBackboneVel * double(timeStep);

		if (firing) {
			// Reduce the alpha for weaker thrusters, so that the exhaust is less pronounced
			const float opacityScale = Clamp(env.opacityAtmosphereFactor * finalThrusterPower, 0.0f, 1.0f);

			// Increase drag for weaker thrusters, so that they slow down more quickly (less volume of exhaust)
			const float dragScale = atmosDragScale * SfxParams::EXHAUST_DRAG_FACTOR / std::max(visualScaled, 0.2f);

			// Keep particle count mostly independent of thrust / density so the stream remains full.
			// If time is accelerated then timeStep could be huge - limit the number of particles in that case
			// If timeStep is tiny then ensure we have at least one particle.
			const float timeStepCapped = std::min(timeStep, SfxParams::EXHAUST_STREAM_TIMESTEP_CAP);
			const float thrusterEmit = particlesPerSecPerThruster * timeStepCapped * finalThrusterPower;
			int count = std::max(1, int(thrusterEmit));

			// If this is the first tick where the thruster is firing, then only add one particle
			// at the current nozzle position as the leading edge. It won't be drawn but will be
			// used as the starting elongation target on the next tick
			if (newPulseLeadingEdge)
				count = 1;

			const vector3f exhaustDirF = vector3f(exhaustDirWorld);
			const vector3f shipY = vector3f(bodyOrient.VectorY());
			const vector3f refAxis = (std::abs(exhaustDirF.Dot(shipY)) < 0.85f) ? shipY : vector3f(bodyOrient.VectorZ());
			vector3f uAxis = refAxis.Cross(exhaustDirF).Normalized();
			if (uAxis.LengthSqr() < 1e-12f)
				uAxis = vector3f(bodyOrient.VectorX());
			const vector3f vAxis = exhaustDirF.Cross(uAxis).Normalized();

			for (int i = 0; i < count; i++) {
				const double segT = double(i + 1) / double(count);
				// Distribute new particles along the line from the projected backbone to the current nozzle.
				const vector3d backbonePosWorld = lastBackbonePosWorld.Lerp(currentNozzleWorld, segT);
				const vector3d backboneVelWorld = ch.lastBackboneVel.Lerp(backboneWorldVel, segT);

				const float jitterRadius = float(Pi::rng.Double(1.0));
				const float jitterTheta = float(Pi::rng.Double(2.0 * M_PI));

				const vector3f jitterDirW = (uAxis * std::cos(jitterTheta) + vAxis * std::sin(jitterTheta)).Normalized();
				const vector3f plumeOffset = jitterDirW * (jitterRadius * SfxParams::EXHAUST_INITIAL_SPREAD);

				const float jitterVel = (env.maxSpread * jitterRadius) / SfxParams::EXHAUST_LIFETIME;
				const float spreadAmp = float(Clamp(1.0 / std::max(env.density, 1e-5), 0.05, 2.0));
				const vector3f plumeOffsetVel = jitterDirW * (jitterVel * spreadAmp);

				const bool suppressStreak = newPulseLeadingEdge && (i == 0);
				SfxManager::AddExhaust(body, Uint16(ti), suppressStreak, backbonePosWorld, backboneVelWorld, plumeOffset, plumeOffsetVel, finalThrusterPower, dragScale, opacityScale, env.windVel, env.groundRadius, env.dustTint, env.baseLifetime, env.maxSpread, noiseStrength);
			}
		}

		ch.wasThrusterFiring = firing;
		ch.lastNozzleWorld = currentNozzleWorld;
		ch.lastBackboneVel = backboneWorldVel;
	}
}
