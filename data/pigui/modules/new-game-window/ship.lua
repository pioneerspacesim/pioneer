-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Lang = require 'Lang'
local leq = Lang.GetResource("equipment-core")
local lc = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local msgbox = require 'pigui.libs.message-box'
local utils = require 'utils'
local ShipDef = require 'ShipDef'
local ShipObject = require 'Ship'
local HullConfig = require 'HullConfig'
local ModalWindow = require 'pigui.libs.modal-win'
local ModelSpinner = require 'PiGui.Modules.ModelSpinner'
local ModelSkin = require 'SceneGraph.ModelSkin'
local Commodities = require 'Commodities'
local Equipment = require 'Equipment'
local EquipSet = require 'EquipSet'
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
			ShipModel.value.pattern = 1
			self:setShipID(self.shipIDs[ret + 1])
		end
	end)
end

function ShipType:setShipID(shipID)
	local index = utils.indexOf(ShipType.shipIDs, shipID)
	assert(index, "unknown ship ID: " .. tostring(shipID))
	self.value = shipID
	self.selected = index - 1
	self.updated()
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
	Widgets.filledHeader(lui.CARGO, ShipType.layout.width)

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
local ShipEquip = GameParam.New(lui.EQUIPMENT, "ship.equipment")

-- tree
--    string (slot id) -> string (installed equipment id),
-- or string (slot id) -> { id: string (installed equipment id), slots: <sub-tree> }
-- id can be nil, if slot is empty, but there are equipment on sub slots
-- slotless equipment is at the top level, as an array
ShipEquip.value = {}

-- tree
-- string (slot id) -> { object: HullConfig.Slot, orphaned: bool, installed: equipID, children: <sub-tree> }
--
-- ship slots merged with slots from value
-- does not contain slotless equipment
ShipEquip.slots = {}

-- array, actually a list of the ship's equipment, ready to be displayed
-- item: {
--     slotName: string (pretty slot name)
--     itemName: string (pretty equip name)
--     itemID: string (equipment id)
--     mass: number
--     volume: number
--     errorString: string, if not nil, then the equipment is installed incorrectly
--     path: array of string, a chain of names of slots and subslots, starting
--           with the parent on which the equipment is located, widely used.
--           has special values: {} - root for slotless equipment
--                               { <number> } - slotless equipment
-- }
ShipEquip.viewData = {}

-- centrally store pretty names for equipment
-- table, string (id) -> string (localized name)
ShipEquip.equipNames = {}

ShipEquip.editDialog = {}

-- summary info
ShipEquip.summaryList = {}
ShipEquip.mass = 0
ShipEquip.volume = 0
ShipEquip.hyperDriveClass = 0
ShipEquip.thrusterUpgradeLevel = 0

ShipEquip.indent = '   '
ShipEquip.warningSign = '⚠'
-- TODO: localize "S" symbol?
ShipEquip.sizeLetter = 'S'

