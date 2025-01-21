// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "GunManager.h"

#include "Beam.h"
#include "DynamicBody.h"
#include "Game.h"
#include "JsonUtils.h"
#include "ModelBody.h"
#include "Pi.h"
#include "Projectile.h"
#include "Space.h"
#include "lua/LuaBodyComponent.h"
#include "matrix4x4.h"
#include "scenegraph/Tag.h"

REGISTER_COMPONENT_TYPE(GunManager) {
	BodyComponentDB::RegisterComponent<GunManager>("GunManager");
	BodyComponentDB::RegisterSerializer<GunManager>();
	BodyComponentDB::RegisterLuaInterface<GunManager>();
}

// =============================================================================

// Forward-declared serialization code
void from_json(const Json &obj, GunManager::WeaponData &data);
void to_json(Json &out, const GunManager::WeaponData &data);
void from_json(const Json &obj, GunManager::WeaponState &weapon);
void to_json(Json &out, const GunManager::WeaponState &weapon);
void from_json(const Json &obj, GunManager::WeaponMount &mount);
void to_json(Json &out, const GunManager::WeaponMount &mount);
void from_json(const Json &obj, GunManager::GroupState &group);
void to_json(Json &out, const GunManager::GroupState &group);

// =============================================================================

void GunManager::Init(ModelBody *b)
{
	m_parent = b;

	// Ensure we always have a default group for weapons
	if (m_groups.empty()) {
		m_groups.push_back({});
	}
}

void GunManager::SaveToJson(Json &out, Space *space)
{
	out["isAnyFiring"] = m_isAnyFiring;
	out["coolingBoost"] = m_coolingBoost;

	Json &mounts = (out["mounts"] = Json::object());
	for (auto &pair : m_mounts) {
		mounts.emplace(pair.first, pair.second);
	}

	Json &weapons = (out["weapons"] = Json::array());
	for (auto &weapon : m_weapons) {
		weapons.push_back(weapon);
	}

	Json &groups = (out["groups"] = Json::array());
	for (auto &group : m_groups) {
		groups.push_back(group);

		if (group.target) {
			groups.back()["target"] = space->GetIndexForBody(group.target);
		}
	}
}

void GunManager::LoadFromJson(const Json &jsonObj, Space *space)
{
	m_isAnyFiring = jsonObj.value<bool>("isAnyFiring", false);
	m_coolingBoost = jsonObj.value<float>("coolingBoost", 1.0);

	const Json &mounts = jsonObj["mounts"];
	for (const auto &pair : mounts.items()) {
		WeaponMount mount = pair.value().get<WeaponMount>();

		// Find the tag for this weapon mount
		mount.tag = m_parent->GetModel()->FindTagByName(pair.value()["tag"]);

		m_mounts.try_emplace(std::string_view(pair.key()), std::move(mount));
	}

	const Json &weapons = jsonObj["weapons"];
	for (const auto &weapon : weapons) {
		m_weapons.push_back(weapon.get<WeaponState>());

		// Fixup the mount pointer
		StringName mount = weapon["mount"].get<std::string_view>();
		m_weapons.back().mount = &m_mounts[mount];
	}

	m_groups.clear();

	const Json &groups = jsonObj["groups"];
	for (const auto &group : groups) {
		m_groups.push_back(group.get<GroupState>());
	}
}

void GunManager::PostLoadFixup(Space *space)
{
	for (size_t groupIdx = 0; groupIdx < m_groups.size(); groupIdx++) {
		GroupState &gs = m_groups[groupIdx];

		if (gs.target_idx == 0)
			continue;

		Body *target = space->GetBodyByIndex(gs.target_idx);

		if (!target) {
			Log::Warning("Could not find target body index {} for ship {} (weapon group {})",
				gs.target_idx, m_parent->GetLabel(), groupIdx);
			continue;
		}

		SetGroupTarget(groupIdx, target);
	}
}

bool GunManager::AddWeaponMount(const StringName &id, const StringName &tagName, vector2f gimbalLimit)
{
	WeaponMount mount = {};

	mount.id = id;
	mount.tag = m_parent->GetModel()->FindTagByName(tagName);
	mount.gimbalLimitTan = vector2f(
		tan(DEG2RAD(gimbalLimit.x)),
		tan(DEG2RAD(gimbalLimit.y))
	);

	auto result = m_mounts.try_emplace(id, mount);
	return result.second;
}

