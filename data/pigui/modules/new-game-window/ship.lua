-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Lang = require 'Lang'
local leq = Lang.GetResource("equipment-core")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local utils = require 'utils'
local ShipDef = require 'ShipDef'
local ShipObject = require 'Ship'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local ModelSkin = require 'SceneGraph.ModelSkin'
local Commodities = require 'Commodities'
local Equipment = require 'Equipment'
local Engine = require 'Engine'
local ShipNames = require 'pigui.modules.new-game-window.ship-names'
local textTable = require 'pigui.libs.text-table'

local Defs = require 'pigui.modules.new-game-window.defs'
local GameParam = require 'pigui.modules.new-game-window.game-param'
local Widgets = require 'pigui.modules.new-game-window.widgets'
local Helpers = require 'pigui.modules.new-game-window.helpers'
local Crew = require 'pigui.modules.new-game-window.crew'

local layout = {}
local ShipModel = {}
local ShipSummary = {}

--
-- ship type
--
-- value: string, shipID
--
local ShipType = GameParam.New(lui.SHIP_TYPE, "ship.type")

ShipType.shipIDs = utils.build_array(
	utils.map(function(k, v) return k, k end,
	  utils.filter(function(k, v) return v.tag == 'SHIP' end, pairs(ShipDef))))
ShipType.shipNames = {}
ShipType.idMap = {}
ShipType.selected = 0

table.sort(ShipType.shipIDs)

for _, id in ipairs(ShipType.shipIDs) do
	table.insert(ShipType.shipNames, ShipDef[id].name)
	ShipType.idMap[id] = true
end

ShipType.value = ShipType.shipIDs[1]

function ShipType:draw()
	Widgets.alignLabel(lui.SHIP_TYPE, ShipType.layout, function()
		local changed, ret = Widgets.combo(self.lock, "##shipNames", self.selected, self.shipNames)
		if changed then
			self.selected = ret
			self.value = self.shipIDs[ret + 1]
			ShipModel.value.pattern = 1
			ShipModel:updateModel()
		end
	end)
end

function ShipType:setShipID(shipID)
	local index = utils.indexOf(ShipType.shipIDs, shipID)
	assert(index, "unknown ship ID: " .. tostring(shipID))
	ShipType.value = shipID
	self.selected = index - 1
end

function ShipType:fromStartVariant(variant)
	self:setShipID(variant.shipType)
	self.lock = true
end

function ShipType:isValid()
	return self.value and self.idMap[self.value]
end

ShipType.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local shipID, errorString = Helpers.getPlayerShipParameter(saveGame, "model_body/model_name")
		if errorString then return nil, errorString end
		return shipID
	end
}}


--
-- ship name
--
-- value: string
--
local ShipName = GameParam.New(lui.SHIP_NAME, "ship.name")
ShipName.value = ""
ShipName.layout = {}
function ShipName:draw()
	Widgets.alignLabel(lui.SHIP_NAME, self.layout, function()
		local txt, changed = Widgets.inputText(self.lock, self:isValid(), "##shipname", self.value, function()
			return ShipNames.generateRandom()
		end)
		if changed then self.value = txt end
	end)
end

function ShipName:fromStartVariant(variant)
	self.value = ShipNames.generateRandom()
	self.lock = false
end

function ShipName:isValid()
	-- I think 50 characters is enough
	return self.value and #self.value > 0 and #self.value < 50
end

ShipName.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local shipName, errorString = Helpers.getPlayerShipParameter(saveGame, "ship/name")
		if errorString then return nil, errorString end
		return shipName
	end
}}


--
-- ship label (registration number)
--
-- value: string
--
local ShipLabel = GameParam.New(lui.REGISTRATION_NUMBER, "ship.label")
ShipLabel.value = ""
function ShipLabel:draw()
	Widgets.oneLiner(lui.REGISTRATION_NUMBER, self.layout, function()
		local txt, changed = Widgets.inputText(self.lock, self:isValid(), "##ShipLabel", self.value, function()
			return ShipObject.MakeRandomLabel()
		end)
		if changed then
			self.value = txt
			ShipModel:updateModel()
		end
	end)
