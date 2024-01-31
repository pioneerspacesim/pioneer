-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
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
			self.value = ShipNames.generateRandom()
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
			self.value = ShipObject.MakeRandomLabel()
		end)
		if changed then self.value = txt end
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
	local allWidth = layout.rightWidth - Defs.scrollWidth
	local spacing = Defs.gap.x
	ui.columns(3, "#cargotable")
	ui.setColumnWidth(0, allWidth - Defs.dragWidth - Defs.removeWidth - spacing * 2)
	ui.setColumnWidth(1, Defs.dragWidth + spacing)
	ui.setColumnWidth(2, Defs.removeWidth + spacing)
	for _, v in ipairs(self.textTable) do
		ui.alignTextToFramePadding()
		ui.text(v.label)
		ui.nextColumn()
		ui.nextItemWidth(Defs.dragWidth)
		local value, changed = Widgets.incrementDrag(self.lock, "##drag"..v.id, self.value[v.id], 1, 1, 1000000, "%.0ft")
		if changed then
			self.value[v.id] = math.round(value)
			self:updateDrawItems()
		end
		ui.nextColumn()
		if not self.lock and ui.iconButton(ui.theme.icons.retrograde, Vector2(Defs.removeWidth, Defs.removeWidth), "##cargoremove" .. v.id) then
			self.value[v.id] = nil
			self:updateDrawItems()
		end
		ui.nextColumn()
	end

	ui.columns(1)

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
-- value: see below
--
local ShipEquip = GameParam.New(lui.EQUIPMENT, "ship.equipment")

-- by default, only _one_ unit can be put on a slot with a given id, i.e. 'ecm', 'atmo_shield'
-- often one EquipmentType has a slot of the same name for itself, i.e hull_autorepair has slot hull_autorepair

-- special slot classes:

-- it is possible to have several _different_ units on this slot
local sharedSlot = { scoop = true }
-- it is possible to have several different or even the _same_ units on this slot
local multiSlot = { missile = true, cabin = true }

-- also very special slots, they have separate lists in Equipment - 'hyperspace' and 'laser'
-- sections of the same name are created for them in self.value
local hardSlots = { 'engine', 'laser_front', 'laser_rear' }

-- also hide these IDs, they are a strange implementation detail
local hiddenIDs = { cabin_occupied = true }

-- all the equipment, sorted
ShipEquip.lists = {
	hyperspace = {},
	laser = {},
	misc = {}
}
-- fill and sort equipment lists
for slot, tbl in pairs(ShipEquip.lists) do
	for k, _ in pairs(Equipment[slot]) do
		if not hiddenIDs[k] then
			table.insert(tbl, k)
		end
	end
end

for _, v in pairs(ShipEquip.lists) do
	table.sort(v)
end

-- we have 4 sections, the first 3 correspond to "special hard slots", and the fourth 'misc' - to all the others -
-- They are listed in the equipment list of the same name 'misc'
ShipEquip.sections = {
	engine =      { list = ShipEquip.lists.hyperspace, label = leq.PROPULSION,   },
	laser_front = { list = ShipEquip.lists.laser,      label = lui.FRONT_WEAPON, },
	laser_rear =  { list = ShipEquip.lists.laser,      label = lui.REAR_WEAPON,  },
	misc =        { list = ShipEquip.lists.misc,       label = lc.MISCELLANEOUS, }
}

ShipEquip.combos = {
	engine =      { l7d_list = nil, selected = 0 },
	laser_front = { l7d_list = nil, selected = 0 },
	laser_rear =  { l7d_list = nil, selected = 0 },
	misc =        { l7d_list = nil, list = nil }
}

-- values is the names of the equipment in the equipment table
-- 'misc' is an array, because we draw this list on the screen
ShipEquip.value = {
	engine = nil,
	laser_front = nil,
	laser_rear = nil,
	misc = {} -- array of struct { id: equipment_id (string), amount: int }
}

-- utils

local function findEquipmentType(eqTypeID)
	for _, eq_list in pairs({ 'misc', 'laser', 'hyperspace' }) do
		if Equipment[eq_list][eqTypeID] then
			return Equipment[eq_list][eqTypeID]
		end
	end
	assert(false, "Wrong Equipment ID: " .. tostring(eqTypeID))
