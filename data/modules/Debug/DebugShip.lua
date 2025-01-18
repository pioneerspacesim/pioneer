-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game        = require 'Game'
local HullConfig  = require 'HullConfig'
local ShipDef     = require 'ShipDef'
local Lang        = require 'Lang'
local Timer       = require 'Timer'

local utils       = require 'utils'

local ui = require 'pigui'

local colors = ui.theme.colors
local icons = ui.theme.icons

local ShipTemplates = require 'modules.MissionUtils.ShipTemplates'
local ShipBuilder   = require 'modules.MissionUtils.ShipBuilder'

local Notification = require 'pigui.libs.notification'
local debugView    = require 'pigui.views.debug'

--=============================================================================

local aiOptions = {
	"FlyTo", "Kamikaze", "Kill", "Hold Position"
}

local spawnOptions = {
	"Nearby",
	"Docked",
	"Orbit"
}

local templateOptions = {
	"GenericPirate",
	"StrongPirate",
	"GenericMercenary",
	"GenericPolice",
	"StationPolice",
}

local missileOptions = {
	"S1 Guided Missile",
	"S1 Unguided Missile",
	"S2 Guided Missile",
	"S3 Smart Missile",
	"S4 Naval Missile"
}

local missileTypes = {
	"missile.guided_s1",
	"missile.unguided_s1",
	"missile.guided_s2",
	"missile.smart_s3",
	"missile.naval_s4",
}

---@type HullConfig[]
local shipList = nil

---@type table<string, HullConfig.Slot[]>
local hullSlots = nil

local function buildShipList()
	shipList = {}
	hullSlots = {}

	for _, hull in pairs(HullConfig.GetHullConfigs()) do
		table.insert(shipList, hull)

		---@type HullConfig.Slot[]
		local slots = {}

		for _, slot in pairs(hull.slots) do
			table.insert(slots, slot)
		end

		table.sort(slots, function(a, b) return a.id < b.id end)

		hullSlots[hull.id] = slots
	end

	table.sort(shipList, function(a, b)
		return ShipBuilder.GetHullThreat(a.id).total < ShipBuilder.GetHullThreat(b.id).total
	end)
end

--=============================================================================

---@class Debug.DebugShipTool : UI.Module
local DebugShipTool = utils.class("DebugShipSpawner", require 'pigui.libs.module')

function DebugShipTool:Constructor()
	DebugShipTool:Super().Constructor(self)

	self.selectedHullIdx = nil

	-- Hull Config options
	self.aiCmdIdx = 1
	self.spawnTemplateIdx = 1
	self.spawnThreat = 20.0
	self.spawnDist = 20.0
	self.spawnLocIdx = 1

	self.missileIdx = 1
end

function DebugShipTool:onClearSelection()
	self.selectedHullIdx = nil
end

function DebugShipTool:onSelectHull(idx)
	self.selectedHullIdx = idx
end

function DebugShipTool:onSetSpawnThreat(threat)
	self.spawnThreat = threat
end

function DebugShipTool:onSetSpawnDistance(dist)
	self.spawnDist = dist
end

function DebugShipTool:onSetSpawnAICmd(option)
	self.aiCmdIdx = option
end

function DebugShipTool:onSetSpawnTemplate(option)
	self.spawnTemplateIdx = option
end

function DebugShipTool:onSetSpawnLocation(loc)
	self.spawnLocIdx = loc
end

function DebugShipTool:onSetMissileIdx(missile)
	self.missileIdx = missile
end

--=============================================================================

function DebugShipTool:onSpawnSelectedHull()

	local hull = shipList[self.selectedHullIdx]

	local templateName = templateOptions[self.spawnTemplateIdx]

	---@type MissionUtils.ShipTemplate
	local template = ShipTemplates[templateName]

	template = template:clone {
		shipId = hull.id
	}

	local location = spawnOptions[self.spawnLocIdx]
	local ship ---@type Ship

	if location == "Nearby" then

		ship = ShipBuilder.MakeShipNear(Game.player, template, self.spawnThreat, self.spawnDist, self.spawnDist)

	elseif location == "Docked" then

		local body = Game.player:GetNavTarget()

		if not body or not body:isa("SpaceStation") then
			Notification.add(Notification.Type.Error, "Debug: no station selected")
			return
		end

		---@cast body SpaceStation
		local ship = ShipBuilder.MakeShipDocked(body, template, self.spawnThreat)

		if not ship then
			Notification.add(Notification.Type.Error, "Can't spawn ship docked at {} - no room?" % { body.label })
			return
		end

	elseif location == "Orbit" then

		local body = Game.player:GetNavTarget() or Game.player.frameBody

		assert(body)

		ship = ShipBuilder.MakeShipOrbit(body, template, self.spawnThreat, self.spawnDist, self.spawnDist)

	end

	if self.aiCmdIdx ~= #aiOptions then
		local aiCmd = "AI" .. aiOptions[self.aiCmdIdx]
		ship[aiCmd](ship, Game.player)
	end

	Notification.add(Notification.Type.Info, "Debug: spawned {} nearby" % { ShipDef[hull.id].name })

	Game.player:SetCombatTarget(ship)