bool GunManager::RemoveWeaponMount(const StringName &id)
{
	auto iter = m_mounts.find(id);
	if (iter == m_mounts.end())
		return false;

	if (IsWeaponMounted(id))
		return false;

	m_mounts.erase(iter);
	return true;
}

bool GunManager::MountWeapon(const StringName &hardpoint, const WeaponData &gunData)
{
	auto iter = m_mounts.find(hardpoint);
	if (iter == m_mounts.end())
		return false;

	if (IsWeaponMounted(hardpoint))
		return false;

	WeaponState ws = {};

	ws.mount = &iter->second;
	ws.data = gunData;

	// TODO: render weapon models on hardpoint
	// if (!gunData.modelPath.empty()) {
	// 	ws.model = Pi::FindModel(gunData.modelPath);
	// }

	// assign the new weapon to group zero
	// TODO: make a new group for this weapon?
	m_groups.front().weapons[m_weapons.size()] = true;
	m_weapons.push_back(ws);

	return true;
}

void GunManager::UnmountWeapon(const StringName &hardpoint)
{
	auto iter = std::find_if(m_weapons.begin(), m_weapons.end(), [&](const WeaponState &ws) {
		return ws.mount->id == hardpoint;
	});

	if (iter != m_weapons.end()) {
		uint32_t index = std::distance(m_weapons.begin(), iter);

		// Update all group weapon assignments
		for (auto &group : m_groups) {
			RemoveGroupIndex(group.weapons, index);
		}

		// TODO: any additional cleanup needed for other systems that hook into GunManager, e.g. heat, targeting, etc.
		m_weapons.erase(iter);
	}
}

bool GunManager::IsWeaponMounted(const StringName &hardpoint) const
{
	return std::find_if(m_weapons.begin(), m_weapons.end(), [&](const WeaponState &ws) {
		return ws.mount->id == hardpoint;
	}) != m_weapons.end();
}

void GunManager::RemoveGroupIndex(WeaponIndexSet &group, uint32_t index)
{
	// Mask containing all of the indices below index
	WeaponIndexSet keep = WeaponIndexSet().set() >> (NUM_WEAPON_INDICES - index);
	// Mask containing all of the indices above index
	WeaponIndexSet mask = WeaponIndexSet().set() << (index + 1);

	// Decrement all contained indices in the set above the removed index down by one
	group = (group & keep) | ((group & mask) >> 1);
}

uint32_t GunManager::GetWeaponIndexForHardpoint(const StringName &hardpoint) const
{
	auto iter = std::find_if(m_weapons.begin(), m_weapons.end(), [&](const WeaponState &ws) {
		return ws.mount->id == hardpoint;
	});

	if (iter == m_weapons.end()) {
		return UINT32_MAX;
	}

	return std::distance(m_weapons.begin(), iter);
}

void GunManager::SetupDefaultGroups()
{
	m_groups.clear();

	GroupState group = {};

	for (size_t i = 0; i < m_weapons.size(); i++) {
		group.weapons[i] = true;
	}

	m_groups.push_back(group);
}

void GunManager::SetAllGroupsFiring(bool firing)
{
	for (size_t idx = 0; idx < m_groups.size(); idx++) {
		SetGroupFiring(idx, firing);
	}

	m_isAnyFiring = firing;
}

void GunManager::AssignWeaponToGroup(uint32_t numWeapon, uint32_t group)
{
	if (numWeapon > m_weapons.size())
		return;

	if (group >= m_groups.size()) {
		m_groups.resize(group + 1);
	}

	WeaponState &ws = m_weapons[numWeapon];

	uint32_t oldGroup = ws.group;
	m_groups[oldGroup].weapons[numWeapon] = false;

	// Prune empty groups when the last weapon is removed
	if (m_groups[oldGroup].weapons.none()) {
		RemoveGroup(oldGroup);
	}

	ws.group = group;
	m_groups[group].weapons[numWeapon] = true;
}

