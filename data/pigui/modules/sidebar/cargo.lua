-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Commodities = require 'Commodities'
local Game = require 'Game'
local utils = require 'utils'
local Vector2 = _G.Vector2

local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core");

local ui = require 'pigui'

local colors = ui.theme.colors
local icons = ui.theme.icons
local buttonColors = ui.theme.buttonColors

local style = {
	cargoColor = colors.gaugeCargo,
	jettisonColor = colors.gaugeJettison
}

local AMT_OF_AMT = "%st / %st"

local gameView = require 'pigui.views.game'

local function draw_cargo_bar(pos, size, pct, color, tooltip)
	local section = Vector2(size.x * pct, size.y)
	ui.addRectFilled(pos, pos + size, colors.lightBlackBackground, 0, 0)
	ui.addRectFilled(pos, pos + section, color, 0, 0)

	if ui.isWindowHovered() and ui.isMouseHoveringRect(pos, pos + size) then
		ui.setTooltip(tooltip)
	end
end

local function draw_cargo_bar_section(pos, size, pct, color, tooltip)
	local section = Vector2(size.x * pct, size.y)
	ui.addRectFilled(pos, pos + section, color, 0, 0)

	if ui.isWindowHovered() and ui.isMouseHoveringRect(pos, pos + section) then
		ui.setTooltip(tooltip)
	end
end

local function transfer_button(icon, tooltip, enabled)
	local size = Vector2(ui.getTextLineHeight())
	if enabled then
		return ui.iconButton(icon, size, tooltip, nil, nil, 0)
	else
		ui.iconButton(icon, size, tooltip, buttonColors.disabled, colors.grey)
	end
end

local function transfer_buttons(amount, min, max, tooltip_reduce, tooltip_increase)
	if transfer_button(icons.time_backward_1x, tooltip_reduce, amount > min) then
		amount = amount - 1
	end
	ui.sameLine(0, 2)
	if transfer_button(icons.time_forward_1x, tooltip_increase, amount < max) then
		amount = amount + 1
	end

	return amount
end

local module = {
	side = "left",
	icon = icons.cargo_crate,
	tooltip = lui.TOGGLE_CARGO_WINDOW,
	exclusive = false,
	debugReload = function() package.reimport() end,

	ship = nil,
	transfer = {},
	transferModes = {},
}

table.insert(module.transferModes, {
	id = "Jettison",
	label = lui.JETTISON,
	color = style.jettisonColor,
	icon = icons.cargo_crate_illegal,
	tooltip = lui.JETTISON_MODE,
	action = function(ship, manifest)
		for k, v in pairs(manifest) do
			local commodity = Commodities[k]
			for i = 1, v do
				ship:Jettison(commodity)
			end
		end
	end,
	canDisplay = function(ship)
		return ship.flightState == "FLYING"
	end
})

function module:startTransfer(mode)
	self.transfer = {}
	self.transferMode = mode
end

function module:resetTransfer()
	self.transfer = {}
	self.transferMode = nil
end

function module:countTransfer()
	local amount = 0
	for k, v in pairs(self.transfer) do
		amount = amount + v
	end

	return amount
end

function module:drawModeButtons()
	local modi = {}

	for _, v in ipairs(self.transferModes) do
		if not v.canDisplay(self.ship) then
			if self.transferMode == v then
				self:resetTransfer()
			end
		else
			table.insert(modi, v)
		end
	end

	local spacing = ui.getItemSpacing().x
	local width = (ui.getButtonHeight() + spacing) * #modi - spacing

	ui.addCursorPos(Vector2(ui.getContentRegion().x - width, 0))

	for _, v in ipairs(modi) do
		local isActive = self.transferMode == v
		if ui.inlineIconButton(v.icon, v.tooltip, isActive) then
			if isActive then
				self:resetTransfer()
			else
				self:startTransfer(v)
			end
		end

		ui.sameLine()
	end

	ui.newLine()
end