end

function ShipLabel:fromStartVariant(--[[variant]])
	self.value = ShipObject.MakeRandomLabel()
	self.lock = false
end

function ShipLabel:isValid()
	-- AB-1234
	return self.value and string.match(self.value, "^%u%u%-%d%d%d%d$")
end

ShipLabel.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local shipLabel, errorString = Helpers.getPlayerShipParameter(saveGame, "body/label")
		if errorString then return nil, errorString end
		return shipLabel
	end
}}


--
-- ship fuel
--
-- value: integer 1 .. 100
--
local ShipFuel = GameParam.New(lui.FUEL, "ship.fuel")
ShipFuel.value = 100
function ShipFuel:draw()
	Widgets.oneLiner(lui.FUEL, self.layout, function()
		local value, changed = Widgets.incrementDrag(self.lock, "##drag_ship_fuel", self.value, 1, 0, 100, "%.0f%%", true)
		if changed then
			self.value = math.round(value)
		end
	end)
end

function ShipFuel:fromStartVariant(--[[variant]])
	self.value = 100
	self.lock = true
end

function ShipFuel:isValid()
	return self.value and self.value >= 0 and self.value <= 100
end

ShipFuel.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local shipFuel, errorString = Helpers.getPlayerShipParameter(saveGame, "ship/thruster_fuel")
		if errorString then return nil, errorString end
		return math.ceil(shipFuel * 100)
	end
}}


--
-- ship model
--
-- value: { pattern: number, colors: array of 3 Color }
--
ShipModel = GameParam.New(lui.MODEL, "ship.model")
ShipModel.value = {
	pattern = 1,
	colors = {}
}

ShipModel.spinner = false
ShipModel.skin = false
ShipModel.numPatterns = 1

function ShipModel:updateModel()
	local modelName = ShipDef[ShipType.value].modelName
	local model = Engine.GetModel(modelName)
	self.numPatterns = model.numPatterns
	if not self.skin then return end
	local c = self.value.colors
	self.skin:SetColors({ primary = c[1], secondary = c[2], trim = c[3] })
	self.skin:SetLabel(ShipLabel.value)
	self.spinner:setModel(modelName, self.skin, self.value.pattern)
end

function ShipModel:setSize(size)
	if self.spinner then
		self.spinner:setSize(size)
	end
end

function ShipModel:draw()
	if not self.spinner then
		self.skin = ModelSkin.New()
		self.spinner = ModelSpinner()
		self.spinner.spinning = false
		self:updateModel()
	end
	-- draw a ship!
	self:setSize(Vector2(self.layout.width, self.layout.height))
	self.spinner:draw()
end

function ShipModel:colorPicker(colorNumber)
	local changed, color = ui.colorEdit("##edit_model_color_" .. tostring(colorNumber), self.value.colors[colorNumber], { "NoAlpha", "NoInputs" })
	if changed then
		self.value.colors[colorNumber] = color
		self:updateModel()
	end
end

function ShipModel:fromStartVariant(variant)
	self.value.colors = {}
	for k,v in pairs(variant.colors) do
		self.value.colors[k] = v
	end
	self.value.pattern = variant.pattern
	self.lock = true
end

function ShipModel:isValid()
	return self.numPatterns == 0 or self.value.pattern <= self.numPatterns
end

ShipModel.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local pattern, errorString = Helpers.getPlayerShipParameter(saveGame, "model_body/model/cur_pattern_index")
		if errorString then return nil, errorString end
		local jsonColors
		jsonColors, errorString = Helpers.getPlayerShipParameter(saveGame, "ship/model_skin/colors")
		if errorString then return nil, errorString end
		if not jsonColors then return nil, "Colors are empty" end
		local colors = {}
		for i = 1, 3 do
			local color = jsonColors[i]
			if not color or #color ~= 4 then
				return nil, "Wrong skin color"
			end
			table.insert(colors, Color(table.unpack(color)))
		end
		return { pattern = pattern + 1, colors = colors }
	end
}}

