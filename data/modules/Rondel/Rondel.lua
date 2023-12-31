-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local Comms = require 'Comms'
local Event = require 'Event'
local Legal = require 'Legal'
local Serializer = require 'Serializer'
local Equipment = require 'Equipment'
local ShipDef = require 'ShipDef'
local SystemPath = require 'SystemPath'
local Timer = require 'Timer'
local Commodities = require 'Commodities'

--local Character = require 'Character'

local l_rondel = Lang.GetResource("module-rondel")
local l_ui_core = Lang.GetResource("ui-core")

local patrol = {}
local shipFiring = false
local jetissionedCargo = false

local rondel_victory = false -- has the player defeated all Rondel guards once in their career?
local rondel_prize = false -- whether the player has been rewarded or not

local rondel_syspath = SystemPath.New(-1,6,2,0)

local ads = {}

local onChat = function (form, ref, option)
	local ad = ads[ref]
	form:Clear()

	form:SetTitle(l_rondel.SOLFED_INTEL)

	if option == -1 then
		form:Close()
		return
	end

	form:SetMessage(l_rondel.SOLFED_INTEL_INTRO)

	if option == 1 then
		ads[ref] = nil
		form:RemoveAdvertOnClose()
		form:SetMessage(l_rondel.PERFECT)
		Game.player:AddMoney(250000)
		rondel_prize = true
		return
	elseif option == 2 then
		form:SetMessage(l_rondel.YOU_VISITED_STAR_SYSTEM)
	elseif option == 3 then
		form:SetMessage(l_rondel.CREDITS_PROMISE)
	end

	form:AddOption(l_rondel.YOU_SHALL_HAVE_THE_DATA, 1)
	form:AddOption(l_rondel.WHAT_IS_THIS_ABOUT, 2)
	form:AddOption(l_rondel.NOT_INTERESTED, 3)

end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
	if rondel_prize or not rondel_victory then return end

	local ad = {
		title    = l_rondel.HEY_OVER_HERE,
		--message  = "",
		station  = station,
		--character = Character.New({armour=false}),
	}

	local ref = station:AddAdvert({
		description = ad.title,
		--icon        = "",
		onChat      = onChat,
		onDelete    = onDelete})
	ads[ref] = ad
end

local attackShip = function (ship)
	for i = 1, #patrol do
		patrol[i]:AIKill(ship)
	end
end

local onShipDestroyed = function (ship, attacker)
	if ship:IsPlayer() or attacker == nil or not Game.system.path:IsSameSystem(rondel_syspath) then return end

	for i = 1, #patrol do
		if patrol[i] == ship then
			table.remove(patrol, i)
			if #patrol > 1 then
				Comms.ImportantMessage(l_rondel.DEFENSE_CRAFT_ELIMINATED, patrol[1].label)
			elseif #patrol == 0 then
				rondel_victory = true
			end
			break
		elseif patrol[i] == attacker then
			Comms.ImportantMessage(l_rondel.INTERCEPTION_SUCCESSFUL, attacker.label)
			break
		end
	end
end

local onShipFiring = function (ship)
	if not Game.system.path:IsSameSystem(rondel_syspath) then return end

	local police = ShipDef[Game.system.faction.policeShip]
	if ship.shipId ~= police.id then
		for i = 1, #patrol do
			if ship:DistanceTo(patrol[i]) <= 4000000 and not shipFiring then
				shipFiring = true
				Comms.ImportantMessage(string.interp(l_rondel.WEAPONS_EVASIVE_ACTION, patrol[1].label))
				attackShip(ship)
				break
			end
		end
	end
end

local onJettison = function (ship, cargo)
	if not Game.system.path:IsSameSystem(rondel_syspath) then return end
	if not jetissionedCargo and #patrol > 1 then
		Comms.ImportantMessage(l_rondel.JETTISON_DEFENSIVE_PRECAUTION, patrol[1].label)
		jetissionedCargo = true
	end
	attackShip(ship)
end

local onEnterSystem = function (player)
	if not player:IsPlayer() then return end

	local system = Game.system
	if not system.path:IsSameSystem(rondel_syspath) then return end

	local tolerance = 1
	local hyperdrive = Game.player:GetEquip('engine',1)
	if hyperdrive.fuel == Commodities.military_fuel then
		tolerance = 0.5
	end

	local ship
	local shipdef = ShipDef[system.faction.policeShip]
	for i = 1, 7 do
		ship = Space.SpawnShipNear(shipdef.id, player, 50, 100)
		ship:SetLabel(l_rondel.HABER_DEFENSE_CRAFT)
		ship:AddEquip(Equipment.laser.pulsecannon_2mw)
		table.insert(patrol, ship)
	end

	Game.SetTimeAcceleration("1x")
	Timer:CallAt(Game.time + 2, function ()
		Comms.ImportantMessage(string.interp(l_rondel.RONDEL_RESTRICTED_ZONE, {seconds = tostring(600*tolerance), playerShipLabel = Game.player:GetLabel()}), ship.label)
		Timer:CallAt(Game.time + 600*tolerance, function()
			if #patrol > 1 then
				attackShip(player)
				Comms.ImportantMessage(l_rondel.HOSTILE_ACTION_REPORTED, ship.label)
			end
		end)
	end)

end

local onLeaveSystem = function (ship)
	if ship:IsPlayer() then
		shipFiring = false
		jetissionedCargo = false
		patrol = {}
	end
end

local loaded_data

local onGameStart = function ()
	shipFiring = false
	jetissionedCargo = false
	if loaded_data then
		patrol = loaded_data.patrol
		rondel_prize = loaded_data.rondel_prize
		rondel_victory = loaded_data.rondel_victory
		if loaded_data.ads then
			for k,ad in pairs(loaded_data.ads) do
			local ref = ad.station:AddAdvert({
				description = ad.title,
				--icon        = "",
				onChat      = onChat,
				onDelete    = onDelete})
			ads[ref] = ad
		end
	end
		loaded_data = nil
	end
end

local onGameEnd = function ()
	shipFiring = false
	jetissionedCargo = false
	rondel_prize = false
	rondel_victory = false
	patrol = {}
end

local serialize = function ()
	return { patrol = patrol, ads = ads, rondel_prize = rondel_prize, rondel_victory = rondel_victory }
end

local unserialize = function (data)
	loaded_data = data
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onLeaveSystem", onLeaveSystem)
Event.Register("onShipDestroyed", onShipDestroyed)
Event.Register("onShipFiring", onShipFiring)
Event.Register("onJettison", onJettison)
Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)

Serializer:Register("Rondel", serialize, unserialize)
