-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This file implements type information about C++ classes for Lua static analysis

---@meta

---@class GunManager
local GunManager = {}

---@param id string
---@param tag string
---@param gimbal Vector2
function GunManager:AddWeaponMount(id, tag, gimbal) end

---@param id string
function GunManager:RemoveWeaponMount(id) end

---@param mount string
---@param weaponData table
---@return boolean
function GunManager:MountWeapon(mount, weaponData) end

---@param mount string
function GunManager:UnmountWeapon(mount) end

---@param mount string
function GunManager:IsWeaponMounted(mount) end

-- TODO...