--
-- ship cargo
--
-- value: map string (commodity id) -> int (amount)
--
local ShipCargo = GameParam.New(lc.CARGO, "ship.cargo")
ShipCargo.value = { plastics = 10, metal_ore = 5, hydrogen = 65, carbon_ore = 87 }

ShipCargo.textTable = {}
ShipCargo.mass = 0

ShipCargo.commodities = {}
-- array
for k,v in pairs(Commodities) do
	table.insert(ShipCargo.commodities, {id = k, label = v.lang.name})
end

table.sort(ShipCargo.commodities, function(x1, x2) return x1.label < x2.label end)

ShipCargo.comboItems = {}
ShipCargo.comboItemsIDs = {}

function ShipCargo:updateDrawItems()
	self.comboItems = { "+" }
	self.comboItemsIDs = {}
	self.textTable = {}
	self.mass = 0
	for k, v in pairs(self.value) do
		self.mass = self.mass + v
		if v == 0 then self.value[k] = nil end
	end
	for _, v in ipairs(self.commodities) do
		if not self.value[v.id] then
			table.insert(self.comboItems, v.label)
			table.insert(self.comboItemsIDs, v.id)
		else
			table.insert(self.textTable, { id = v.id, label = v.label, amount = self.value[v.id] })
		end
	end
end

ShipCargo:updateDrawItems()

function ShipCargo:countCommodity(name)
	return self.value[name] or 0
end

function ShipCargo:draw()
	if not ui.collapsingHeader(lui.CARGO, { "DefaultOpen" }) then return end

	-- Indent the table slightly
	ui.addCursorPos(Vector2(ui.getItemSpacing().x, 0))
	ui.beginTable("#cargotable", 3, { "SizingFixedFit" })
	ui.tableSetupColumn("label", { "WidthStretch" })

	for _, v in ipairs(self.textTable) do
		ui.tableNextRow()
		ui.tableNextColumn()

		ui.alignTextToFramePadding()
		ui.text(v.label)

		ui.tableNextColumn()
		ui.nextItemWidth(Defs.dragWidth)
		local value, changed = Widgets.incrementDrag(self.lock, "##drag"..v.id, self.value[v.id], 1, 1, 1000000, "%.0ft")
		if changed then
			self.value[v.id] = math.round(value)
			self:updateDrawItems()
		end

		ui.tableNextColumn()
		if not self.lock and ui.iconButton("##cargoremove" .. v.id, ui.theme.icons.cross, nil, nil, Vector2(Defs.removeWidth, Defs.removeWidth)) then
			self.value[v.id] = nil
			self:updateDrawItems()
		end
	end

	ui.endTable()

	if not self.lock then
		ui.nextItemWidth(Defs.addWidth)
		local changed, ret = ui.combo("##cargo", 0, self.comboItems)
		if changed and ret > 0 then
			-- adding 1 item
			self.value[self.comboItemsIDs[ret]] = 1
			self:updateDrawItems()
		end
	end
end

function ShipCargo:fromStartVariant(variant)
	self.value = {}
	for _, entry in pairs(variant.cargo) do
		--	{ Commodities.hydrogen, 2 } ...
		self.value[entry[1].name] = entry[2]
	end
	self:updateDrawItems()
	self.lock = true
end
--
-- to_remove: table { x1 = true, ... }
-- where x_i is some value that can be contained in the array
-- array, to_remove is not corrupted
-- the order of the elements of the resulting array corresponds to the original
--
local function removeElems(array, to_remove)
	local result = {}
	local removed = {}
	for _, v in ipairs(array) do
		if to_remove[v] and not removed[v] then
			removed[v] = true
		else
			table.insert(result, v)
		end
	end
	return result
end

function ShipCargo:isValid()
	ShipSummary:prepareAndValidateParamList()
	return ShipSummary.cargo.valid
end

