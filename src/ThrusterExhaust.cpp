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
#include <vector>

namespace {

struct ExhaustMountCandidate {
	SceneGraph::Thruster *thr = nullptr;
	vector3f pos;
	vector3f dir;
	float scale = 0.f;
};

size_t ExhaustClusterRoot(std::vector<size_t> &parent, size_t x)
{
	size_t root = x;
	while (parent[root] != root)
		root = parent[root];
	while (parent[x] != root) {
		const size_t next = parent[x];
		parent[x] = root;
		x = next;
	}
	return root;
}

// Work out where thruster exhaust jets should originate from.
// Tiny thrusters sitting next to a larger same-direction thruster are dropped.
// Packed groups of small same-direction thrusters share a single exhaust jet.
// Similar-sized thrusters use the average nozzle position (midpoint of a 2-pack,
// centre of a 3-pack). If one thruster is much larger, the jet comes from that one.
// Distinct large thrusters are left as one jet each.
void SelectExhaustJetCandidates(std::vector<ExhaustMountCandidate> &candidates)
{
	const size_t n = candidates.size();
	if (n <= 1) return;

	float minScale = candidates[0].scale;
	float maxScale = candidates[0].scale;
	for (size_t i = 1; i < n; ++i) {
		minScale = std::min(minScale, candidates[i].scale);
		maxScale = std::max(maxScale, candidates[i].scale);
	}

	// If the ship has a mix of large thrusters and small ones, only glob the small ones.
	// If every thruster is a similar size, they are all eligible to glob by proximity.
	const bool hasSizeRange = minScale < SfxParams::EXHAUST_CLUSTER_IGNORE_SIZE_RATIO * maxScale;
	const float smallCut = hasSizeRange ? (SfxParams::EXHAUST_CLUSTER_SMALL_SIZE_FRACTION * maxScale) : (maxScale * 2.0f);

	std::vector<char> suppressed(n, 0);
	for (size_t i = 0; i < n; ++i) {
		for (size_t j = 0; j < n; ++j) {
			if (i == j) continue;
			if (candidates[i].scale >= SfxParams::EXHAUST_CLUSTER_IGNORE_SIZE_RATIO * candidates[j].scale)
				continue;
			if (candidates[i].dir.Dot(candidates[j].dir) < SfxParams::EXHAUST_CLUSTER_MIN_DIR_DOT)
				continue;
			const float distSqr = (candidates[i].pos - candidates[j].pos).LengthSqr();
			const float limit = SfxParams::EXHAUST_CLUSTER_IGNORE_DIST_SCALE * candidates[j].scale;
			if (distSqr <= limit * limit) {
				suppressed[i] = 1;
				break;
			}
		}
	}

	std::vector<size_t> parent(n);
	for (size_t i = 0; i < n; ++i)
		parent[i] = i;

	for (size_t i = 0; i < n; ++i) {
		if (suppressed[i]) continue;
		for (size_t j = i + 1; j < n; ++j) {
			if (suppressed[j]) continue;
			if (candidates[i].scale >= smallCut || candidates[j].scale >= smallCut)
				continue;
			if (candidates[i].dir.Dot(candidates[j].dir) < SfxParams::EXHAUST_CLUSTER_MIN_DIR_DOT)
				continue;
			const float mn = std::min(candidates[i].scale, candidates[j].scale);
			const float mx = std::max(candidates[i].scale, candidates[j].scale);
			if (mn < SfxParams::EXHAUST_CLUSTER_IGNORE_SIZE_RATIO * mx)
				continue;
			// Link packed neighbours only. Union-find transitivity still chains a 3-pack
			// (A-B and B-C) without joining a left pack to a right pack across the hull.
			const float distSqr = (candidates[i].pos - candidates[j].pos).LengthSqr();
			const float limit = SfxParams::EXHAUST_CLUSTER_MERGE_DIST_SCALE * mx;
			if (distSqr > limit * limit)
				continue;
			const size_t ri = ExhaustClusterRoot(parent, i);
			const size_t rj = ExhaustClusterRoot(parent, j);
			if (ri != rj)
				parent[rj] = ri;
		}
	}

	std::vector<std::vector<size_t>> groups(n);
	for (size_t i = 0; i < n; ++i) {
		if (suppressed[i]) continue;
		groups[ExhaustClusterRoot(parent, i)].push_back(i);
	}

	std::vector<ExhaustMountCandidate> selected;
	selected.reserve(n);
	for (size_t r = 0; r < n; ++r) {
		const std::vector<size_t> &members = groups[r];
		if (members.empty()) continue;
		if (members.size() == 1) {
			selected.push_back(candidates[members[0]]);
			continue;
		}

		size_t largest = members[0];
		float groupMaxScale = candidates[largest].scale;
		for (size_t k = 1; k < members.size(); ++k) {
			const size_t idx = members[k];
			if (candidates[idx].scale > groupMaxScale) {
				largest = idx;
				groupMaxScale = candidates[idx].scale;
			}
		}

		// Similar-sized nozzles (within IGNORE_SIZE_RATIO of the largest) contribute to
		// the jet origin. A clearly smaller extra member is skipped so a dominant engine
		// keeps its own nozzle. Equal 2-packs average to the midpoint, 3-packs to the centre.
		vector3f origin(0.f);
		int originCount = 0;
		for (size_t idx : members) {
			if (candidates[idx].scale < SfxParams::EXHAUST_CLUSTER_IGNORE_SIZE_RATIO * groupMaxScale)
				continue;
			origin += candidates[idx].pos;
			++originCount;
		}

		ExhaustMountCandidate jet = candidates[largest];
		jet.pos = origin * (1.0f / float(originCount));
		selected.push_back(jet);
	}

	candidates.swap(selected);
}

} // namespace

