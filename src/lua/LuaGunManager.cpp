// Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
// Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

#include "LuaMetaType.h"
#include "LuaObject.h"
#include "LuaTable.h"
#include "LuaColor.h"
#include "LuaVector2.h"
#include "ship/GunManager.h"

void pi_lua_generic_pull(lua_State *l, int idx, ProjectileData &out)
{
	luaL_checktype(l, idx, LUA_TTABLE);
	LuaTable tab(l, idx);

	out.damage = tab.Get<float>("damage");
	out.lifespan = tab.Get<float>("lifespan");
	out.speed = tab.Get<float>("speed");

	out.beam = tab.Get<bool>("beam");
	out.mining = tab.Get<bool>("mining");

	out.length = tab.Get<float>("length");
	out.width = tab.Get<float>("width");
	out.color = tab.Get<Color>("color");
}

void pi_lua_generic_pull(lua_State *l, int idx, GunManager::WeaponData &out)
{
	luaL_checktype(l, idx, LUA_TTABLE);
	LuaTable tab(l, idx);

	out.firingRPM = tab.Get<float>("rpm");
	out.firingHeat = tab.Get<float>("heatPerShot");
	out.coolingPerSecond = tab.Get<float>("cooling");
	out.overheatThreshold = tab.Get<float>("overheat");
	out.modelPath = tab.Get<std::string>("model", "");
	out.projectile = tab.Get<ProjectileData>("projectile");
	out.projectileType = out.projectile.beam ? GunManager::PROJECTILE_BEAM : GunManager::PROJECTILE_BALLISTIC;
	out.numBarrels = tab.Get<uint32_t>("numBarrels", 1);
	out.staggerBarrels = tab.Get<bool>("stagger", true);
}

template<>
const char *LuaObject<GunManager>::s_type = "GunManager";
template<>
void LuaObject<GunManager>::RegisterClass()
{
	lua_State *l = Lua::manager->GetLuaState();

	LuaMetaType<GunManager> metaType(s_type);

	metaType.CreateMetaType(l);

	metaType.StartRecording()
		.AddFunction("AddWeaponMount", &GunManager::AddWeaponMount)
		.AddFunction("RemoveWeaponMount", &GunManager::RemoveWeaponMount)
		.AddFunction("MountWeapon", &GunManager::MountWeapon)
		.AddFunction("UnmountWeapon", &GunManager::UnmountWeapon)
		.AddFunction("IsWeaponMounted", &GunManager::IsWeaponMounted)
		.AddFunction("GetNumWeapons", &GunManager::GetNumWeapons)
		.AddFunction("GetWeaponIndexForHardpoint", &GunManager::GetWeaponIndexForHardpoint)
		.AddFunction("SetupDefaultGroups", &GunManager::SetupDefaultGroups)
		.AddFunction("AssignWeaponToGroup", &GunManager::AssignWeaponToGroup)
		.AddFunction("RemoveGroup", &GunManager::RemoveGroup)
		.AddFunction("SetGroupTarget", &GunManager::SetGroupTarget)
		.AddFunction("SetGroupFiring", &GunManager::SetGroupFiring)
		.AddFunction("SetGroupFireWithoutTargeting", &GunManager::SetGroupFireWithoutTargeting)
		.AddFunction("IsFiring", &GunManager::IsFiring)
		.AddFunction("IsGroupFiring", &GunManager::IsGroupFiring);
	metaType.StopRecording();

	LuaObject::CreateClass(&metaType);
}