function ShipEquip:removeItem(path)

	local value = self.value
	for i = 1, #path - 1, 1 do
		value = value[path[i]].slots
	end

	local key = path[#path]

	if type(key) == 'number' then
		table.remove(value, key)
	else
		value[key] = nil
	end
end

local function isEquipmentSlotless(id)
	local eqObject = Equipment.Get(id)
	if not eqObject or eqObject.slot then return false end
	return true
end

local function isPathSlotless(path)
	if not path then return false end
	if #path ~= 1 then return false end
	return type(path[1]) == 'number'
end

function ShipEquip.pathIsEqual(path1, path2)

	if not path1 then return not path2 end
	if not path2 then return false end

	if #path1 ~= #path2 then return false end
	for i = 1, #path1, 1 do
		if path1[i] ~= path2[i] then return false end
	end
	return true
end

-- put node to value
-- create passing nodes if necessary
local function putNode(value, node, path)

	-- slotless
	if #path == 0 then
		table.insert(value, node)
		return
	end

	for i = 1, #path - 1, 1 do
		local slotID = path[i]
		if not value[slotID] then
			value[slotID] = { slots = {} }
		elseif type(value[slotID]) == 'string' then
			value[slotID] = { id = value[slotID], slots = {} }
		end
		value = value[slotID].slots
	end

	local slotID = path[#path]

	value[slotID] = node
end

local function safeFormat(value, format)
	if value then
		if format then return format(value) end
		return value
	end
	return '-'
end

local selectedRow = 0

function ShipEquip:draw()

	local h = Widgets.filledHeader(lui.EQUIPMENT, self.layout.width)
	local headerEnd = ui.getCursorPos()

	local buttonSize = Vector2(h - Defs.gap.y * 2, h - Defs.gap.y * 2)
	local buttonColors = ui.theme.buttonColors.transparent

	ui.setCursorPos(headerEnd + Vector2(self.layout.width - buttonSize.x - Defs.gap.x, Defs.gap.y * 0.5 - h))

	local selected = self.viewData[selectedRow]

	if not self.lock and ui.iconButton("##equip_edit", ui.theme.icons.pencil, nil, buttonColors, buttonSize) then
		ShipEquip:openEditDialog(selected and selected.path)
	end

	ui.setCursorPos(headerEnd)

	-- Indent the table slightly
	ui.addCursorPos(Vector2(Defs.gap.x, 0))

	ui.withStyleVars({ CellPadding = Vector2(Defs.gap.x * 2, Defs.gap.y * 0.5) }, function()

		ui.beginTable("equip_table", 4, { "SizingFixedFit", "ScrollY" }, Vector2(0, self.layout.height - h))
		ui.tableSetupScrollFreeze(0, 1)
		ui.tableSetupColumn(lc.NAME_OBJECT, { "WidthStretch" })
		ui.tableSetupColumn(leq.SLOT)
		ui.tableSetupColumn(leq.STAT_VOLUME)
		ui.tableSetupColumn(lc.MASS)
		ui.tableHeadersRow()

		for i, v in ipairs(self.viewData) do
			ui.tableNextRow()
			ui.tableNextColumn()

			if v.errorString then ui.raw.PushStyleColor("Text", ui.theme.colors.alertRed) end

			local indent = Defs.gap.x * 2
			local depth = #v.path - 1

			if depth > 0 then
				ui.addCursorPos(Vector2(depth * indent, 0))
			end
			if (ui.selectable(v.itemName .. "##" .. tostring(i), i == selectedRow , { "SpanAllColumns" })) then
				selectedRow = i
			end

			if v.errorString and ui.isItemHovered() then
				ui.withStyleColors({ Text = ui.theme.colors.font }, function()
					ui.setTooltip(v.errorString)
				end)
			end

			ui.tableNextColumn()
			if depth > 0 then
				ui.addCursorPos(Vector2(depth * indent, 0))
			end
			ui.text(v.slotName)

			ui.tableNextColumn()
			ui.text(safeFormat(v.volume, ui.Format.Volume))

			ui.tableNextColumn()
			-- add some spaces, because imgui does not do external padding unless you draw borders
			ui.text(safeFormat(v.mass, function(x) return ui.Format.Mass(x * 1000, 0) end) .. "  ")

			if v.errorString then ui.raw.PopStyleColor(1) end
		end
		ui.endTable()
	end)
end

function ShipEquip:valueNodeByPath(path)

	-- slotless root is not node
	if #path == 0 then return nil end

	local node = ShipEquip.value

	for _, slot in ipairs(path) do
		if not node then return nil end
		if node.slots then node = node.slots end
		node = node[slot]
	end

	return node
end

function ShipEquip:itemIdFromValueNode(node)
	if not node then return nil end
	if type(node) == 'string' then return node end
	if type(node) == 'table' then return node.id end
	return nil
end

function ShipEquip:slotNodeByPath(path)

	local i = 1
	local node = ShipEquip.slots

	while i <= #path do
		local slot = path[i]

		if not node then return nil end

		if node.children then node = node.children end

		node = node[slot]

		i = i + 1
	end
	return node
end

function ShipEquip:slotObjectByPath(path)
	local node = self:slotNodeByPath(path)
	return node and node.object
end

function ShipEquip:canPutItemOnSlot(eqID, path)

	if not eqID or not path then return false end

	if #path == 0 then return isEquipmentSlotless(eqID) end

	local slotObject = self:slotObjectByPath(path)
	if not slotObject then return false end

	local eqObject = Equipment.Get(eqID)
	if not eqObject then return false end

	return EquipSet.CompatibleWithSlot(eqObject, slotObject)
end

function ShipEquip:viewDataForItem(itemID, path)

	local slotName = self:getPrettySlotName(path)
	local errorString

	local warn = ''
	local slot = self:slotNodeByPath(path)
	if slot and slot.orphaned then
		warn = ' ' .. self.warningSign
		errorString = leq.INVALID_SLOT
	end

	if not errorString and slot and slot.object and slot.object.required and not ShipEquip:itemByPath(path) then
		errorString = leq.REQUIRED_SLOT_IS_EMPTY
	end

	if not errorString and not itemID then
		errorString = leq.EMPTY_EQUIPMENT_PROVIDING_SLOTS
	end

	if not errorString and not self:canPutItemOnSlot(itemID, path) and not isEquipmentSlotless(itemID) and not isPathSlotless(path) then
		errorString = leq.EQUIPMENT_DOES_NOT_FIT_INTO_THE_SLOT
	end

	-- top-level slotless equipment
	if type(slotName) == 'number' and #path == 1 then slotName = '' end

	local p = self:getEquipParams(itemID)

	return {
		slotName = slotName .. (warn or ''),
		itemName = p.name, mass = p.mass, volume = p.volume, path = path, itemID = itemID,
		errorString = errorString
	}
end

-- slot can be number or string
-- strings go before the numbers so that slotless things come at the end
local function slotLess(x1, x2)
	if type(x1) ~= type(x2) then return type(x1) == 'string'
	else return x1 < x2
	end
end

function ShipEquip.itemLess(x1, x2)
	return slotLess(x1.path[#x1.path], x2.path[#x2.path])
end

function ShipEquip:itemByPath(path)

	local node = self:valueNodeByPath(path)

	if not node then return nil end
	if type(node) == 'table' then return node.id end
	return node
end

function ShipEquip.removeNotProvidedSlots(itemID, slots)

	local obj = Equipment.Get(itemID)

	if not obj then return end

	if not obj.provides_slots then
		for k, _ in pairs(slots) do
			slots[k] = nil
		end
		return
	end

	local providedSlots = {}
	for _, slot in pairs(obj.provides_slots) do
		providedSlots[slot.id] = true
	end

	for k, _ in pairs(slots) do
		if not providedSlots[k] then
			slots[k] = nil
		end
	end
end

function ShipEquip:isValid()
	return self.valid and ShipSummary.equip.valid
end

ShipEquip.slotPrettyNamesCache = {}
function ShipEquip:tryCachedPrettySlotName(slotID)

	if not slotID then return nil end

	local cached = self.slotPrettyNamesCache[slotID]
	if cached then
		return cached
	else
		return slotID
	end
end

function ShipEquip:cachePrettySlotName(slotID, name)

	assert(slotID)

	self.slotPrettyNamesCache[slotID] = name
	return name
end

function ShipEquip:getPrettySlotName(path)

	assert(path)

	local slotID = path[#path]

	local slot = self:slotNodeByPath(path)
	if not slot then return self:tryCachedPrettySlotName(slotID) end

	assert(slot.prettyName)

	return slot.prettyName
end

function ShipEquip:createPrettySlotName(slotID, slot)

	assert(slotID)
	if not slot then return self:tryCachedPrettySlotName(slotID) end

	-- will not show S1
	local size = ''
	if slot.size and slot.size > 1 then
		size = ' ' .. self.sizeLetter .. tostring(slot.size)
	end

	if slot.i18n_key then
		return self:cachePrettySlotName(slot.id, Lang.GetResource(slot.i18n_res)[slot.i18n_key] .. size)
	end

	local base_type = slot.type:match("([%w_-]+)%.?")
	local i18n_key = (slot.hardpoint and "HARDPOINT_" or "SLOT_") .. base_type:upper()
	return self:cachePrettySlotName(slot.id, leq[i18n_key] .. size)
end

function ShipEquip:getPrettyItemName(itemID)
	return self.equipNames[itemID] or itemID or lc.UNKNOWN
end

function ShipEquip:getEquipParams(itemID)

	if not itemID then return { name = leq.EMPTY_SLOT } end

	local itemName = self:getPrettyItemName(itemID)

	local obj = Equipment.Get(itemID)
	if not obj then return { name = itemName } end

	return { name = itemName, mass = obj.mass or 0.0, volume = obj.volume or 0.0 }
end

-- return path or nil
function ShipEquip:findSlotForEquipment(eqID, dir)

	local slots
	if not dir then
		if isEquipmentSlotless(eqID) then return {} end
		dir = {}
		slots = self.slots
	else
		slots = self:slotNodeByPath(dir).children
	end

	local subNodes = {}

	for slot, node in pairs(slots) do
		local path = table.copy(dir)
		table.insert(path, slot)
		if self:canPutItemOnSlot(eqID, path) and not self:itemByPath(path) then
			return path
		end
		if node.children then
			local item = self:itemByPath(path)
			if item and self:canPutItemOnSlot(item, path) then
				table.insert(subNodes, path)
			end
		end
	end
	for _, path in ipairs(subNodes) do
		local goodPath = self:findSlotForEquipment(eqID, path)
		if goodPath then return goodPath end
	end
end

function ShipEquip:tryFixOneBadSlot(dir, assumeBad)

	local value
	if not dir then
		dir = {}
		value = self.value
	else
		value = self:valueNodeByPath(dir).slots
	end

	for slotID, node in pairs(value) do

		local path = table.copy(dir)
		table.insert(path, slotID)

		local eqID = type(node) == 'table' and node.id or node

		local slotlessOK = eqID and isPathSlotless(path) and isEquipmentSlotless(eqID)
		local goodEquipOnBadPlace = eqID and Equipment.Get(eqID) and (not self:canPutItemOnSlot(eqID, path) and not slotlessOK or assumeBad)

		local assumeBadChildren = assumeBad
		if goodEquipOnBadPlace then
			local goodPath = self:findSlotForEquipment(eqID)
			if goodPath then
				putNode(self.value, node, goodPath)
				self:removeItem(path)
				return true
			end
			assumeBadChildren = true
		end

		if type(node) == 'table' then
			if self:tryFixOneBadSlot(path, assumeBadChildren) then return true end
		end
	end
end

local function checkNonExistentEquipment(value)

	local function checkID(itemID)
		local obj = Equipment.Get(itemID)
		assert(obj, "Non existent equipment: " .. tostring(itemID))
	end

	for _, node in pairs(value) do
		if type(node) == 'table' then
			-- id is allowed to be empty, there may be subslots
			if node.id then
				checkID(node.id)
			end
		elseif type(node) == 'string' then
			checkID(node)
		else
			assert(false, "Unexpected node type in the equipment tree")
		end
	end
end

function ShipEquip:fromStartVariant(variant)

	self.value = table.deepcopy(variant.equipment)
	checkNonExistentEquipment(self.value)
	self.lock = true
end

function ShipEquip:cleanupValue(value)
	for k, v in pairs(value) do
		if type(v) == 'table' then
			if v.slots then

				if v.id then
					self.removeNotProvidedSlots(v.id, v.slots)
				end

				if utils.count(v.slots) ~= 0 then
					self:cleanupValue(v.slots)
				end

				if utils.count(v.slots) == 0 then
					-- v.id can be nil, if it had subslots, bu it itself was deleted earlier
					-- now there are no subslots and it will be deleted completely
					value[k] = v.id
				end
			else
				assert(false, 'item is table, but has no "slots" field')
			end
		end
	end
end

function ShipEquip:mergeSlots(slots, value)

	local nodes = {}

	if slots then
		for k, v in pairs(slots) do
			nodes[k] = { object = v }
		end
	end

	if value then -- merge empty and orphaned slots
		for k, _ in pairs(value) do
			-- don't merge numbered (slotless) equipment
			if not nodes[k] and type(k) ~= 'number' then
				nodes[k] = { orphaned = true  }
			end
		end
	end

	if utils.count(nodes) == 0 then return end

	for id, node in pairs(nodes) do
		local eq = value and value[id]
		local eqName = eq
		local eqSlots
		if eq and type(eq) ~= 'string' then
			eqName = eq.id
			eqSlots = eq.slots
		end
		if eqName then
			local obj = Equipment.Get(eqName)
			if obj and obj.provides_slots then
				local slotTable = {}
				for _, slot in pairs(obj.provides_slots) do
					slotTable[slot.id] = slot
				end

				node.children = self:mergeSlots(slotTable, eqSlots)
			end
			node.installed = eqName
		elseif eqSlots then
			node.children = self:mergeSlots(nil, eqSlots)
		end
		node.prettyName = self:createPrettySlotName(id, node.object)
	end
	return nodes
end

function ShipEquip:update()

	local repCount = 0
	repeat
		self:cleanupValue(self.value)
		self.slots = self:mergeSlots(HullConfig.GetHullConfigs()[ShipType.value].slots, self.value)

		repCount = repCount + 1
		assert(repCount < 1000, "probably endless loop")

	until not self:tryFixOneBadSlot()

	local function createViewData(result, slots, dir)

		if not slots then return end

		local sorted = {}
		for id, node in pairs(slots) do
			if node.object and node.object.required or node.installed then
				table.insert(sorted, id)
			end
		end
		table.sort(sorted, slotLess)

		for _, slot in ipairs(sorted) do

			local eq = slots[slot]

			local path = table.copy(dir)
			table.insert(path, slot)

			table.insert(result, self:viewDataForItem(eq.installed, path))

			if eq.children then
				createViewData(result, eq.children, path)
			end
		end
	end
	self.viewData = {}
	createViewData(self.viewData, self.slots, {})

	for i, slotless in ipairs(self.value) do
		table.insert(self.viewData, self:viewDataForItem(slotless, { i }))
	end

	local sameName = {}
	for id, eq in pairs(Equipment.new) do

		local localized

		local l = Lang.GetResource(eq.l10n_resource)
		localized = l[eq.l10n_key] or id

		if sameName[localized] then
			table.insert(sameName[localized], id)
		else
			sameName[localized] = { id }
		end
	end

	self.equipNames = {}
	for localized, ids in pairs(sameName) do

		-- assume that only equipment differing in size may have an identical name
		if #ids > 1 then
			for _, id in ipairs(ids) do
				local slot = Equipment.new[id].slot
				local size = "?"
				if not slot then
					logWarning("Equipment with a non-unique localized name has no slot: " .. id)
				elseif not slot.size then
					logWarning("Equipment with a non-unique localized name has no size: " .. id)
				else
					size = self.sizeLetter .. tostring(slot.size)
				end

				self.equipNames[id] = localized .. " " .. size
			end
		else
			self.equipNames[ids[1]] = localized
		end
	end

	self.editDialog:updateData(self)

	local function addToList(list, obj, count)
		for k, v in pairs(list) do
			if v.obj == obj then
				list[k].count = list[k].count + count
				return
			end
		end
		table.insert(list, { obj = obj, count = count })
	end

	self.hyperDriveClass = 0
	self.thrusterUpgradeLevel = 0
	self.summaryList = {}
	self.valid = true
	self.volume = 0
	self.mass = 0

	for _, item in ipairs(self.viewData) do
		if item.errorString then
			self.valid = false
		end
		if item.volume then
			self.volume = self.volume + item.volume
		end
		if item.mass then
			self.mass = self.mass + item.mass
		end

		local eq = Equipment.Get(item.itemID)
		local slot = self:slotObjectByPath(item.path)
		local isThruster = slot and slot.type and EquipSet.SlotTypeMatches(slot.type, "thruster")
		local isHyperDrive = slot and slot.type and EquipSet.SlotTypeMatches(slot.type, "hyperdrive")

		if isThruster then
			self.thrusterUpgradeLevel = eq and eq.capabilities and eq.capabilities.thruster_power or 0
		elseif isHyperDrive then
			self.hyperDriveClass = eq and eq.slot and eq.slot.size or 0
		elseif eq then
			addToList(self.summaryList, eq, 1)
		end
	end

	ShipSummary:prepareAndValidateParamList()
end

ShipEquip.editDialog = ModalWindow.New('ShipEquipEditDialog', function(self)

	local listWidth = self.layout.width / 2

	ui.text(self.name)
	ui.text("")

	local childHeight = self.layout.height - ui.getCursorPos().y

	ui.child('ShipEquipEditDialogLeft', Vector2(listWidth, childHeight), function()

		ui.alignTextToFramePadding()
		ui.text(self.leftPanel.label)
		ui.sameLine()

		-- https://github.com/ocornut/imgui/issues/455
		if ui.isWindowFocused("RootAndChildWindows") and not ui.isAnyItemActive() and not ui.isMouseClicked(0) and self:topmost() then
			ui.setKeyboardFocusHere(0)
		end
		local filter, entered = ui.inputText("##EditDialogListFilter", self.itemFilter)
		if entered then
			self.itemFilter = filter
			self.leftPanel:masterFilter(self, true)
		end

		ui.separator()

		ui.child('ShipEquipEditDialogLeftList', ui.getContentRegion(), function()
			local changed = self.leftPanel:show(self)
			if changed then
				self.rightPanel:slaveFilter(self)
			end
		end)
	end)

	ui.sameLine()

	ui.child('ShipEquipEditDialogRight', Vector2(listWidth, childHeight), function()

		ui.alignTextToFramePadding()
		ui.text(self.rightPanel.label)

		ui.separator()

		ui.child('ShipEquipEditDialogRightList', ui.getContentRegion(), function()
			self.rightPanel:show(self)
		end)
	end)

	local function property(key, value, format)
		ui.withStyleColors({Text = ui.theme.colors.fontDark}, function()
			ui.text(key)
		end)
		ui.sameLine()
		ui.text(safeFormat(value, format))
	end

	ui.separator()

	if self.selectedItem then
		property(leq.STAT_VOLUME, self.selectedItemProperties.volume, ui.Format.Volume)
		ui.sameLine()
		property(lc.MASS, self.selectedItemProperties.mass, function(v) return ui.Format.Mass(v * 1000, 0) end)
	else
		ui.text("")
	end

	if ui.button(self.rightPanel.switchLabel) then

		local temp = self.rightPanel
		self.rightPanel = self.leftPanel
		self.leftPanel = temp
		self:updateViews(true)
	end

	ui.sameLine()

	local canInstall = self.selectedSlot and self.selectedItem
	local variant = ui.theme.buttonColors

	if ui.button(lui.INSTALL, nil, canInstall and variant.default or variant.disabled) and canInstall then

		local replaced = ShipEquip:itemByPath(self.selectedSlot)

		if self.selectedItem == replaced then
			msgbox.OK(leq.THE_SAME_EQUIPMENT_HAS_BEEN_ALREADY_INSTALLED)
		elseif replaced then
			msgbox.OK_CANCEL(leq.SLOT_IS_NOT_EMPTY_REPLACE_THE_EQUIPMENT_QUESTION, function()
				putNode(ShipEquip.value, self.selectedItem, self.selectedSlot)
				ShipEquip:update()
				self:updateViews()
			end)
		else
			putNode(ShipEquip.value, self.selectedItem, self.selectedSlot)
			ShipEquip:update()
			self:updateViews()
		end
	end

	ui.sameLine()

	local valueNode = self.selectedSlot and ShipEquip:valueNodeByPath(self.selectedSlot)

	local canDelete = valueNode

	if ui.button(lui.DELETE, nil, canDelete and variant.default or variant.disabled) and canDelete then
		ShipEquip:removeItem(self.selectedSlot)
		ShipEquip:update()
		self:updateViews()
	end

	ui.sameLine()

	local canCut = valueNode

	if ui.button(lui.CUT, nil, canCut and variant.default or variant.disabled) and canCut then
		self.clipBoard = valueNode
		ShipEquip:removeItem(self.selectedSlot)
		ShipEquip:update()
		self:updateViews()
	end

	ui.sameLine()

	local canCopy = valueNode
	if ui.button(lui.COPY, nil, canCopy and variant.default or variant.disabled) and canCopy then
		self.clipBoard = valueNode
	end

	ui.sameLine()

	local canPaste
	if self.clipBoard and self.selectedSlot then
		local sourceID = ShipEquip:itemIdFromValueNode(self.clipBoard)
		if sourceID then
			canPaste = ShipEquip:canPutItemOnSlot(sourceID, self.selectedSlot)
		end
	end
	if ui.button(lui.PASTE, nil, canPaste and variant.default or variant.disabled) and canPaste then
		local target = ShipEquip:valueNodeByPath(self.selectedSlot)
		if target then
			msgbox.YES_NO(leq.TARGET_SLOT_IS_NOT_EMPTY_PUT_THE_REPLACED_ONE_IN_THE_CLIPBOARD_QUESTION, {
				yes = function()
					self.clipBoard = target
				end
			})
		end
		local copied = type(self.clipBoard) == 'table' and table.deepcopy(self.clipBoard) or self.clipBoard
		putNode(ShipEquip.value, copied, self.selectedSlot)
		ShipEquip:update()
		self:updateViews()
	end

	ui.sameLine()

	if ui.button(lui.CLOSE) then
		self:close()
	end
end,
function (_, drawPopupFn)
	ui.setNextWindowPosCenter('Always')
	ui.withFont(Defs.mainFont, drawPopupFn)
end)

function ShipEquip.editDialog:updateData(host)

	self.items = {}
	for id, localized in pairs(host.equipNames) do
		if host:compatibleWithShip(id, host.slots) then
			table.insert(self.items, { id = id, localized = localized })
		end
	end

	table.sort(self.items, function(a, b) return a.localized < b.localized end)

	local function flatSlots(result, slots, dir)

		if not slots then return end

		local sorted = {}
		for k, _ in pairs(slots) do
			table.insert(sorted, k)
		end
		table.sort(sorted, slotLess)

		for _, slot in ipairs(sorted) do

			local path = table.copy(dir)
			table.insert(path, slot)

			table.insert(result, { path = path, view = self.generateRow(path) })
			if slots[slot].children then
				flatSlots(result, slots[slot].children, path)
			end
		end

		return result
	end
	self.slots = {}
	flatSlots(self.slots, host.slots, {})

	table.insert(self.slots, { path = {}, view = leq.SLOTLESS })

	-- add slotless equipment from value to the slots
	for i, id in ipairs(host.value) do
		table.insert(self.slots, { path = { i }, grayed = true, view = ' ← ' .. host:getPrettyItemName(id) })
	end
end

ShipEquip.editDialog.itemPanel = { label = leq.ITEM, switchLabel = leq.BY_ITEM }
ShipEquip.editDialog.slotPanel = { label = leq.SLOT, switchLabel = leq.BY_SLOT }

function ShipEquip.editDialog.itemPanel:show(dialog)

	local changed = false

	if #dialog.itemsFiltered == 0 then
		ui.text(leq.NO_SUITABLE_EQUIPMENT)
		return false
	end

	for i, item in ipairs(dialog.itemsFiltered) do

		local selected = dialog.selectedItem and dialog.selectedItem == item.id

		if ui.selectable(item.localized .. "##" .. tostring(i), selected) then
			dialog.selectedItem = item.id
			dialog.selectedItemProperties = ShipEquip:getEquipParams(item.id)
			changed = true
		end

		if selected and self.justFiltered then
			self.justFiltered = false
			ui.setScrollHereY()
		end
	end

	return changed
end

-- so that we can faster compare paths using operator "=" for references
function ShipEquip.editDialog.slotPanel.fixupPaths(dialog)

	if dialog.selectedSlot then
		for _, slot in ipairs(dialog.slots) do
			if ShipEquip.pathIsEqual(dialog.selectedSlot, slot.path) then
				dialog.selectedSlot = slot.path
				break
			end
		end
	end
end

function ShipEquip.editDialog.slotPanel:show(dialog)

	if #dialog.slotsFiltered == 0 then
		ui.text(leq.NO_SUITABLE_SLOTS)
		return false
	end

	local changed = false

	for i, slot in ipairs(dialog.slotsFiltered) do

		local grayed = slot.grayed
		if grayed then ui.raw.PushStyleColor("Text", ui.theme.colors.fontDark) end

		local selected = dialog.selectedSlot and dialog.selectedSlot == slot.path

		if ui.selectable(slot.view .. "##" .. tostring(i), selected) and not slot.grayed then
			dialog.selectedSlot = slot.path
			changed = true
		end

		if selected and self.justFiltered then
			self.justFiltered = false
			ui.setScrollHereY()
		end

		if grayed then ui.raw.PopStyleColor(1) end
	end

	return changed
end

local function matchesString(name, str)
	return string.match(string.lower(name), string.lower(str))
end

function ShipEquip:compatibleWithShip(itemID, nodes, dir)

	if not dir then dir = {} end

	for id, node in pairs(nodes) do

		local path = table.copy(dir)
		table.insert(path, id)

		if self:canPutItemOnSlot(itemID, path) then return true end
		if node.children and self:compatibleWithShip(itemID, node.children, path) then
			return true
		end
	end

	-- slotless on top-level, always compatible
	if #dir == 0 and isEquipmentSlotless(itemID) then return true end

	return false
end

function ShipEquip.editDialog.itemPanel:masterFilter(dialog, moveToCursor)

	self.justFiltered = moveToCursor

	local matchFilter = string.len(dialog.itemFilter) > 0 and dialog.itemFilter

	if not matchFilter then
		dialog.itemsFiltered = dialog.items
		return
	end

	dialog.itemsFiltered = utils.filter_array(dialog.items, function(item)
		return matchesString(item.localized, matchFilter)
	end)
end

function ShipEquip.editDialog.itemPanel:slaveFilter(dialog, moveToCursor)

	self.justFiltered = moveToCursor

	if not dialog.selectedSlot then
		dialog.itemsFiltered = {}
		return
	end

	dialog.itemsFiltered = utils.filter_array(dialog.items, function(item)
		return ShipEquip:canPutItemOnSlot(item.id, dialog.selectedSlot)
	end)

	-- ensure selected is visible in list
	if dialog.selectedItem then
		for _, item in ipairs(dialog.itemsFiltered) do
			if dialog.selectedItem == item.id then return end
		end
	end
	dialog.selectedItem = nil
end

local function filterSlotsWithParents(slots, passFnc)

	local pass = {}

	for _, slot in ipairs(slots) do
		if passFnc(slot) then
			pass[slot] = true
		end
	end

	local function isParent(slot, child)

		if not slot or not slot.path or not child or not child.path then return false end
		if #slot.path == 0 then return isPathSlotless(child.path) end
		if #child.path == 0 or #child.path <= #slot.path then return false end

		for i = 1, #slot.path, 1 do
			if slot.path[i] ~= child.path[i] then return false end
		end
		return true
	end

	local function isParentForSome(slot, childTable)
		for child, _ in pairs(childTable) do
			if isParent(slot, child) then return true end
		end
		return false
	end

	for _, slot in ipairs(slots) do
		if isParentForSome(slot, pass) then
			pass[slot] = true
			slot.grayed = true
		end
	end

	return utils.filter_array(slots, function(slot)
		return pass[slot]
	end)
end

function ShipEquip.editDialog.slotPanel:masterFilter(dialog, moveToCursor)

	self.justFiltered = moveToCursor

	for _, slot in pairs(dialog.slots) do
		slot.grayed = false
	end

	local matchFilter = string.len(dialog.itemFilter) > 0 and dialog.itemFilter
	if not matchFilter then
		dialog.slotsFiltered = dialog.slots
		return
	end

	dialog.slotsFiltered = filterSlotsWithParents(dialog.slots, function(slot)
		return matchesString(slot.view, matchFilter)
	end)
end

function ShipEquip.editDialog.slotPanel:slaveFilter(dialog, moveToCursor)

	self.justFiltered = moveToCursor

	if not dialog.selectedItem then
		dialog.slotsFiltered = {}
		return
	end

	for _, slot in pairs(dialog.slots) do
		slot.grayed = false
	end

	dialog.slotsFiltered = filterSlotsWithParents(dialog.slots, function(slot)
		return ShipEquip:canPutItemOnSlot(dialog.selectedItem, slot.path)
	end)

	-- ensure selected is visible in list
	if dialog.selectedSlot then
		for _, slot in ipairs(dialog.slotsFiltered) do
			if dialog.selectedSlot == slot.path then return end
		end
	end
	dialog.selectedSlot = nil
end

function ShipEquip:openEditDialog(path)
	local dialog = self.editDialog
	dialog.name = leq.EDIT_EQUIPMENT
	dialog.selectedItem = nil
	dialog.selectedSlot = path
	dialog.clipBoard = nil
	dialog.itemFilter = ""
	dialog.leftPanel = dialog.slotPanel
	dialog.rightPanel = dialog.itemPanel
	dialog:updateViews(true)
	dialog:open()
end

function ShipEquip.editDialog:updateViews(moveToCursor)
	self.slotPanel.fixupPaths(self)
	self.leftPanel:masterFilter(self, moveToCursor)
	self.rightPanel:slaveFilter(self, moveToCursor)
end

function ShipEquip.editDialog.generateRow(path)

	local indent = ''

	for _ = 2, #path, 1 do
		indent = indent .. ShipEquip.indent
	end

	local slotName = ShipEquip:getPrettySlotName(path)
	local slot = ShipEquip:slotNodeByPath(path)

	if slot and slot.orphaned then
		slotName = slotName .. ' ' .. ShipEquip.warningSign
	end

	local item = ShipEquip:itemByPath(path)
	local itemName = ''
	if item then
		itemName = ShipEquip:getPrettyItemName(item)
		if itemName and string.len(itemName) > 0 then
			itemName = ' ← ' .. itemName
		else
			itemName = ''
		end
	end
	return indent .. slotName .. itemName
end


local function patch_v90_v91(old)

	local newPrefixes = {
		'hyperspace', 'misc', 'sensor', 'hull', 'laser'
	}

	local function tryWithPrefix(oldID)
		for _, prefix in ipairs(newPrefixes) do
			local prefixed = prefix .. '.' .. oldID
			local obj = Equipment.Get(prefixed)
			if obj then return prefixed end
		end
		return oldID
	end

	local new = {}
	local i = 1

	for _, section in ipairs({ 'engine', 'laser_front', 'laser_rear' }) do
		if old[section] then
			new['old_equipment_' .. tostring(i)] = tryWithPrefix(old[section])
			i = i + 1
		end
	end

	for _, eqHunk in ipairs(old.misc) do
		for _ = 1, eqHunk.amount, 1 do
			new['old_equipment_' .. tostring(i)] = tryWithPrefix(eqHunk.id)
			i = i + 1
		end
	end

	return new
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

ShipEquip.reader = Helpers.versioned {{
	version = 89, -- valid up to v90
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

		return patch_v90_v91 {
			engine = engine,
			laser_front = laser_front,
			laser_rear = laser_rear,
			misc = misc
		}
	end
},{
	version = 91,
	fnc = function(saveGame)

		local equip, errorString = Helpers.getPlayerShipParameter(saveGame, "lua_components/EquipSet/installed")
		if errorString then return nil, errorString end

		local value = {}
		local success = true
		local sorted = {}

		for slotID, data in pairs(equip) do

			local path = {}
			for i in string.gmatch(slotID, "[^#]+") do
				table.insert(path, i)
			end

			local eqID = data.__proto and data.__proto.id

			if eqID then
				table.insert(sorted, { id = eqID, path = path })
			else
				success = false
			end
		end

		-- make sure the parent is added before the child
		table.sort(sorted, function(x1, x2)
			return #x1.path < #x2.path
		end)

		for _, v in ipairs(sorted) do
			putNode(value, v.id, v.path)
		end

		if not success then
			errorString = lui.FAILED_TO_RECOVER_SOME_EQUIPMENT
		end
		return value, errorString
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

local function rowWithAlert(output, label, current, project, alertIf, unitStr, format)
	if current == nil then current = 0 end
	if project == nil then project = 0 end
	if unitStr == nil then unitStr = '' end
	if not format then format = '%d' end
	local color
	local valid = not alertIf(current, project)
	if not valid then color = ui.theme.colors.alertRed end
	output.valid = output.valid and valid
	return { label .. ":", string.format(format .. " / " .. format .. unitStr .. " ", current, project), color = color }
end

-- prepare a table for output and at the same time check the parameters
function ShipSummary:prepareAndValidateParamList()
	local def = ShipDef[ShipType.value]
	local freeCargo = def.cargo
	self.cargo.valid = true
	self.equip.valid = true
	local eq_n_cargo = { valid = true }
	freeCargo = math.max(freeCargo, 0)
	local paramList = {
		rowWithAlert(self.equip, lui.CAPACITY, ShipEquip.volume, def.equipCapacity, greater, lc.UNIT_CUBIC_METERS, "%.1f"),
		rowWithAlert(self.cargo, lui.CARGO_SPACE, ShipCargo.mass, freeCargo, greater, 't'),
		rowWithAlert(self.equip, lui.CREW_CABINS, #Crew.value + 1, def.maxCrew, greater),
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

	ui.child("param_table1", Vector2(self.layout.width, self.layout.table1_height), function()
		local p1 = ui.getCursorPos()
		textTable.drawTable(2, { self.layout.width - self.valueWidth, self.valueWidth }, paramList)
		local p2 = ui.getCursorPos()
		self.layout.table1_height = p2.y - p1.y
	end)

	ui.text("")

	local fuel_mass = def.fuelTankMass * ShipFuel.value * 0.01 -- ShipFuel.value in %
	local mass_with_fuel = def.hullMass + ShipCargo.mass + ShipEquip.mass + fuel_mass

	-- adapted from ShipType
	local power_mul = 1.0 + math.clamp(ShipEquip.thrusterUpgradeLevel, 0, 3) * 0.1

	local fwd_cap = def.linAccelerationCap.FORWARD
	local fwd_acc = math.min(def.linearThrust.FORWARD * power_mul / mass_with_fuel / 1000, fwd_cap)
	local up_cap = def.linAccelerationCap.UP
	local up_acc = math.min(def.linearThrust.UP * power_mul / mass_with_fuel / 1000, up_cap)
	local hyperclass = ShipEquip.hyperDriveClass

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

	ui.child("##param_table2", Vector2(self.layout.width, self.layout.table2_height), function()
		local p1 = ui.getCursorPos()
		textTable.drawTable(2, { self.layout.width - self.valueWidth, self.valueWidth }, {
		{ lui.ALL_UP_WEIGHT..":", string.format("%dt", mass_with_fuel ) },
		{ lui.HYPERSPACE_RANGE..":", string.format("%.1f / %.1f " .. lui.LY, hyper_range, range_max) },
		{ lui.DELTA_V..":",      string.format("%s %s", ui.Format.SpeedUnit(delta_v)) },
		{ lui.REFUEL .. " " .. lui.DELTA_V..":", string.format("%s %s", ui.Format.SpeedUnit(p_delta_v)) },
		{ lui.FORWARD_ACCEL..":",  string.format("%.1f / %.1f g", fwd_acc / 9.81, fwd_cap / 9.81) },
		{ lui.UP_ACCEL..":",       string.format("%.1f / %.1f g", up_acc / 9.81, up_cap / 9.81) },
		})
		local p2 = ui.getCursorPos()
		self.layout.table2_height = p2.y - p1.y
	end)
end


--
-- draw tab
--
local function draw()
	ui.child("params_and_cargo", Vector2(layout.leftWidth, Defs.contentRegion.y), function()

		ShipType:draw()
		ShipLabel:draw()
		ShipName:draw()
		ShipFuel:draw()
		ui.text("")
		ShipSummary:draw()
		ui.text("")
		ShipCargo:draw()

	end)

	ui.sameLine()

	ui.child("ship_and_equip", Vector2(layout.rightWidth, Defs.contentRegion.y), function()

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

		ShipEquip:draw()
	end)
end

local function updateLayout()

	layout.leftWidth = math.round(0.35 * Defs.contentRegion.x)
	layout.rightWidth = Defs.contentRegion.x - layout.leftWidth - Defs.gap.x
	layout.patternDragWidth = ui.calcTextSize("<-- " .. lui.PATTERN .. " 00/00 -->").x

	local paramWidth = layout.leftWidth - Defs.gap.x

	-- link layouts to align the width of labels
	ShipType.layout = { width = paramWidth }
	ShipName.layout = ShipType.layout

	-- do not link layouts because the label lengths are too different
	ShipFuel.layout = { width = paramWidth }
	ShipLabel.layout = { width = paramWidth }

	ShipModel.layout = {
		width = layout.rightWidth,
		height = Defs.contentRegion.y * 0.5
	}
	ShipSummary.layout = { width = paramWidth, table1_height = 0, table2_height = 0 }
	ShipSummary.valueWidth = ui.calcTextSize("--3000t / 3000t--").x -- to update

	ShipEquip.layout = {
		width = ShipModel.layout.width,
		height = Defs.contentRegion.y - ShipModel.layout.height - Defs.gap.y
	}

	ShipEquip.editDialog.layout = {
		width = ShipEquip.layout.width,
		height = Defs.lineHeight * 18,
	}
end

local function updateParams()
	ShipType:setShipID(ShipType.value)
	ShipCargo:updateDrawItems()
end

function ShipType.updated()
	ShipEquip:update()
	ShipModel:updateModel()
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
	Fuel = ShipFuel,
}