void GunManager::RemoveGroup(uint32_t group)
{
	if (group == 0 || group >= m_groups.size())
		return;

	GroupState &gzero = m_groups[0];

	for (size_t idx = 0; idx < m_weapons.size(); idx++) {
		WeaponState &weapon = m_weapons[idx];

		// Reassign weapons from this group to group zero
		if (weapon.group == group) {
			weapon.group = 0;
			gzero.weapons[idx] = true;
		}

		// Reduce index of all groups above this
		if (weapon.group > group) {
			weapon.group--;
		}
	}

	// Finally, clear the group from the list
	m_groups.erase(m_groups.begin() + group);
}

const Body *GunManager::GetGroupTarget(uint32_t group)
{
	if (group >= m_groups.size())
		return nullptr;

	return m_groups[group].target;
}

void GunManager::SetGroupTarget(uint32_t group, const Body *target)
{
	if (group >= m_groups.size())
		return;

	GroupState &gs = m_groups[group];
	// No weapons at all, ignore
	if (!gs.weapons.any())
		return;

	// Clear the prior callback, if any
	if (gs.target)
		gs.onTargetDestroyed.disconnect();

	m_groups[group].target = target;

	if (target) {
		gs.onTargetDestroyed = target->onDelete.connect([=]() {
			this->SetGroupTarget(group, nullptr);
		});
	}
}

void GunManager::SetGroupFiring(uint32_t group, bool firing)
{
	if (group >= m_groups.size())
		return;

	GroupState &gs = m_groups[group];
	// No weapons at all, ignore
	if (!gs.weapons.any())
		return;

	gs.firing = firing;
	m_isAnyFiring |= firing;

	if (firing) {
		// Update the next-firing time for all weapons in this group so they actually fire
		// (Ensures you can't spam the fire command to shoot more rapidly than the weapon's rate of fire)
		for (size_t idx = 0; idx < m_weapons.size(); idx++) {
			if (gs.weapons[idx]) {
				m_weapons[idx].nextFireTime = std::max(m_weapons[idx].nextFireTime, Pi::game->GetTime());
			}
		}
	} else {
		m_stoppedNextFrame |= gs.weapons;
	}
}

void GunManager::SetGroupFireWithoutTargeting(uint32_t group, bool enabled)
{
	if (group >= m_groups.size())
		return;

	GroupState &gs = m_groups[group];
	gs.fireWithoutTargeting = enabled;
}

vector3d GunManager::GetGroupLeadPos(uint32_t group)
{
	if (group >= m_groups.size() || !m_groups[group].target || m_groups[group].weapons.none())
		return vector3d(0, 0, 0);

	GroupState &gs = m_groups[group];
	double inv_count = 1.0 / double(gs.weapons.count());
	vector3d lead_pos = vector3d(0, 0, 0);

	for (size_t idx = 0; idx < m_weapons.size(); idx++) {
		if (gs.weapons[idx]) {
			lead_pos += m_weapons[idx].currentLeadPos * inv_count;
		}
	}

	return lead_pos;
}

float GunManager::GetGroupTemperatureState(uint32_t group)
{
	if (group >= m_groups.size() || m_groups[group].weapons.none())
		return 0.f;

	GroupState &gs = m_groups[group];
	double inv_count = 1.0 / double(gs.weapons.count());
	float temperature = 0.f;

	for (size_t idx = 0; idx < m_weapons.size(); idx++) {
		if (gs.weapons[idx]) {
			temperature += inv_count * (m_weapons[idx].temperature / m_weapons[idx].data.overheatThreshold);
		}
	}

	return temperature;
}

