-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")
local Game = import("Game")
local EquipDef = import("EquipDef")

local SmallLabeledButton = import("ui/SmallLabeledButton")
local InfoGauge = import("ui/InfoGauge")

local ui = Engine.ui
local t = Translate:GetTranslator()

local econTrade = function ()

	local cash = Game.player:GetMoney()

	local stats = Game.player:GetStats()

	local usedCabins = Game.player:GetEquipCount("CABIN", "PASSENGER_CABIN")
	local totalCabins = Game.player:GetEquipCount("CABIN", "UNOCCUPIED_CABIN") + usedCabins

	-- Using econTrade as an enclosure for the functions attached to the
	-- buttons in the UI object that it returns. Seems like the most sane
	-- way to handle it; hopefully the enclosure will evaporate shortly
	-- after the UI is disposed of.

	-- Make a cargo list widget that we can revisit and update
	local cargoListWidget = ui:Margin(0)

	function updateCargoListWidget ()

		local cargoNameColumn = {}
		local cargoQuantityColumn = {}
		local cargoJettisonColumn = {}

		for i = 1,#Constants.EquipType do
			local type = Constants.EquipType[i]
			if type ~= "NONE" then
				local et = EquipDef[type]
				local slot = et.slot
				if slot == "CARGO" then
					local count = Game.player:GetEquipCount(slot, type)
					if count > 0 then
						table.insert(cargoNameColumn, ui:Label(et.name))
						table.insert(cargoQuantityColumn, ui:Label(count.."t"))

						local jettisonButton = SmallLabeledButton.New(t("Jettison"))
						jettisonButton.button.onClick:Connect(function ()
							Game.player:Jettison(type)
							updateCargoListWidget()
							cargoListWidget:SetInnerWidget(updateCargoListWidget())
						end)
						table.insert(cargoJettisonColumn, jettisonButton.widget)
					end
				end
			end
		end

		-- Function returns a UI with which to populate the cargo list widget
		return
			ui:VBox(10):PackEnd({
				ui:Label(t("CARGO")):SetFont("HEADING_LARGE"),
				ui:Scroller():SetInnerWidget(
					ui:Grid(3,1)
						:SetColumn(0, { ui:VBox():PackEnd(cargoNameColumn) })
						:SetColumn(1, { ui:VBox():PackEnd(cargoQuantityColumn) })
						:SetColumn(2, { ui:VBox():PackEnd(cargoJettisonColumn) })
				)
			})
	end

	cargoListWidget:SetInnerWidget(updateCargoListWidget())

	local cargoGauge = InfoGauge.New({
		formatter = function (v)
			local stats = Game.player:GetStats()
			return string.format("%d/%dt", stats.usedCargo, stats.freeCapacity)
		end
	})
	cargoGauge:SetValue(stats.usedCargo/stats.freeCapacity)

	local fuelGauge = InfoGauge.New({
		label          = ui:NumberLabel("PERCENT"),
		warningLevel   = 0.1,
		criticalLevel  = 0.05,
		levelAscending = false,
	})
	fuelGauge.label:Bind("valuePercent", Game.player, "fuel")
	fuelGauge.gauge:Bind("valuePercent", Game.player, "fuel")

	-- Define the refuel button
	local refuelButton = SmallLabeledButton.New(t('Refuel'))

	local refuelButtonRefresh = function ()
		if Game.player.fuel == 100 or Game.player:GetEquipCount('CARGO', 'WATER') == 0 then refuelButton.widget:Disable() end
		fuelGauge:SetValue(Game.player.fuel/100)
	end
	refuelButtonRefresh()

	local refuel = function ()
		-- UI button where the player clicks to refuel...
		Game.player:Refuel(1)
		-- ...then we update the cargo list widget...
		cargoListWidget:SetInnerWidget(updateCargoListWidget())
		-- ...and the gauge.
		stats = Game.player:GetStats()
		cargoGauge:SetValue(stats.usedCargo/stats.freeCapacity)

		refuelButtonRefresh()
	end

	refuelButton.button.onClick:Connect(refuel)

	return ui:Expand():SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Margin(5, "HORIZONTAL",
					ui:VBox(20):PackEnd({
						ui:Grid(2,1)
							:SetColumn(0, {
								ui:VBox():PackEnd({
									ui:Label(t("CASH")..":"),
									ui:Margin(10),
									ui:Label(t("CARGO_SPACE")..":"),
									ui:Label(t("CABINS")..":"),
									ui:Margin(10),
								})
							})
							:SetColumn(1, {
								ui:VBox():PackEnd({
									ui:Label(string.format("$%.2f", cash)),
									ui:Margin(10),
									cargoGauge.widget,
									ui:Grid(2,1):SetRow(0, { ui:Label(t("Total: ")..totalCabins), ui:Label(t("USED")..": "..usedCabins) }),
									ui:Margin(10),
								})
							}),
						ui:Grid({50,10,40},1)
							:SetRow(0, {
								ui:HBox(5):PackEnd({
									ui:Label("Fuel:"),
									fuelGauge,
								}),
								nil,
								refuelButton.widget,
							})
					})
				)
			})
			:SetColumn(1, {
				cargoListWidget
			})
	)
end

return econTrade