end

local function findEquipmentPath(eqKey)
	for _, eq_list in pairs({ 'misc', 'laser', 'hyperspace' }) do
		for id, obj in pairs(Equipment[eq_list]) do
			if obj.l10n_key == eqKey then
				return eq_list, id
			end
		end
	end
	assert(false, "Wrong Equipment ID: " .. tostring(eqKey))
end

local function hasSlotClass(eqTypeID, slotClass)
	local eqType = findEquipmentType(eqTypeID)
	for _, slot in pairs(eqType.slots) do
		if slotClass[slot] then return true end
	end
	return false
end

-- if the slot is already occupied, we return the id of the equipment with which it is occupied
local function checkIfSlotAlreadyOccupied(eqTypeID, list)
	local eqType = findEquipmentType(eqTypeID)
	-- misc always has one slot
	local slot = eqType.slots[1]
	for _, entry in pairs(list) do
		local checkEqType = findEquipmentType(entry.id)
		if checkEqType.slots[1] == slot then return entry.id end
	end
	return nil
end

function ShipEquip:getHyperDriveClass()
	if not self.value.engine then return 0 end
	local drive = Equipment.hyperspace[self.value.engine]
	return drive.capabilities.hyperclass
end

function ShipEquip:getThrusterUpgradeLevel()
	for _, eq_entry in pairs(self.value.misc) do
		local eq = Equipment.misc[eq_entry.id]
		if eq.capabilities.thruster_power then
			return eq.capabilities.thruster_power
		end
	end
	return 0
end

function ShipEquip:setDefaultHyperdrive()
	local drive_class = ShipDef[ShipType.value].hyperdriveClass
	if drive_class == 0 then
		self.value.engine = nil
	else
		local driveID = "hyperdrive_" .. drive_class
		local index = utils.indexOf(self.lists.hyperspace, driveID)
		assert(index, "unknown drive ID: " .. tostring(driveID))
		self.value.engine = driveID
	end
	self:update()
end

function ShipEquip:removeHyperdrive()
	self.value.engine = nil
end

-- adding 1 item, without breaking the alphabetical order
local function addMiscEntry(miscTable, newID)
	for i, entry in ipairs(miscTable) do
		if newID < entry.id then
			table.insert(miscTable, i, { id = newID, amount = 1 })
			return
		elseif newID == entry.id then
			entry.amount = entry.amount + 1
			return
		end
	end
	table.insert(miscTable, { id = newID, amount = 1 })
end

local function addToTable(tbl, key, count)
	if tbl[key] then
		tbl[key] = tbl[key] + count
	else
		tbl[key] = count
	end
end

--
-- for summary e.t.c.
-- array of struct: { eq: EquipmentType, amount: int }
--
-- a predictable display order of the list of installed equipment is required
-- content is requested every frame, generated only when the equipment changes
--
ShipEquip.summaryList = {}
function ShipEquip:addToSummary(key, count)
	local eqlist = self.summaryList
	for k, v in pairs(eqlist) do
		if v.obj == key then
			eqlist[k].count = eqlist[k].count + count
			return
		end
	end
	table.insert(eqlist, { obj = key, count = count })
end

-- generate static localized equipment lists for 'hard slot' combos
for _, section_id in pairs(hardSlots) do
	local combo = ShipEquip.combos[section_id]
	local section = ShipEquip.sections[section_id]
	combo.l7d_list = { lui.NO }
	for _, id in ipairs(section.list) do
		table.insert(combo.l7d_list, leq[findEquipmentType(id).l10n_key])
	end
end