ShipCargo.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local cargo, errorString = Helpers.getPlayerShipParameter(saveGame, "lua_components/CargoManager/commodities")
		if errorString then return nil, errorString end
		local result = {}
		for k, v in pairs(cargo) do
			result[k] = v.count
		end
		return result
	end
}}


--
-- ship equipment
--

---@param unitBase table where to find the equipment unit
---@param unitPath string
---@param eqTable table with IDs, can be subtable
---@param eqTableSectionPath string
---@return string? id
---@return string? errorString
local function findSavedEquipmentID(unitBase, unitPath, eqTable, eqTableSectionPath)
	local entry, errorString = Helpers.getByPath(unitBase, unitPath)
	-- missing entry is acceptable
	if errorString then return nil end
	if entry then
		local eqSection
		eqSection, errorString = Helpers.getByPath(eqTable, eqTableSectionPath)
		if errorString then return nil, errorString end
		assert(eqSection)
		for id, eq in pairs(eqSection) do
			if eq == entry then return id end
		end
	end
end

ShipEquip.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		-- a table with all possible equipment is located directly in the saveGame
		-- The ship's equipment set stores refs to this table items, and the
		-- string IDs of the equipment are only in this table
		local eqTable, errorString = Helpers.getByPath(saveGame, "lua_modules_json/Equipment")
		if errorString then return nil, errorString end
		assert(eqTable)

		local eqTableMisc
		eqTableMisc, errorString = Helpers.getByPath(eqTable, "misc")
		if errorString then return nil, errorString end
		assert(eqTableMisc)

		-- table with slots
		local eqSet
		eqSet, errorString = Helpers.getPlayerShipParameter(saveGame, "ship/equipSet/lua_ref_json/slots")
		if errorString then return nil, errorString end

		local engine, laser_front, laser_rear

		local misc = {}
		for slotName, slot in pairs(eqSet) do
			if slotName == 'engine' then
				engine, errorString = findSavedEquipmentID(slot, "#1", eqTable, "hyperspace")
			elseif slotName == 'laser_front' then
				laser_front, errorString = findSavedEquipmentID(slot, "#1", eqTable, "laser")
			elseif slotName == 'laser_rear' then
				laser_rear, errorString = findSavedEquipmentID(slot, "#1", eqTable, "laser")
			elseif #slot > 0 then
				for _, unit in ipairs(slot) do
					local entry
					entry, errorString = findSavedEquipmentID(unit, "", eqTableMisc, "")
					if entry then
						addMiscEntry(misc, entry)
					end
				end
			end
			if errorString then return nil, errorString end
		end

		return {
			engine = engine,
			laser_front = laser_front,
			laser_rear = laser_rear,
			misc = misc
		}
	end
}}


--
-- ship summary - not a param, depends both on the model of the ship and on its cargo/eq
--
-- also checks values
--
ShipSummary = {}
ShipSummary.noName = true
ShipSummary.cargo = { valid = true }
ShipSummary.equip = { valid = true }

local function greater(x, y)
	return x > y
end

local function rowWithAlert(output, label, current, project, alertIf, unitStr)
	if current == nil then current = 0 end
	if project == nil then project = 0 end
	if unitStr == nil then unitStr = '' end
	local color
	local valid = not alertIf(current, project)
	if not valid then color = ui.theme.colors.alertRed end
	output.valid = output.valid and valid
	return { label .. ":", string.format("%d / %d" .. unitStr .. " ", current, project), color = color }
end