function module:drawTitle()
	local cargoMgr = self.ship:GetComponent("CargoManager")

	ui.text(lui.CARGO)

	ui.sameLine()

	local pos = ui.getCursorScreenPos() + Vector2(0,
		(ui.getLineHeight() - ui.getTextLineHeight()) / 2)

	local size = Vector2(
		ui.getContentRegion().x - ui.getItemSpacing().x,
		ui.getTextLineHeight())

	local usedSpace = cargoMgr:GetUsedSpace()
	local totalSpace = cargoMgr:GetTotalSpace()

	local tooltip = AMT_OF_AMT:format(usedSpace, totalSpace)
	draw_cargo_bar(pos, size, usedSpace / totalSpace, style.cargoColor, tooltip)

	if self.transferMode then
		local amount = self:countTransfer()

		tooltip = AMT_OF_AMT:format(amount, usedSpace)
		draw_cargo_bar_section(pos, size, amount / totalSpace, self.transferMode.color, tooltip)
	end
end

function module:drawCargoRow(v, rowWidth, totalSpace)
	local commodity = Commodities[v.name]
	local transferAmt = self.transfer[v.name] or 0

	ui.tableNextRow()

	-- Draw name
	ui.tableNextColumn()
	ui.text(commodity:GetName())

	-- Draw contained amount or transferred amount
	ui.tableNextColumn()
	if self.transferMode then
		local fontCol = transferAmt > 0 and self.transferMode.color or colors.font

		ui.withStyleColors({ Text = fontCol }, function()
			ui.text(transferAmt .. "t")
		end)
	else
		ui.text(v.count .. "t")
	end

	-- Draw cargo gauge
	ui.tableNextColumn()

	local width = math.max(ui.getContentRegion().x, rowWidth / 4)
	local pos = ui.getCursorScreenPos()
	local size = Vector2(width, ui.getTextLineHeight())

	ui.dummy(size)

	local tooltip = AMT_OF_AMT:format(v.count, totalSpace)
	draw_cargo_bar(pos, size, v.count / totalSpace, style.cargoColor, tooltip)

	-- Draw transfer gauge
	if self.transferMode and transferAmt > 0 then
		tooltip = AMT_OF_AMT:format(transferAmt, v.count)
		draw_cargo_bar_section(pos, size, transferAmt / totalSpace, self.transferMode.color, tooltip)
	end

	-- Draw transfer buttons
	ui.tableNextColumn()
	if self.transferMode then
		ui.withID(commodity.name, function()
			local max = self.transferMode and v.count or 0
			self.transfer[v.name] = transfer_buttons(transferAmt, 0, max, lui.DECREASE, lui.INCREASE)
		end)
	end
end

function module:drawBody()
	local cargoMgr = self.ship:GetComponent("CargoManager")

	local sortTable = {}

	for k, v in pairs(cargoMgr.commodities) do
		table.insert(sortTable, { name = k, comm = Commodities[k], count = v.count })
	end

	table.sort(sortTable, function(a, b)
		return a.count > b.count or (a.count == b.count and a.comm:GetName() < b.comm:GetName())
	end)

	local maxWidth = ui.getContentRegion().x
	local totalSpace = cargoMgr:GetTotalSpace()

	ui.alignTextToButtonPadding()
	ui.text(lui.CARGO_CAPACITY .. ": " .. totalSpace .. "t")
	ui.sameLine()

	self:drawModeButtons()

	ui.separator()

	if cargoMgr:GetUsedSpace() > 0 then

		if ui.beginTable("cargo", 4) then
			ui.tableSetupColumn("Cargo")
			ui.tableSetupColumn("Amount")
			ui.tableSetupColumn("Gauge", { "WidthStretch" })
			ui.tableSetupColumn("Buttons")

			for _, v in ipairs(sortTable) do
				self:drawCargoRow(v, maxWidth, totalSpace)
			end

			ui.endTable()
		end

	else
		ui.alignTextToButtonPadding()
		ui.textAligned(lui.NO_CARGO, 0.5)
	end

	ui.separator()
	ui.spacing()

	ui.alignTextToButtonPadding()
	ui.text("{} {}t {} / {}t {}" % {
		lui.TOTAL,
		cargoMgr:GetUsedSpace(), lui.USED,
		cargoMgr:GetFreeSpace(), lui.FREE
	})

	if self.transferMode then
		ui.sameLine()
		local amount = self:countTransfer()

		local buttonText = string.format("%s %st", self.transferMode.label, amount)
		ui.addCursorPos(Vector2(ui.getContentRegion().x - ui.calcButtonSize(buttonText).x, 0))

		if ui.button(buttonText) then
			self.transferMode.action(self.ship, self.transfer)
			self:resetTransfer()
		end
	end
end

function module:refresh()
	self.ship = Game.player
	self:resetTransfer()
end

gameView.registerSidebarModule("cargo", module)
