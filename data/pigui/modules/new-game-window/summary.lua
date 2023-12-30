-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local Crew = require 'pigui.modules.new-game-window.crew'
local Ship = require 'pigui.modules.new-game-window.ship'
local Defs = require 'pigui.modules.new-game-window.defs'
local Format = require 'Format'
local Widgets = require 'pigui.modules.new-game-window.widgets'
local GameParam = require 'pigui.modules.new-game-window.game-param'
local ShipDef = require 'ShipDef'
local Lang = require 'Lang'
local lui = Lang.GetResource("ui-core")
local leq = Lang.GetResource("equipment-core")

local Summary = {}

local layout = {}

local Description = GameParam.New(lui.DESCRIPTION, "description")

function Description:fromStartVariant(variant)
	self.value = variant.desc
end

function Description:isValid()
	return true
end

function Description:fromSaveGame(saveGame)
	self.value = lui.RECOVER_SAVEGAME
end

---@type string
Description.value = ""

function Summary:updateLayout()
	layout.picsize = Defs.mainFont.size * 12
	layout.leftWidth = math.round(0.6 * Defs.contentRegion.x)
	layout.rightWidth = Defs.contentRegion.x - layout.leftWidth
	layout.playerParam = {}
	layout.shipParam = {}
	layout.paramsGroup = {
		child_id = "params",
		width = layout.leftWidth - layout.picsize - Defs.gap.x,
		height = layout.picsize
	}
end

function Summary:draw()
	local player = Crew.Player.Char.value

	ui.child("leftside", Vector2(layout.leftWidth, Defs.contentRegion.y), function()
		local player_member = Crew.Player.Char
		Crew:initMemberFace(player_member)
		ui.child("face", Vector2(layout.picsize, layout.picsize), function()
			player_member.face:renderFaceDisplay()
		end)
		ui.sameLine()
		Widgets.centeredIn(layout.paramsGroup, function()
			ui.beginGroup()
			Widgets.alignLabel(lui.NAME_PERSON, layout.playerParam, function()
				ui.text(player.name)
			end)
			Widgets.alignLabel(lui.CASH, layout.playerParam, function()
				ui.text(Format.Money(Crew.Player.Money.value, false))
			end)
			Widgets.alignLabel(lui.SHIP_TYPE, layout.playerParam, function()
				ui.text(ShipDef[Ship.Type.value].name)
			end)
			Widgets.alignLabel(lui.SHIP_NAME, layout.playerParam, function()
				ui.text(Ship.Name.value)
			end)
			ui.endGroup()
		end)
		Ship.Model:draw()
	end)

	ui.sameLine()

	ui.child("rightside", Vector2(layout.rightWidth - Defs.scrollWidth, Defs.contentRegion.y), function()

		ui.text("")
		if Description.value and Description.value ~= "" then
			ui.textWrapped(Description.value)
			ui.text("")
		end
		Widgets.alignLabel(lui.HYPERDRIVE, layout.shipParam, function()
			ui.text(Ship.Equip:getHyperDriveClass() > 0 and lui.YES or lui.NO)
		end)
		ui.text("")
		if #Ship.Equip.summaryList == 0 then
			Widgets.alignLabel(lui.EQUIPMENT, layout.shipParam, function() ui.text(lui.NO) end)
		else
			ui.text(lui.EQUIPMENT .. ":")
		end

		for _, eq in ipairs(Ship.Equip.summaryList) do
			-- eq: { obj, count }
			if not eq.obj.capabilities.hyperclass then
				local count = eq.count > 1 and " x " .. tostring(eq.count) or ""
				ui.text("    - " .. leq[eq.obj.l10n_key] .. count)
			end
		end

		ui.text("")
		if #Ship.Cargo.textTable == 0 then
			Widgets.alignLabel(lui.CARGO, layout.shipParam, function() ui.text(lui.NO) end)
		else
			ui.text(lui.CARGO .. ":")
		end
		for _, entry in ipairs(Ship.Cargo.textTable) do
			ui.text("    - " .. entry.label .. " x " .. entry.amount)
		end
	end)
end

Summary.TabName = lui.SUMMARY
Summary.Description = Description

return Summary
