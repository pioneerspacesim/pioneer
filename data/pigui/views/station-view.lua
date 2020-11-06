-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'

local l = Lang.GetResource("ui-core")
local ui = require 'pigui'
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local TabView = require 'pigui.views.tab-view'

local stationView

if not stationView then
	stationView = TabView.New("space_station")
	stationView.style = ui.rescaleUI({
		itemSpacing = Vector2(4,9),
		inventoryPadding = Vector2(20, 10),
		fontSize = 22,
		height = 22 + 10 + 10 + 9
	}, Vector2(1600, 900))

	function stationView:shipSummary()
		local player = Game.player

		ui.withFont(pionillium.medlarge.name, self.style.fontSize, function()
			ui.withStyleVars({WindowPadding = self.style.inventoryPadding, ItemSpacing = self.style.itemSpacing}, function()
				ui.child("shipInventoryContainer", Vector2(0, 0), {"AlwaysUseWindowPadding"}, function()
					local moneyText = l.CASH .. ': ' ..  Format.Money(Game.player:GetMoney())
					local legalText = l.LEGAL_STATUS .. ': ' .. l.CLEAN
					local moneySize = ui.calcTextSize(moneyText) + self.style.inventoryPadding + self.style.itemSpacing
					local legalSize = ui.calcTextSize(legalText) + self.style.inventoryPadding + self.style.itemSpacing
					local gaugeSize = (ui.getContentRegion().x - moneySize.x - legalSize.x) / 2
					ui.columns(4, 'shipInventory', false)
					ui.setColumnWidth(0, moneySize.x)
					ui.setColumnWidth(1, gaugeSize)
					ui.setColumnWidth(2, gaugeSize)
					ui.setColumnWidth(3, legalSize.x)
					ui.text(moneyText)
					ui.nextColumn()
					ui.text(l.CAPACITY .. ': ')
					ui.sameLine()
					local gaugePos = ui.getWindowPos() + ui.getCursorPos() + self.style.inventoryPadding
					local gaugeWidth = ui.getContentRegion().x - self.style.inventoryPadding.x - self.style.itemSpacing.x
					ui.gauge(gaugePos, player.usedCapacity, '', string.format('%%it %s / %it %s', l.USED, player.freeCapacity, l.FREE), 0, player.usedCapacity + player.freeCapacity, icons.market, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(l.CABINS .. ': ')
					ui.sameLine()
					local cabins_total = Game.player:GetEquipCountOccupied("cabin")
					local cabins_free = player.cabin_cap or 0
					local cabins_used = cabins_total - cabins_free
					gaugePos = ui.getWindowPos() + ui.getCursorPos() + self.style.inventoryPadding
					gaugeWidth = ui.getContentRegion().x - self.style.inventoryPadding.x - self.style.itemSpacing.x
					ui.gauge(gaugePos, cabins_used, '', string.format('%%i %s / %i %s', l.USED, cabins_free, l.FREE), 0, cabins_total, icons.personal, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(legalText)
					ui.columns(1, '', false)
				end)
			end)
		end)
	end

	ui.registerModule("game", function()
		stationView:renderTabView()
		if stationView.isActive and ui.noModifierHeld() and ui.isKeyReleased(ui.keys.escape) then
			Game.SetView("world")
		end
	end)
end

return stationView