end

function DebugShipTool:onSetPlayerShipType()

	local hull = shipList[self.selectedHullIdx]

	local equipSet = Game.player:GetComponent('EquipSet')

	local refundTotal = 0.0

	-- Refund the player the value of their currently equipped items
	-- (Not worth it to try to migrate it)
	for _, equip in pairs(equipSet:GetInstalledEquipment()) do
		refundTotal = refundTotal + equip.price
	end

	Game.player:AddMoney(refundTotal)
	Game.player:SetShipType(hull.id)

	Notification.add(Notification.Type.Info,
		"Debug: set player ship to {}" % { ShipDef[hull.id].name },
		"Refunded {} in equipment value" % { ui.Format.Money(refundTotal) })

end

function DebugShipTool:onSpawnMissile()

	local missile_type = require 'Equipment'.Get(missileTypes[self.missileIdx]) --[[@as Equipment.MissileType?]]

	if not missile_type then
		Notification.add(Notification.Type.Error, "No missile equipment {}" % { missileTypes[self.missileIdx] })
		return
	end

	if missile_type.missile_stats.guided and not Game.player:GetCombatTarget() then
		Notification.add(Notification.Type.Error, "Debug: no target for {}" % { missileOptions[self.missileIdx] })
		return
	end

	local missile = Game.player:SpawnMissile(missile_type.missile_stats, Game.player:GetCombatTarget())

	if not missile then return end

	Timer:CallAt(Game.time + 1, function()
		if missile:exists() then
			missile:Arm()
		end
	end)

end

---@param station SpaceStation
function DebugShipTool:onSetDockedWith(station)
	Game.player:SetDockedWith(station)
end

---@param systemPath SystemPath
function DebugShipTool:onHyperjumpTo(systemPath)
	Game.player:InitiateHyperjumpTo(systemPath, 1.0, 0.0, {})
end

--=============================================================================