void GunManager::StaticUpdate(float deltaTime)
{
	bool isAnyFiring = false;

	m_stoppedThisFrame = m_stoppedNextFrame;
	m_stoppedNextFrame.reset();
	m_firedThisFrame.reset();

	// TODO(sensors): go through groups and collate sensor information on group target

	for (WeaponState &gun : m_weapons) {

		GroupState &gs = m_groups[gun.group];

		bool isBeam = gun.data.projectile.beam || gun.data.projectileType == PROJECTILE_BEAM;

		// Compute weapon lead direction
		if (gs.target) {
			const matrix3x3d &orient = m_parent->GetOrient();
			const vector3d relPosition = gs.target->GetPositionRelTo(m_parent);
			const vector3d relVelocity = gs.target->GetVelocityRelTo(m_parent->GetFrame()) - m_parent->GetVelocity();
			vector3d relAccel = vector3d(0, 0, 0);

			if (gs.target->IsType(ObjectType::DYNAMICBODY)) {
				relAccel = static_cast<const DynamicBody *>(gs.target)->GetLastForce() / gs.target->GetMass();
			}

			// bring position, velocity and acceleration into ship-space
			CalcWeaponLead(gun, relPosition * orient, relVelocity * orient, relAccel * orient);
		} else {
			gun.currentLead = vector3f(0, 0, 1);
			gun.currentLeadPos = vector3d(0, 0, 0);
		}

		// Update gun cooling per tick
		gun.temperature = std::max(0.0f, gun.temperature - gun.data.coolingPerSecond * m_coolingBoost * deltaTime);
		double currentTime = Pi::game->GetTime();

		// determine if we should fire this update
		isAnyFiring |= gs.firing;

		// determine if something prevents this gun from firing
		// Temperature, gimbal checks, etc.
		bool canFire = gun.temperature < gun.data.overheatThreshold && (gs.fireWithoutTargeting || gs.target && gun.withinGimbalLimit);

		if (gs.firing && currentTime >= gun.nextFireTime) {

			// Determine how much time we missed since the gun was supposed to fire
			double missedTime = currentTime - gun.nextFireTime;
			// time between shots, used to determine how many shots we need to 'catch up' on this timestep
			double deltaShot = 60.0 / gun.data.firingRPM;
			// only fire multiple shots per timestep if the accumulated error and the length of the timestep require it
			// given that timescale is set to 1 while in combat, this is likely not going to be required except for NPCs
			uint32_t numShots = 1 + floor((missedTime + deltaTime) / deltaShot);

			for (uint32_t i = 0; i < numShots; ++i) {
				Fire(gun);
			}

			// set the next fire time, making sure to preserve accumulated (fractional) shot time
			gun.nextFireTime += deltaShot * numShots;

			size_t weaponIdx = std::distance(&m_weapons.front(), &gun);
			m_firedThisFrame[weaponIdx] = true;

		}

		// ensure next fire time is properly handled when the gun is meant to be firing
		// but unable to fire (e.g. during gun overheat)
		if (gs.firing)
			gun.nextFireTime = std::max(gun.nextFireTime, currentTime);

	}

	m_isAnyFiring = isAnyFiring;
}

void GunManager::Fire(WeaponState &weapon)
{
	WeaponData *data = &weapon.data;

	// either fire the next barrel in sequence or fire all at the same time
	size_t firstBarrel = 0;
	size_t numBarrels = 1;
	if (data->staggerBarrels || data->numBarrels == 1) {
		firstBarrel = (weapon.lastBarrel + 1) % data->numBarrels;
		weapon.lastBarrel = firstBarrel;
	} else {
		numBarrels = data->numBarrels;
	}

	const matrix4x4f &xform = GetMountTransform(weapon);
	const vector3d wpn_pos = vector3d(xform.GetTranslate());
	const matrix3x3f wpn_orient = xform.GetOrient();

	const matrix3x3d &orient = m_parent->GetOrient();

	// mount-relative aiming direction
	const vector3d leadDir = vector3d(wpn_orient * weapon.currentLead).Normalized();

	for (size_t idx = firstBarrel; idx < firstBarrel + numBarrels; idx++) {
		weapon.temperature += data->firingHeat;
		// TODO: get individual barrel locations from gun model and cache them
		const vector3d dir = orient * leadDir;
		const vector3d pos = orient * wpn_pos + m_parent->GetPosition();

		// TODO: projectile system using new ProjectileDef data
		if (data->projectile.beam) {
			Beam::Add(m_parent, data->projectile, pos, m_parent->GetVelocity(), dir);
		} else {
			Projectile::Add(m_parent, data->projectile, pos, m_parent->GetVelocity(), data->projectile.speed * dir);
		}
	}
}