-- bring everything into a consistent state based on the (most likely) updated value
function ShipEquip:update()

	-- update combos
	for _, section in ipairs(hardSlots) do
		local eqSection = self.sections[section]
		local eqID = self.value[section]
		self.combos[section].selected = eqID and utils.indexOf(eqSection.list, eqID) or 0
	end
	local misc_combo = self.combos.misc
	local selectedIDs = {}
	for _, entry in ipairs(self.value.misc) do
		selectedIDs[entry.id] = true
	end
	-- in a misc_combo, only those elements that are not in the list
	misc_combo.list = removeElems(self.lists.misc, selectedIDs)
	misc_combo.selected = 0
	misc_combo.l7d_list = {"+"}
	for _, id in ipairs(misc_combo.list) do
		table.insert(misc_combo.l7d_list, leq[findEquipmentType(id).l10n_key])
	end

	-- update stats and summary
	self.usedSlots = {} -- many slots do not allow more than one unit
	self.mass = 0 -- equipment mass
	self.summaryList = {}

	for _, section_id in pairs(hardSlots) do
		local eqID = self.value[section_id]
		if eqID then
			local eqType = findEquipmentType(eqID)
			local slot = section_id
			addToTable(self.usedSlots, slot, 1)
			self:addToSummary(eqType, 1)
			self.mass = self.mass + eqType.capabilities.mass
		end
	end

	for _, entry in ipairs(self.value.misc) do
		local eqType = findEquipmentType(entry.id)
		local slot = eqType.slots[1] -- misc always has one slot
		addToTable(self.usedSlots, slot, entry.amount)
		self:addToSummary(eqType, entry.amount)
		self.mass = self.mass + eqType.capabilities.mass * entry.amount
	end
end

ShipEquip:update()

function ShipEquip:draw()
	if not ui.collapsingHeader(lui.EQUIPMENT, { "DefaultOpen" }) then return end

	-- hard slots

	local allWidth = layout.rightWidth - Defs.scrollWidth
	local spacing = Defs.gap.x

	local combosWidth = allWidth - layout.sectionWidth - Defs.eqTonnesWidth - spacing * 3
	ui.columns(3, "#equip-oneliners")
	ui.setColumnWidth(0, layout.sectionWidth + spacing)
	ui.setColumnWidth(1, combosWidth + spacing)
	ui.setColumnWidth(2, Defs.eqTonnesWidth + spacing)
	for _, section_id in ipairs(hardSlots) do
		local section = self.sections[section_id]
		local combo = self.combos[section_id]
		ui.alignTextToFramePadding()
		ui.text(section.label)
		layout.sectionWidth = math.max(ui.calcTextSize(section.label).x, layout.sectionWidth)
		ui.nextColumn()
		ui.nextItemWidth(combosWidth)
		local changed, ret = Widgets.combo(self.lock, "##equip_hardslot_" .. section_id, combo.selected, combo.l7d_list)
		if changed then
			self.value[section_id] = section.list[ret]
			self:update()
		end
		ui.nextColumn()
		if combo.selected ~= 0 then
			local eqID = section.list[combo.selected]
			local mass = findEquipmentType(eqID).capabilities.mass
			ui.alignTextToFramePadding()
			ui.text(tostring(mass)..'t')
		end
		ui.nextColumn()
	end
	ui.columns(1)

	ui.dummy(ui.getItemSpacing())

	-- misc slots

	local section = self.sections.misc
	ui.text(section.label .. ":")

	ui.columns(4, "#misc_equip_table")
	ui.setColumnWidth(0, allWidth - Defs.dragWidth - Defs.removeWidth - Defs.eqTonnesWidth - spacing * 3)
	ui.setColumnWidth(1, Defs.dragWidth + spacing)
	ui.setColumnWidth(2, Defs.eqTonnesWidth + spacing)
	ui.setColumnWidth(3, Defs.removeWidth + spacing)
	local count = 0

	for _, v in ipairs(self.value.misc) do
		count = count + 1
		local eqType = findEquipmentType(v.id)
		ui.alignTextToFramePadding()
		ui.text(leq[eqType.l10n_key])
		ui.nextColumn()
		if hasSlotClass(v.id, multiSlot) then
			ui.nextItemWidth(Defs.dragWidth)
			local value, changed = Widgets.incrementDrag(self.lock, "##eqdrag"..v.id, v.amount, 1, 1, 1000000, "x %.0f")
			if changed then
				v.amount = math.round(value)
				self:update()
			end
		end
		ui.nextColumn()
		ui.alignTextToFramePadding()
		ui.text(tostring(eqType.capabilities.mass * v.amount)..'t')
		ui.nextColumn()
		if not self.lock and ui.iconButton(ui.theme.icons.retrograde, Vector2(Defs.removeWidth, Defs.removeWidth), "##eqremove" .. v.id) then
			table.remove(self.value.misc, count)
			self:update()
		end
		ui.nextColumn()
	end

	ui.columns(1)

	if not self.lock then
		local combo = self.combos.misc
		ui.nextItemWidth(Defs.addWidth)
		local changed, ret = ui.combo("##addequip", 0, combo.l7d_list)
		if changed and ret > 0 then
			local newID = combo.list[ret]
			local occupied
			if not (hasSlotClass(newID, multiSlot) or hasSlotClass(newID, sharedSlot)) then
				occupied = checkIfSlotAlreadyOccupied(newID, self.value.misc)
				if occupied then
					local popup = Widgets.ConfirmPopup()
					popup.drawQuestion = function()
						ui.text("Replace " .. leq[findEquipmentType(occupied).l10n_key])
						ui.text("with " .. leq[findEquipmentType(newID).l10n_key] .. "?")
					end
					popup.yesAction = function()
						for i, entry in ipairs(self.value.misc) do
							if entry.id == occupied then
								self.value.misc[i] = { id = newID, amount = 1 }
								ShipEquip:update()
								break;
							end
						end
					end
					popup:open()
				end
			end
			if not occupied then
				addMiscEntry(self.value.misc, newID)
				self:update()
			end
		end
	end