-- prepare a table for output and at the same time check the parameters
function ShipSummary:prepareAndValidateParamList()
	local def = ShipDef[ShipType.value]
	local usedSlots = ShipEquip.usedSlots
	local freeCargo = def.cargo
	self.cargo.valid = true
	self.equip.valid = true
	local eq_n_cargo = { valid = true }
	freeCargo = math.max(freeCargo, 0)
	local paramList = {
		rowWithAlert(eq_n_cargo, lui.CAPACITY, ShipCargo.mass + ShipEquip.mass, def.equipCapacity, greater, 't'),
		rowWithAlert(self.cargo, lui.CARGO_SPACE, ShipCargo.mass, freeCargo, greater, 't'),
		--rowWithAlert(self.equip, lui.FRONT_WEAPON, usedSlots.laser_front, def.equipSlotCapacity.laser_front, greater),
		--rowWithAlert(self.equip, lui.REAR_WEAPON, usedSlots.laser_rear, def.equipSlotCapacity.laser_rear, greater),
		--rowWithAlert(self.equip, lui.CABINS, usedSlots.cabin, def.equipSlotCapacity.cabin, greater),
		rowWithAlert(self.equip, lui.CREW_CABINS, #Crew.value + 1, def.maxCrew, greater),
		--rowWithAlert(self.equip, lui.MISSILE_MOUNTS, usedSlots.missile, def.equipSlotCapacity.missile, greater),
		--rowWithAlert(self.equip, lui.SCOOP_MOUNTS, usedSlots.scoop, def.equipSlotCapacity.scoop, greater),
	}
	self.cargo.valid = self.cargo.valid and eq_n_cargo.valid
	self.equip.valid = self.equip.valid and eq_n_cargo.valid
	return paramList
end

local function getFuelUse(hyperclass, range, range_max)
	local distance = range or range_max
	local hyperclass_squared = hyperclass^2
	return math.clamp(math.ceil(hyperclass_squared*distance / range_max), 1, hyperclass_squared);
end

function ShipSummary:draw()

	local def = ShipDef[ShipType.value]

	local paramList = self:prepareAndValidateParamList()

	ui.child("param_table1", Vector2(self.layout.width, 0), function()
		textTable.drawTable(2, { self.layout.width - self.valueWidth, self.valueWidth }, paramList)
	end)

	ui.sameLine()

	local fuel_mass = def.fuelTankMass * ShipFuel.value * 0.01 -- ShipFuel.value in %
	local mass_with_fuel = def.hullMass + ShipCargo.mass + ShipEquip.mass + fuel_mass

	-- adapted from ShipType
	local power_mul = 1.0 + ShipEquip:getThrusterUpgradeLevel() * 0.1

	local fwd_cap = def.linAccelerationCap.FORWARD
	local fwd_acc = math.min(def.linearThrust.FORWARD * power_mul / mass_with_fuel / 1000, fwd_cap)
	local up_cap = def.linAccelerationCap.UP
	local up_acc = math.min(def.linearThrust.UP * power_mul / mass_with_fuel / 1000, up_cap)
	local hyperclass = ShipEquip:getHyperDriveClass()

	local remaining_fuel = ShipCargo:countCommodity('hydrogen')
	-- adapted from HyperDriveType
	local range_max = 625.0 * (hyperclass^2) / (mass_with_fuel)
	local hyper_range = hyperclass ~= 0 and math.min(range_max, remaining_fuel * range_max / hyperclass^2) or 0
	local delta_v = def.effectiveExhaustVelocity * math.log(mass_with_fuel / (mass_with_fuel - fuel_mass));
	local p_delta_v = def.effectiveExhaustVelocity * math.log(mass_with_fuel / (mass_with_fuel - (fuel_mass + remaining_fuel)));

	local fuel_max = getFuelUse(hyperclass, range_max, range_max)
	local range
	if fuel_max <= remaining_fuel then
		range = range_max
	else
		range = range_max*remaining_fuel/fuel_max

		while range > 0 and getFuelUse(hyperclass, range, range_max) > remaining_fuel do
			range = range - 0.05
		end
	end

	ui.child("##param_table2", Vector2(self.layout.width, 0), function()
		textTable.drawTable(2, { self.layout.width - self.valueWidth, self.valueWidth }, {
		{ lui.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
		{ lui.HYPERSPACE_RANGE..":", string.format("%.1f / %.1f " .. lui.LY, hyper_range, range_max) },
		{ lui.DELTA_V..":",      string.format("%s %s", ui.Format.SpeedUnit(delta_v)) },
		{ lui.REFUEL .. " " .. lui.DELTA_V..":", string.format("%s %s", ui.Format.SpeedUnit(p_delta_v)) },
		{ lui.FORWARD_ACCEL..":",  string.format("%.1f / %.1f g", fwd_acc / 9.81, fwd_cap / 9.81) },
		{ lui.UP_ACCEL..":",       string.format("%.1f / %.1f g", up_acc / 9.81, up_cap / 9.81) },
		})
	end)
	ui.sameLine(0, 0)
end


--
-- draw tab
--
local function draw()
	ui.child("ship_and_params", Vector2(layout.leftWidth, Defs.contentRegion.y), function()

		ShipType:draw() ui.sameLine() ShipLabel:draw()
		ShipName:draw() ui.sameLine() ShipFuel:draw()

		local top = ui.getCursorPos()
		ShipModel:draw()
		local pos = ui.getCursorPos()

		if ShipModel.numPatterns > 0 and not ShipModel.lock then
			ui.setCursorPos(top + Vector2(Defs.gap.x, Defs.gap.y))
			ui.nextItemWidth(layout.patternDragWidth)
			local value, changed = ui.incrementDrag("##ship_model_drag_pattern", ShipModel.value.pattern, 0.1, 1, ShipModel.numPatterns, lui.PATTERN .. " %.0f/" .. tostring(ShipModel.numPatterns))
			if changed then
				ShipModel.value.pattern = value
				ShipModel:updateModel()
			end

			ui.setCursorPos(pos + Vector2(Defs.gap.x, - Defs.gap.y * 2 - Defs.buttonSize))
			if ui.iconButton("randomize_colors", ui.theme.icons.random, nil, nil, Vector2(Defs.buttonSize)) then
				ShipModel.skin:SetRandomColors(Defs.rand)
				ShipModel.value.colors = ShipModel.skin:GetColors()
				ShipModel:updateModel()
			end
			ui.sameLine()
			ShipModel:colorPicker(1)
			ui.sameLine()
			ShipModel:colorPicker(2)
			ui.sameLine()
			ShipModel:colorPicker(3)
			ui.setCursorPos(pos)
		end

		ShipSummary:draw()
	end)
	ui.sameLine()
	ui.child("cargo_and_equip", Vector2(layout.rightWidth, Defs.contentRegion.y), function()
		ShipCargo:draw()
		ui.text("")
		ShipEquip:draw()
	end)
end

local function updateLayout()
	layout.sectionWidth = math.round(Defs.goodWidth / 5)
	layout.leftWidth = math.round(0.6 * Defs.contentRegion.x)
	layout.rightWidth = Defs.contentRegion.x - layout.leftWidth - Defs.gap.x
	layout.patternDragWidth = ui.calcTextSize("<-- " .. lui.PATTERN .. " 00/00 -->").x

	local paramWidth = (layout.leftWidth - Defs.gap.x) * 0.5

	-- link layouts to align the width of labels
	ShipType.layout = { width = paramWidth }
	ShipName.layout = ShipType.layout

	-- do not link layouts because the label lengths are too different
	ShipFuel.layout = { width = paramWidth }
	ShipLabel.layout = { width = paramWidth }

	ShipModel.layout = {
		width = layout.leftWidth,
		height = Defs.contentRegion.y * 0.54 -- just picked it up to fit everything
	}
	ShipSummary.layout = { width = paramWidth }
	ShipSummary.valueWidth = ui.calcTextSize("-3000t / 3000t-").x -- to update
end

local function updateParams()
	ShipType:setShipID(ShipType.value)
	ShipModel:updateModel()
	ShipCargo:updateDrawItems()
	ShipEquip:update()
end

return {
	TabName = lui.SHIP,
	draw = draw,
	updateLayout = updateLayout,
	updateParams = updateParams,
	Type = ShipType,
	Name = ShipName,
	Label = ShipLabel,
	Cargo = ShipCargo,
	Equip = ShipEquip,
	Model = ShipModel,
	Fuel = ShipFuel
}