// Note that position and relative velocity are in the coordinate system of the host body
void GunManager::CalcWeaponLead(WeaponState &state, vector3d position, vector3d relativeVelocity, vector3d relativeAccel)
{
	// Compute the forward vector for the weapon mount
	// NOTE: weapons by convention use +Z as their "forward" vector,
	// as this is the most natural mapping to the content authoring pipeline for ships
	const matrix4x4f &xform = GetMountTransform(state);
	const vector3f forward = vector3f(0, 0, 1);

	if (state.data.projectileType == PROJECTILE_BALLISTIC) {
		// Calculate firing solution and relative velocity along our z axis by
		// computing the position along the enemy ship's lead vector at which to aim
		const double projspeed = state.data.projectile.speed;
		// Account for the distance between the weapon mount and the center of the parent
		position -= vector3d(xform.GetTranslate());

		//Exact lead calculation. We start with:
		// |targpos * l + targvel| = projspeed
		//we solve for l which can be interpreted as 1/time for the projectile to reach the target
		//it gives:
		// |targpos|^2 * l^2 + targpos*targvel * 2l + |targvel|^2 - projspeed^2 = 0;
		// so it gives scalar quadratic equation with two possible solutions - we care only about the positive one - shooting forward
		// A basic math for solving, there is probably more elegant and efficient way to do this:
		double a = position.LengthSqr();
		double b = position.Dot(relativeVelocity) * 2;
		double c = relativeVelocity.LengthSqr() - projspeed * projspeed;
		double delta = b * b - 4 * a * c;

		vector3d leadPos = position;

		if (delta >= 0) {
			//l = (-b + sqrt(delta)) / 2a; t=1/l; a>0
			double t = 2 * a / (-b + sqrt(delta));

			if (t < 0 || t > state.data.projectile.lifespan) {
				//no positive solution or target too far
			} else {
				//This is an exact solution as opposed to 2 step approximation used before.
				//It does not improve the accuracy as expected though.
				//If the target is accelerating and is far enough then this aim assist will
				//actually make sure that it is mpossible to hit..
				leadPos = position + relativeVelocity * t;

				//lets try to adjust for acceleration of the target ship
				//s=a*t^2/2 -> hitting steadily accelerating ships works at much greater distance
				leadPos += relativeAccel * t * t * 0.5;
			}
		} else {
			//no solution
		}

		state.currentLeadPos = leadPos;
	} else if (state.data.projectileType == PROJECTILE_BEAM) {
		// Beam weapons should just aim at the target
		state.currentLeadPos = position;
	}

	// Transform the target's direction into the coordinate space of the mount,
	// with the barrel pointing "forward" towards +Z.
	// float has plenty of precision when working with normalized directions.
	vector3f targetDir = vector3f(state.currentLeadPos.Normalized()) * xform.GetOrient();

	// We represent the maximum traverse of the weapon as an ellipse relative
	// to the +Z axis of the mount.
	// To determine whether the lead target is within this traverse, we modify
	// the coordinate system such that the ellipse becomes the unit circle in
	// 2D space, and test the length of the 2D components of the direction
	// vector.
	// Note that we scale the targetDir vector such that the z component has a length of 1.0,
	// so that the comparison with the tangent of the gimbal limit is correct.
	vector2f traverseRel = (targetDir * (1.0 / targetDir.z)).xy() / state.mount->gimbalLimitTan;

	state.withinGimbalLimit = targetDir.z > 0 && traverseRel.LengthSqr() <= 1.0;
	state.currentLead = state.withinGimbalLimit ? targetDir : forward;
}

// Default matrix to use +Z as weapon forward
static const matrix4x4f s_noMountTransform = matrix4x4f::RotateXMatrix(M_PI);

const matrix4x4f &GunManager::GetMountTransform(WeaponState &state)
{
	if (state.mount->tag) {
		return state.mount->tag->GetGlobalTransform();
	}

	return s_noMountTransform;
}

// =============================================================================

// JSON serialization functions