void ThrusterExhaustSpawner::RefreshMounts(SceneGraph::Model *model)
{
	m_nozzleLocal.clear();
	m_thrusters.clear();
	if (!model) return;

	std::vector<std::pair<SceneGraph::MatrixTransform *, SceneGraph::Thruster *>> tmp;
	model->GatherThrusterMounts(tmp);

	std::vector<ExhaustMountCandidate> candidates;
	candidates.reserve(tmp.size());
	float maxThrusterScale = 0.0f;
	for (const auto &pr : tmp) {
		const matrix4x4f M = pr.first->CalcGlobalTransform();
		const float sx = vector3f(M[0], M[1], M[2]).Length();
		const float sy = vector3f(M[4], M[5], M[6]).Length();
		const float sz = vector3f(M[8], M[9], M[10]).Length();
		const float scaleAvg = (sx + sy + sz) / 3.0f;
		maxThrusterScale = std::max(maxThrusterScale, scaleAvg);

		ExhaustMountCandidate c;
		c.thr = pr.second;
		c.pos = M.GetTranslate();
		c.dir = pr.second->GetDirection().NormalizedSafe();
		c.scale = scaleAvg;
		candidates.push_back(c);
	}

	// Visual flame size still uses every model thruster, including nozzles we skip for exhaust.
	for (const auto &c : candidates) {
		const float scaleProportional = (maxThrusterScale > 1e-6f) ? (c.scale / maxThrusterScale) : 1.0f;
		c.thr->SetVisualSizeInfo(c.scale, scaleProportional);
	}

	SelectExhaustJetCandidates(candidates);

	m_nozzleLocal.reserve(candidates.size());
	m_thrusters.reserve(candidates.size());
	for (const auto &c : candidates) {
		m_nozzleLocal.push_back(c.pos);
		m_thrusters.push_back(c.thr);
	}
	m_channels.assign(candidates.size(), ExhaustThrusterChannel{});
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
	if (!body || !propulsion || m_nozzleLocal.empty()) return;

	if (m_nozzleLocal.size() != m_thrusters.size())
		return;
	if (m_channels.size() != m_nozzleLocal.size())
		m_channels.assign(m_nozzleLocal.size(), ExhaustThrusterChannel{});

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
	const float particlesPerSecPerThruster = particlesPerSecTotal / m_nozzleLocal.size();

	for (size_t ti = 0; ti < m_nozzleLocal.size(); ++ti) {
		SceneGraph::Thruster *thr = m_thrusters[ti];
		ExhaustThrusterChannel &ch = m_channels[ti];

		const vector3d nozzleLocal = vector3d(m_nozzleLocal[ti]);
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
