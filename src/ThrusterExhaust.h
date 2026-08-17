// Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#ifndef _THRUSTER_EXHAUST_H
#define _THRUSTER_EXHAUST_H

#include "Color.h"
#include "Sfx.h"
#include "vector3.h"

#include <vector>

class Body;
class Propulsion;

namespace SceneGraph {
	class MatrixTransform;
	class Model;
	class Thruster;
} // namespace SceneGraph

// Per-thruster atmospheric exhaust (nozzle anchor + jet backbone)
struct ExhaustThrusterChannel {
	bool hasLastNozzle = false;
	vector3d lastNozzleWorld = vector3d::Zero;
	vector3d lastBackboneVel = vector3d::Zero;
	bool wasThrusterFiring = false;
};

struct ExhaustEnvironment {
	double density = 0.0;
	float opacityAtmosphereFactor = 1.0f;
	vector3f windVel = vector3f(0.f);
	double groundRadius = 0.0;
	Color dustTint = Color(128, 128, 128, 255);
	float baseLifetime = SfxParams::EXHAUST_LIFETIME;
	float maxSpread = SfxParams::EXHAUST_MAX_SPREAD;
};

class ThrusterExhaustSpawner {
public:
	void RefreshMounts(SceneGraph::Model *model);
	void ClearChannelState();
	void Spawn(const Body *body, const Propulsion *propulsion, float timeStep, const ExhaustEnvironment &env, float particlesPerSecTotal);

private:
	std::vector<SceneGraph::MatrixTransform *> m_mounts;
	std::vector<SceneGraph::Thruster *> m_thrusters;
	std::vector<ExhaustThrusterChannel> m_channels;
};

#endif /* _THRUSTER_EXHAUST_H */