void from_json(const Json &obj, GunManager::WeaponData &data)
{
	data.firingRPM = obj.value("rpm", 1.f);
	data.firingHeat = obj.value("heat", 0.f);
	data.coolingPerSecond = obj.value("cooling", 0.f);
	data.overheatThreshold = obj.value("overheat", 1.f);

	data.projectileType = GunManager::ProjectileType(obj.value("type", 0));
	data.numBarrels = obj.value("numBarrels", 1);
	data.staggerBarrels = obj.value("staggerBarrels", false);

	data.modelPath = obj.value("modelPath", std::string());

	data.projectile = {};

	if (const Json &pr = obj.value("projectile", Json()); pr.is_object()) {
		ProjectileData &out = data.projectile;
		out.lifespan = pr.value("lifespan", 0.f);
		out.damage = pr.value("damage", 0.f);
		out.length = pr.value("length", 0.f);
		out.width = pr.value("width", 0.f);
		out.speed = pr.value("speed", 0.f);
		out.color = pr.value("color", Color());
		out.mining = pr.value("mining", false);
		out.beam = pr.value("beam", false);
	}
}

void to_json(Json &out, const GunManager::WeaponData &data)
{
	out["rpm"] = data.firingRPM;
	out["heat"] = data.firingHeat;
	out["cooling"] = data.coolingPerSecond;
	out["overheat"] = data.overheatThreshold;

	out["type"] = int(data.projectileType);
	out["numBarrels"] = data.numBarrels;
	out["staggerBarrels"] = data.staggerBarrels;

	if (!data.modelPath.empty()) {
		out["modelPath"] = data.modelPath;
	}

	Json &outProj = (out["projectile"] = Json::object());
	const ProjectileData &pr = data.projectile;

	outProj["lifespan"] = pr.lifespan;
	outProj["damage"] = pr.damage;
	outProj["length"] = pr.length;
	outProj["width"] = pr.width;
	outProj["speed"] = pr.speed;
	outProj["color"] = pr.color;
	outProj["mining"] = pr.mining;
	outProj["beam"] = pr.beam;
}

void from_json(const Json &obj, GunManager::WeaponState &weapon)
{
	weapon.group = obj.value("group", 0);
	weapon.lastBarrel = obj.value("lastBarrel", 0);
	weapon.temperature = obj.value("temperature", 0.f);
	weapon.nextFireTime = obj.value("nextFireTime", 0.0);
	weapon.data = obj["data"].get<GunManager::WeaponData>();
}

void to_json(Json &out, const GunManager::WeaponState &weapon)
{
	out["mount"] = weapon.mount->id;
	out["group"] = weapon.group;
	out["lastBarrel"] = weapon.lastBarrel;
	out["temperature"] = weapon.temperature;
	out["nextFireTime"] = weapon.nextFireTime;

	// All lead calculation variables are transient and will be recreated on next timestep
	// Weapon model is reconstructed from WeaponData
	out["data"] = weapon.data;
}

void from_json(const Json &obj, GunManager::WeaponMount &mount)
{
	mount.id = obj["id"].get<StringName>();
	// mount tag will be fixed up on load
	mount.tag = nullptr;
	mount.gimbalLimitTan = obj["gimbalLimit"].get<vector2f>();
}

void to_json(Json &out, const GunManager::WeaponMount &mount)
{
	out["id"] = mount.id;
	out["tag"] = mount.tag ? mount.tag->GetName() : "";
	out["gimbalLimit"] = mount.gimbalLimitTan;
}

void from_json(const Json &obj, GunManager::GroupState &group)
{
	if (const Json &weapons = obj.value("weapons", Json()); weapons.is_array()) {
		for (const auto &index : weapons) {
			group.weapons[index.get<size_t>()] = true;
		}
	}

	// Target will be fixed up externally
	group.target_idx = obj.value<uint32_t>("target", 0);
	group.target = nullptr;
	group.firing = obj.value("firing", false);
	group.fireWithoutTargeting = obj.value("fireWithoutTargeting", false);
}

void to_json(Json &out, const GunManager::GroupState &group)
{
	Json &weapons = (out["weapons"] = Json::array());
	for (size_t idx = 0; idx < group.weapons.size(); idx++) {
		if (group.weapons[idx]) {
			weapons.push_back(idx);
		}
	}

	// Target is added externally, needs a reference to Space
	out["firing"] = group.firing;
	out["fireWithoutTargeting"] = group.fireWithoutTargeting;
}