function DebugShipTool:drawShipSelector(currentIdx)
	if currentIdx then
		if ui.iconButton("back", icons.decrease_1, "Go Back") then
			self:message('onClearSelection')
		end

		ui.sameLine(0, 2)
	end

	local preview = currentIdx and ShipDef[shipList[currentIdx].id].name or "<No Ship Selected>"

	ui.nextItemWidth(ui.getContentRegion().x - (currentIdx and ui.getFrameHeight() + 2 or 0) * 2)

	ui.comboBox("##HullConfig", preview, function()
		for idx, hull in ipairs(shipList) do
			local clicked = ui.selectable(ShipDef[hull.id].name, idx == currentIdx)

			local threatScore = ShipBuilder.GetHullThreat(hull.id).total
			local threatStr = ui.Format.Number(threatScore, 2) .. " Thr."

			ui.sameLine(ui.getContentRegion().x - ui.calcTextSize(threatStr).x)
			ui.text(threatStr)

			if clicked then
				self:message('onSelectHull', idx)
			end
		end
	end)

	if currentIdx then
		ui.sameLine(0, 2)
		if ui.iconButton("prev", icons.chevron_up, "Select Previous") then
			self:message('onSelectHull', math.max(currentIdx - 1, 1))
		end

		ui.sameLine(0, 2)
		if ui.iconButton("next", icons.chevron_down, "Select Next") then
			self:message('onSelectHull', math.min(currentIdx + 1, #shipList))
		end
	end
end

local function drawKeyValue(name, val)
	ui.tableNextRow()

	ui.tableNextColumn()
	ui.text(name .. ":")

	ui.tableNextColumn()
	ui.text(type(val) == "number" and ui.Format.Number(val) or tostring(val))
end

---@param shipDef ShipDef
function DebugShipTool:drawShipDefInfo(shipDef)
	if ui.beginTable("Ship Def", 2) then

		drawKeyValue("Name", shipDef.name)
		drawKeyValue("Manufacturer", shipDef.manufacturer)
		drawKeyValue("Ship Class", shipDef.shipClass)
		drawKeyValue("Model", shipDef.modelName)
		drawKeyValue("Tag", shipDef.tag)

		drawKeyValue("Cargo Capacity", shipDef.cargo)
		drawKeyValue("Equip Capacity", shipDef.equipCapacity)
		drawKeyValue("Hull Mass", shipDef.hullMass)
		drawKeyValue("Fuel Tank Mass", shipDef.fuelTankMass)
		drawKeyValue("Base Price", shipDef.basePrice)
		drawKeyValue("Min. Crew", shipDef.minCrew)
		drawKeyValue("Max. Crew", shipDef.maxCrew)

		drawKeyValue("Angular Thrust", shipDef.angularThrust)
		drawKeyValue("Foward Thrust", shipDef.linearThrust.FORWARD)
		drawKeyValue("Reverse Thrust", shipDef.linearThrust.REVERSE)
		drawKeyValue("Up Thrust", shipDef.linearThrust.UP)
		drawKeyValue("Down Thrust", shipDef.linearThrust.DOWN)
		drawKeyValue("Left Thrust", shipDef.linearThrust.LEFT)
		drawKeyValue("Right Thrust", shipDef.linearThrust.RIGHT)
		drawKeyValue("Exhaust Velocity", shipDef.effectiveExhaustVelocity)
		drawKeyValue("Pressure Limit", shipDef.atmosphericPressureLimit)

		drawKeyValue("Front Cross-Section", shipDef.frontCrossSec)
		drawKeyValue("Side Cross-Section", shipDef.sideCrossSec)
		drawKeyValue("Top Cross-Section", shipDef.topCrossSec)

		ui.endTable()
	end
end

local function drawSlotValue(slot, key)
	if slot[key] then
		ui.tableNextRow()

		ui.tableNextColumn()
		ui.text(key .. ":")

		local val = slot[key]
		if type(val) == "number" then
			val = ui.Format.Number(val)
		end

		ui.tableNextColumn()
		ui.text(tostring(val))
	end
end

---@param slot HullConfig.Slot
function DebugShipTool:drawSlotDetail(slot)
	if ui.beginTable("SlotDetails", 2) then

		drawSlotValue(slot, "id")
		drawSlotValue(slot, "type")
		drawSlotValue(slot, "size")
		drawSlotValue(slot, "size_min")
		drawSlotValue(slot, "tag")
		drawSlotValue(slot, "default")
		if slot.required then
			drawSlotValue(slot, "required")
		end
		drawSlotValue(slot, "hardpoint")
		drawSlotValue(slot, "count")

		ui.endTable()
	end
end

---@param hull HullConfig
function DebugShipTool:drawHullSlots(hull)
	for _, slot in ipairs(hullSlots[hull.id]) do
		local open = ui.treeNode(slot.id)

		if slot.i18n_key then
			local tl, br = ui.getItemRect()
			local name = Lang.GetResource(slot.i18n_res)[slot.i18n_key]

			local pos = Vector2(ui.getCursorScreenPos().x + ui.getContentRegion().x - ui.calcTextSize(name).x - ui.getItemSpacing().x, tl.y)
			ui.addText(pos, colors.font, name)
		end

		if open then
			self:drawSlotDetail(slot)

			ui.treePop()
		end
	end
end

function DebugShipTool:drawHullThreat(hull)
	local threat = ShipBuilder.GetHullThreat(hull.id)

	if ui.beginTable("Hull Threat", 2) then
		for k, v in pairs(threat) do
			ui.tableNextRow()

			ui.tableNextColumn()
			ui.text(tostring(k)..":")

			ui.tableNextColumn()
			ui.text(type(v) == "number" and ui.Format.Number(v) or tostring(v))
		end

		ui.endTable()
	end
end

function DebugShipTool:drawSpawnButtons()

	local threat, dist, changed

	local halfWidth = (ui.getContentRegion().x - ui.getItemSpacing().x) * 0.5

	-- Template line

	ui.nextItemWidth(halfWidth)
	ui.comboBox("##Template", templateOptions[self.spawnTemplateIdx], function()
		for idx, option in ipairs(templateOptions) do
			if ui.selectable(option, idx == self.spawnTemplateIdx) then
				self:message('onSetSpawnTemplate', idx)
			end
		end
	end)

	ui.sameLine()

	ui.nextItemWidth(halfWidth)
	threat, changed = ui.sliderFloat("##Threat", self.spawnThreat, 5.0, 300.0, "Threat: %.1f")

	if changed then
		self:message('onSetSpawnThreat', threat)
	end

	-- Spawn location line

	ui.nextItemWidth(halfWidth)
	ui.comboBox("##AICmd", aiOptions[self.aiCmdIdx], function()
		for idx, option in ipairs(aiOptions) do
			if ui.selectable(option, idx == self.aiCmdIdx) then
				self:message('onSetSpawnAICmd', idx)
			end
		end
	end)

	ui.sameLine()

	ui.nextItemWidth(halfWidth)
	dist, changed = ui.sliderFloat("##Distance", self.spawnDist, 1.0, 300.0, "Distance: %.1fkm")

	if changed then
		self:message('onSetSpawnDistance', dist)
	end

	-- Button Line

	local buttonSize = Vector2((ui.getContentRegion().x - ui.getItemSpacing().x * 2) / 3, 0)

	if ui.button("Spawn Ship", buttonSize) then
		self:message('onSpawnSelectedHull')
	end

	ui.sameLine()

	ui.nextItemWidth(buttonSize.x)
	ui.comboBox("##SpawnOption", spawnOptions[self.spawnLocIdx], function()
		for idx, option in ipairs(spawnOptions) do
			if ui.selectable(option, idx == self.spawnLocIdx) then
				self:message('onSetSpawnLocation', idx)
			end
		end
	end)

	ui.sameLine()

	if ui.button("Set Ship Type", buttonSize) then
		self:message('onSetPlayerShipType')
	end

end

function DebugShipTool:drawMissileOptions()
	if ui.button("Launch Missile") then
		self:message('onSpawnMissile')
	end

	ui.sameLine()

	ui.addCursorPos(Vector2(0, (ui.getButtonHeight() - ui.getFrameHeight()) * 0.5))

	ui.nextItemWidth(ui.getContentRegion().x)
	ui.comboBox("##missile", missileOptions[self.missileIdx], function()
		for idx, option in ipairs(missileOptions) do
			if ui.selectable(option, idx == self.missileIdx) then
				self:message('onSetMissileIdx', idx)
			end
		end
	end)
end

function DebugShipTool:drawTeleportOptions()
	---@type SystemPath
	local hyperspaceTarget = Game.sectorView:GetSelectedSystemPath()
	local navTarget = Game.player:GetNavTarget()

	ui.horizontalGroup(function()

		if navTarget and navTarget:isa("SpaceStation") then
			if ui.button("Dock with {}" % { navTarget.label }) then
				self:message('onSetDockedWith', navTarget)
			end
		end

		if Game.system and hyperspaceTarget and not hyperspaceTarget:IsSameSystem(Game.system.path) then
			if ui.button("Hyperjump to {}" % { hyperspaceTarget:GetStarSystem().name }) then
				self:message('onHyperjumpTo', hyperspaceTarget)
			end
		end

	end)
end

---@param ship Ship
function DebugShipTool:drawShipEquipment(ship)
	local equipSet = ship:GetComponent('EquipSet')

	if ui.beginTable("shipEquip", 2) then

		for id, equip in pairs(equipSet:GetInstalledEquipment()) do
			drawKeyValue(id, equip:GetName())
		end

		ui.endTable()
	end

	ui.spacing()
end

function DebugShipTool:render()
	self:drawShipSelector(self.selectedHullIdx)

	if self.selectedHullIdx then

		local buttonRowHeight = ui.getFrameHeightWithSpacing() * 2 + ui.getButtonHeightWithSpacing()

		ui.child("innerScroll", Vector2(0, -buttonRowHeight), function()
			local hull = shipList[self.selectedHullIdx]

			if ui.collapsingHeader("ShipDef") then
				self:drawShipDefInfo(ShipDef[hull.id])
			end

			if ui.collapsingHeader("Hull Slots") then
				self:drawHullSlots(hull)
			end

			if ui.collapsingHeader("Hull Threat") then
				self:drawHullThreat(hull)
			end
		end)

		self:drawSpawnButtons()

	else

		ui.separator()

		if not Game.player:IsDocked() then
			self:drawMissileOptions()

			self:drawTeleportOptions()
		end

		local target = Game.player:GetCombatTarget()

		if target and target:isa('Ship') then

			---@cast target Ship

			ui.text("{} ({}) | Equipment:" % { target.label, ShipDef[target.shipId].name })

			ui.child("equipScroll", Vector2(0, 0), function()

				self:drawShipEquipment(target)

			end)

		end

	end
end

--=============================================================================

debugView.registerTab("DebugShip", {
	label = "Ship Debug",
	icon = icons.ship,
	debugUI = DebugShipTool.New(),
	show = function() return Game.player and not Game:InHyperspace() end,
	draw = function(self)
		if not shipList then
			buildShipList()
		end

		self.debugUI:update()
		self.debugUI:render()
	end
})