end

function ShipEquip:fromStartVariant(variant)
	if variant.hyperdrive then
		self:setDefaultHyperdrive()
	else
		self:removeHyperdrive()
	end

	local eq_value = self.value
	eq_value.laser_front = nil
	eq_value.laser_rear = nil
	eq_value.misc = {}
	for _, entry in pairs(variant.equipment) do
		local eq, amount = table.unpack(entry)
		local eq_list, id = findEquipmentPath(eq.l10n_key)
		if eq_list == 'misc' then
			addMiscEntry(eq_value.misc, id)
		elseif eq_list == 'laser' then
			for _ = 1, amount do
				if not eq_value.laser_front then
					eq_value.laser_front = id
				elseif not eq_value.laser_rear then
					eq_value.laser_rear = id
				else
					assert(false, "Too many lasers in start variant!")
				end
			end
		else
			assert(false, "unacceptable eq_list: " .. tostring(eq_list))
		end
	end
	self:update()
	self.lock = true
end

function ShipEquip:isValid()
	ShipSummary:prepareAndValidateParamList()
	return ShipSummary.equip.valid
end

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
	local freeCargo = math.min(def.equipSlotCapacity.cargo, def.capacity - ShipEquip.mass)
	self.cargo.valid = true
	self.equip.valid = true
	local eq_n_cargo = { valid = true }
	freeCargo = math.max(freeCargo, 0)
	local paramList = {
		rowWithAlert(eq_n_cargo, lui.CAPACITY, ShipCargo.mass + ShipEquip.mass, def.capacity, greater, 't'),
		rowWithAlert(self.cargo, lui.CARGO_SPACE, ShipCargo.mass, freeCargo, greater, 't'),
		rowWithAlert(self.equip, lui.FRONT_WEAPON, usedSlots.laser_front, def.equipSlotCapacity.laser_front, greater),
		rowWithAlert(self.equip, lui.REAR_WEAPON, usedSlots.laser_rear, def.equipSlotCapacity.laser_rear, greater),
		rowWithAlert(self.equip, lui.CABINS, usedSlots.cabin, def.equipSlotCapacity.cabin, greater),
		rowWithAlert(self.equip, lui.CREW_CABINS, #Crew.value + 1, def.maxCrew, greater),
		rowWithAlert(self.equip, lui.MISSILE_MOUNTS, usedSlots.missile, def.equipSlotCapacity.missile, greater),
		rowWithAlert(self.equip, lui.SCOOP_MOUNTS, usedSlots.scoop, def.equipSlotCapacity.scoop, greater),
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
			if ui.iconButton(ui.theme.icons.random, Vector2(Defs.buttonSize), "##ship_model_random_button") then
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
