-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'
local Passengers = require 'Passengers'
local PlayerState= require 'PlayerState'

local l = Lang.GetResource("ui-core")

local ui = require 'pigui'
local colors = ui.theme.colors
local icons = ui.theme.icons
local pionillium = ui.fonts.pionillium
local TabView = require 'pigui.views.tab-view'

local stationView

local useWindowPadding = ui.ChildFlags { 'AlwaysUseWindowPadding' }

if not stationView then
	stationView = TabView.New("StationView")
	-- stationView.windowPadding = ui.rescaleUI(Vector2(18, 18))
	stationView.style = ui.rescaleUI({
		windowPadding = Vector2(18, 18),
		itemSpacing = Vector2(4,9),
		inventoryPadding = Vector2(20, 10),
		fontSize = 22,
		height = 22 + 10 + 10 + 9
	}, Vector2(1600, 900))

	function stationView:renderTab(tabFn)
		ui.withStyleVars({WindowPadding = self.style.windowPadding}, function()
			ui.child("Container", Vector2(0, -self.style.height), nil, useWindowPadding, tabFn)
		end)
		self:shipSummary()
	end

	function stationView:shipSummary()
		local player = Game.player

		ui.withFont(pionillium.medlarge.name, self.style.fontSize, function()
			ui.withStyleVars({WindowPadding = self.style.inventoryPadding, ItemSpacing = self.style.itemSpacing}, function()
				ui.child("shipInventoryContainer", Vector2(0, 0), nil, useWindowPadding, function()
					local moneyText = l.CASH .. ': ' ..  Format.Money(PlayerState.GetMoney())
					local legalText = l.LEGAL_STATUS .. ': ' .. l[PlayerState.GetLegalStatus()]
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
					local gaugePos = ui.getWindowPos() + ui.getCursorPos() + Vector2(0, ui.getTextLineHeight() / 2)
					local gaugeWidth = ui.getContentRegion().x - self.style.inventoryPadding.x - self.style.itemSpacing.x

					local fmt = "{} {} / {} {}" % {
						ui.Format.Volume(player.equipVolume), l.USED,
						ui.Format.Volume(player.totalVolume - player.equipVolume), l.FREE
					}
					ui.gauge(gaugePos, player.equipVolume, '', fmt, 0, player.totalVolume, icons.market, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(l.CABINS .. ': ')
					ui.sameLine()

					local berths_free = Passengers.CountFreeBerths(player)
					local berths_used = Passengers.CountOccupiedBerths(player)
					local berths_total = berths_used + berths_free

					gaugePos = ui.getWindowPos() + ui.getCursorPos() + Vector2(0, ui.getTextLineHeight() / 2)
					gaugeWidth = ui.getContentRegion().x - self.style.inventoryPadding.x - self.style.itemSpacing.x
					ui.gauge(gaugePos, berths_used, '', string.format('%%i %s / %i %s', l.USED, berths_free, l.FREE), 0, berths_total, icons.personal, colors.gaugeEquipmentMarket, '', gaugeWidth, ui.getTextLineHeight())
					ui.nextColumn()
					ui.text(legalText)
					ui.columns(1, '', false)
				end)
			end)
		end)
	end

	ui.registerModule("game", function()
		stationView:renderTabView()
		if stationView.isActive and ui.escapeKeyReleased() then
			Game.SetView("WorldView")
		end
	end)
end

return stationView
