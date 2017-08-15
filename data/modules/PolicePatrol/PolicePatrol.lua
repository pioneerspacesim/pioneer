-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Event = import("Event")
local Legal = import("Legal")
local Serializer = import("Serializer")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")
local Timer = import("Timer")

local l = Lang.GetResource("module-policepatrol")
local l_ui_core = Lang.GetResource("ui-core")

-- Fine at which police will hunt down outlaw player
-- This is a copy from CrimeTracking.lua.
local maxFineTolerated = 300

local getNumberOfFlavours = function (str)
	local num = 1

	while l:get(str .. "_" .. num) do
		num = num + 1
	end
	return num - 1
end

local hasIllegalGoods = function (cargo)
	local illegal = false

	for _, e in pairs(cargo) do
		if not Game.system:IsCommodityLegal(e) then
			illegal = true
			break
		end
	end
	return illegal
end


local patrol = {}
local showMercy = true

local attackShip = function (ship)
	for i = 1, #patrol do
		patrol[i]:AIKill(ship)
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil then return end

	for i = 1, #patrol do
		if patrol[i] == ship then
			table.remove(patrol, i)
			if attacker:isa("Ship") and attacker:IsPlayer() then
				showMercy = false
			end
			break
		end
	end
end

local onShipHit = function (ship, attacker)
	if attacker == nil then return end
	if not attacker:isa('Ship') then return end

	-- Support all police, not only the patrol
	local police = ShipDef[Game.system.faction.policeShip]
	if ship.shipId == police.id and attacker.shipId ~= police.id then
		attackShip(attacker)
	end
end

local onJettison = function (ship, cargo)
	if not ship:IsPlayer() or Game.system:IsCommodityLegal(cargo) then return end

	if #patrol > 0 and showMercy then
		local manifest = ship:GetEquip("cargo")
		if not hasIllegalGoods(manifest) then
			Comms.ImportantMessage(l.TAKE_A_HIKE, patrol[1].label)
			for i = 1, #patrol do
				patrol[i]:CancelAI()
			end
		end
	end
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	if not hasIllegalGoods(Equipment.cargo) then return end

	local system = Game.system
	if (1 - system.lawlessness) < Engine.rand:Number(4) then return end

	local crimes, fine = player:GetCrimeOutstanding()
	local ship
	local shipdef = ShipDef[system.faction.policeShip]
	local n = 1 + math.floor((1 - system.lawlessness) * (system.population / 3))
	for i = 1, n do
		ship = Space.SpawnShipNear(shipdef.id, player, 50, 100)
		ship:SetLabel(l_ui_core.POLICE)
		ship:AddEquip(Equipment.laser.pulsecannon_1mw)
		table.insert(patrol, ship)
	end

	if Engine.rand:Number(1) < system.lawlessness then
		-- You are lucky. They are busy, eating donuts ;-)
		Comms.ImportantMessage(string.interp(l["RESPECT_THE_LAW_" .. Engine.rand:Integer(1, getNumberOfFlavours("RESPECT_THE_LAW"))], { system = system.name }), ship.label)
	else
		if fine > maxFineTolerated then
			Comms.ImportantMessage(string.interp(l["OUTLAW_DETECTED_" .. Engine.rand:Integer(1, getNumberOfFlavours("OUTLAW_DETECTED"))], { ship_label = player.label }, ship.label))
			showMercy = false
			attackShip(player)
		else
			Comms.ImportantMessage(l["INITIATE_CARGO_SCAN_" .. Engine.rand:Integer(1, getNumberOfFlavours("INITIATE_CARGO_SCAN"))], ship.label)
			Timer:CallAt(Game.time + Engine.rand:Integer(3, 9), function ()
				local manifest = player:GetEquip("cargo")
				if hasIllegalGoods(manifest) then
					Comms.ImportantMessage(l.ILLEGAL_GOODS_DETECTED, ship.label)
					attackShip(player)
					Comms.ImportantMessage(l["POLICE_TAUNT_" .. Engine.rand:Integer(1, getNumberOfFlavours("POLICE_TAUNT"))], ship.label)
				else
					Comms.ImportantMessage(l.NOTHING_DETECTED, ship.label)
				end
			end)
		end
	end
end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		patrol = {}
		showMercy = true
	end
end


local loaded_data

local onGameStart = function ()
	if loaded_data then
		patrol = loaded_data.patrol
		showMercy = loaded_data.showMercy
		loaded_data = nil
	end
end

local onGameEnd = function ()
	patrol = {}
	showMercy = true
end

local serialize = function ()
	return { patrol = patrol, showMercy = showMercy }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipHit", onShipHit)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("PolicePatrol", serialize, unserialize)

