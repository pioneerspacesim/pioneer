-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'

local l = Lang.GetResource("ui-core")
local ui = import 'pigui/pigui.lua'
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local TabView = import 'pigui/views/tab-view.lua'

local itemSpacing = Vector2(math.ceil(6 * (ui.screenHeight / 1200)), math.ceil(12 * (ui.screenHeight / 1200)))

local stationView

if not stationView then
	stationView = TabView.New("space_station")
	stationView.shipSummary = function()
		local inventoryPadding = Vector2(20, 10)
		local player = Game.player

		ui.withFont(pionillium.medlarge.name, pionillium.large.size, function()
			ui.withStyleVars({WindowPadding = inventoryPadding, ItemSpacing = itemSpacing}, function()
				ui.child("shipInventoryContainer", Vector2(0, 0), {"AlwaysUseWindowPadding"}, function()
					ui.text('')
					ui.columns(4, 'shipInventory', false)
					ui.text(l.CASH .. ': ' .. Format.Money(Game.player:GetMoney()))
					ui.nextColumn()
					ui.text(l.CAPACITY .. ': ')
					ui.sameLine()
					local gaugePos = ui.getWindowPos() + ui.getCursorPos() + inventoryPadding
					local gaugeWidth = ui.getContentRegion().x - inventoryPadding.x - itemSpacing.x
					ui.gauge(gaugePos, player.usedCapacity, '', string.format('%%it %s / %it %s', l.USED, player.freeCapacity, l.FREE), 0, player.usedCapacity + player.freeCapacity, icons.market, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(l.CABINS .. ': ')
					ui.sameLine()
					local cabins_free = player.cabin_cap or 0
					local cabins = player:GetEquipCountOccupied("cabin")
					gaugePos = ui.getWindowPos() + ui.getCursorPos() + inventoryPadding
					gaugeWidth = ui.getContentRegion().x - inventoryPadding.x - itemSpacing.x
					ui.gauge(gaugePos, cabins - cabins_free, '', string.format('%%it %s / %it %s', l.USED, cabins, l.FREE), 0, cabins, icons.personal, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(l.LEGAL_STATUS .. ': ' .. l.CLEAN)
					ui.columns(1, '', false)
				end)
			end)
		end)
	end

	ui.registerModule("game", function() stationView:renderTabView() end)
end

return stationView