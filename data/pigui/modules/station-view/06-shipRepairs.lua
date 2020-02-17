-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = import 'pigui/pigui.lua'
local StationView = import 'pigui/views/station-view'
local ShipDef = import("ShipDef")
local Game = import "Game"
local Rand = import "Rand"
local InfoFace = import 'ui/PiguiFace'
local Format = import "Format"
local Character = import "Character"
local ModalWindow = import 'pigui/libs/modal-win.lua'
local Lang = import 'Lang'
local l = Lang.GetResource("ui-core")

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local colors = ui.theme.colors
local icons = ui.theme.icons

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer

local face = nil
local stationSeed = false
local damageToRepair = nil
local repair_cost = 0

local widgetSizes = ui.rescaleUI({
	itemSpacing = Vector2(4, 9),
	windowPadding = Vector2(14, 14),
	faceSize = Vector2(586,565),
	buttonSize = Vector2(100, 0),
	verticalDummy = Vector2(0, 50),
	gaugeWidth = 600,
}, Vector2(1600, 900))


local popup = ModalWindow.New('ChiefMechanicPopup', function(self)
	ui.text(l.YOU_DONT_HAVE_ENOUGH_MONEY_FOR_THAT_OPTION)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)


local tryRepair = function (damage, price)
	if Game.player:GetMoney() >= price then
		Game.player:AddMoney(-price)
		Game.player:SetHullPercent(Game.player:GetHullPercent() + damage)
	else
		popup:open()
	end
end


local getRepairCost = function (percent, shipDef)
	-- repairing 1% hull damage costs 0.1% of ship price
	shipDef = shipDef or ShipDef[Game.player.shipId]
	return math.ceil(shipDef.basePrice * (percent * 0.1)) * 0.01
end


local getRepairMessage = function (damage, price)
	return string.interp(
		l.REPAIR_X_HULL_DAMAGE_FOR_X, {
			damage = string.format('%.1f', damage),
			price = Format.Money(price)
		})
end


function round(num, numDecimalPlaces)
	local mult = 10^(numDecimalPlaces or 0)
	return math.floor(num * mult + 0.5) / mult
end


local function drawShipRepair()
	local hullPercent = round(Game.player:GetHullPercent())
	local damage = 100 - hullPercent

	local intro = string.interp(l.YOUR_HULL_IS_AT_X_INTEGRITY, {value = string.format('%.1f', hullPercent)})

	ui.withStyleVars({WindowPadding = widgetSizes.windowPadding,
		ItemSpacing = widgetSizes.itemSpacing}, function ()
			local infoColumnWidth = ui.getContentRegion().x - widgetSizes.faceSize.x - widgetSizes.windowPadding.x*3

			ui.child("ShipStatus", Vector2(infoColumnWidth, 0), {"AlwaysUseWindowPadding"}, function ()
				if hullPercent > 99.9 then
					ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size,
						function () ui.text(l.SHIP_IS_ALREADY_FULLY_REPAIRED) end)
				else
					ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function ()
						ui.text(getRepairMessage(damageToRepair, repair_cost))
						ui.pushItemWidth(widgetSizes.gaugeWidth)
						damageToRepair = ui.sliderInt("", damageToRepair, 1, damage, "%d%%")
						repair_cost = getRepairCost(damageToRepair, shipDef)
					end)

					ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function ()
						if  ui.button(l.PAY, widgetSizes.buttonSize) then
							tryRepair(damageToRepair, repair_cost)
							-- If we repaired more than 50% of the damage, reset
							if  damageToRepair > damage / 2 then
								damageToRepair = damage - damageToRepair
							end
						end
					end)
				end

				ui.dummy(widgetSizes.verticalDummy)

				local gaugePos = ui.getCursorScreenPos()
				gaugePos.y = gaugePos.y + 50

				local text = string.format(hullPercent)
				ui.gauge(gaugePos, hullPercent, '', l.HULL_INTEGRITY, 0, 100, icons.hull, colors.gaugeEquipmentMarket, l.HUD_HULL_STRENGTH, widgetSizes.gaugeWidth)

			end)
			ui.sameLine()
			ui.child("ChiefMechanic", Vector2(0, 0), {"AlwaysUseWindowPadding", "NoScrollbar"},
				function ()
					if(face ~= nil) then
						face:render()
			end end)
	end)
end


StationView:registerView({
	id = "shipRepairs",
	name = l.SHIP_REPAIRS,
	icon = ui.theme.icons.repairs,
	showView = true,
	draw = function ()
		ui.child("StationRepair", Vector2(0, ui.getContentRegion().y - StationView.style.height), {}, drawShipRepair)
		StationView:shipSummary()
	end,
	refresh = function ()
		local station = Game.player:GetDockedWith()
		-- Don't reset player's choice if temporarily leaving ship repair screen
		if not damageToRepair then
			damageToRepair = 1
		end
		if (station) then
			if (stationSeed ~= station.seed) then
				stationSeed = station.seed
				local rand = Rand.New(station.seed .. '-repair-guy')
				face = InfoFace.New(Character.New({ title = l.CHIEF_MECHANIC }, rand),
							{windowPadding = widgetSizes.windowPadding, itemSpacing = widgetSizes.itemSpacing, size = widgetSizes.faceSize})
			end
		end
	end,
})
