-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local EquipDef = import("EquipDef")

local SmallLabeledButton = import("ui/SmallLabeledButton")
local InfoGauge = import("ui/InfoGauge")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local econTrade = function ()

	local cash = Game.player:GetMoney()

	local player = Game.player

	local usedCabins = Game.player:GetEquipCount("CABIN", "PASSENGER_CABIN")
	local totalCabins = Game.player:GetEquipCount("CABIN", "UNOCCUPIED_CABIN") + usedCabins

	-- Using econTrade as an enclosure for the functions attached to the
	-- buttons in the UI object that it returns. Seems like the most sane
	-- way to handle it; hopefully the enclosure will evaporate shortly
	-- after the UI is disposed of.

	-- Make a cargo list widget that we can revisit and update
	local cargoListWidget = ui:Margin(0)

	local cargoGauge = ui:Gauge()
	local cargoUsedLabel = ui:Label("")
	local cargoFreeLabel = ui:Label("")
	local function cargoUpdate ()
		cargoGauge:SetUpperValue(player.totalCargo)
		cargoGauge:SetValue(player.usedCargo)
		cargoUsedLabel:SetText(string.interp(l.CARGO_T_USED, { amount = player.usedCargo }))
		cargoFreeLabel:SetText(string.interp(l.CARGO_T_FREE, { amount = player.totalCargo-player.usedCargo }))
	end
	player:Connect("usedCargo", cargoUpdate)
	player:Connect("totalCargo", cargoUpdate)
	cargoUpdate()

	local cabinGauge = ui:Gauge()
	local cabinUsedLabel = ui:Label("")
	local cabinFreeLabel = ui:Label("")
	local function cabinUpdate ()
		cabinGauge:SetUpperValue(player.totalCabins)
		cabinGauge:SetValue(player.usedCabins)
		cabinUsedLabel:SetText(string.interp(l.CABIN_USED, { amount = player.usedCabins }))
		cabinFreeLabel:SetText(string.interp(l.CABIN_FREE, { amount = player.totalCabins-player.usedCabins }))
	end
	player:Connect("usedCabins", cabinUpdate)
	player:Connect("totalCabins", cabinUpdate)
	cabinUpdate()

	local equipIcon = {
		HYDROGEN =              "Hydrogen",
		LIQUID_OXYGEN =         "Liquid_Oxygen",
		METAL_ORE =             "Metal_ore",
		CARBON_ORE =            "Carbon_ore",
		METAL_ALLOYS =          "Metal_alloys",
		PLASTICS =              "Plastics",
		FRUIT_AND_VEG =         "Fruit_and_Veg",
		ANIMAL_MEAT =           "Animal_Meat",
		LIVE_ANIMALS =          "Live_Animals",
		LIQUOR =                "Liquor",
		GRAIN =                 "Grain",
		TEXTILES =              "Textiles",
		FERTILIZER =            "Fertilizer",
		WATER =                 "Water",
		MEDICINES =             "Medicines",
		CONSUMER_GOODS =        "Consumer_goods",
		COMPUTERS =             "Computers",
		ROBOTS =                "Robots",
		PRECIOUS_METALS =       "Precious_metals",
		INDUSTRIAL_MACHINERY =  "Industrial_machinery",
		FARM_MACHINERY =        "Farm_machinery",
		MINING_MACHINERY =      "Mining_machinery",
		AIR_PROCESSORS =        "Air_processors",
		SLAVES =                "Slaves",
		HAND_WEAPONS =          "Hand_weapons",
		BATTLE_WEAPONS =        "Battle_weapons",
		NERVE_GAS =             "Nerve_Gas",
		NARCOTICS =             "Narcotics",
		MILITARY_FUEL =         "Military_fuel",
		RUBBISH =               "Rubbish",
		RADIOACTIVES =          "Radioactive_waste",
	}

	function updateCargoListWidget ()

		local cargolistbox = ui:VBox(10)
		local rowspec = {2,10,3,6}

		for i = 1,#Constants.EquipType do
			local type = Constants.EquipType[i]
			if type ~= "NONE" then
				local et = EquipDef[type]
				local slot = et.slot
				if slot == "CARGO" then
					local count = Game.player:GetEquipCount(slot, type)
					if count > 0 then
						local jettisonButton = SmallLabeledButton.New(l.JETTISON)
						jettisonButton.button.onClick:Connect(function ()
							Game.player:Jettison(type)
							updateCargoListWidget()
							cargoListWidget:SetInnerWidget(updateCargoListWidget())
						end)
						cargolistbox:PackEnd(ui:Grid(rowspec,1):SetRow(0, {
							ui:Align("LEFT", ui:Image("icons/goods/"..equipIcon[type]..".png") or ""),
							ui:Label(et.name),
							ui:Label(count.."t"),
							jettisonButton.widget,
						}))
					end
				end
			end
		end

		-- Function returns a UI with which to populate the cargo list widget
		return
			ui:VBox(10):PackEnd({
				ui:Label(l.CARGO):SetFont("HEADING_LARGE"):SetColor({ r = 1.0, g = 1.0, b = 0.0 }),
				ui:HBox(5):PackEnd({
					ui:Align("LEFT", l.CARGO_SPACE..":"),
					ui:HBox(10):PackEnd({
						ui:Align("MIDDLE",
							ui:HBox(10):PackEnd({
								cargoGauge,
							})
						),
						ui:VBox():PackEnd({
							cargoUsedLabel,
							cargoFreeLabel,
						}):SetFont("XSMALL"),
					}),
					ui:Margin(10),
				}),
			ui:Expand():SetInnerWidget(ui:Scroller():SetInnerWidget(cargolistbox))
			})
	end

	cargoListWidget:SetInnerWidget(updateCargoListWidget())

	local fuelGauge = InfoGauge.New({
		label          = ui:NumberLabel("PERCENT"),
		warningLevel   = 0.1,
		criticalLevel  = 0.05,
		levelAscending = false,
	})
	fuelGauge.label:Bind("valuePercent", Game.player, "fuel")
	fuelGauge.gauge:Bind("valuePercent", Game.player, "fuel")

	-- Define the refuel button
	local refuelButton = SmallLabeledButton.New(l.REFUEL)

	local refuelButtonRefresh = function ()
		if Game.player.fuel == 100 or Game.player:GetEquipCount('CARGO', 'WATER') == 0 then refuelButton.widget:Disable() end
		local fuel_percent = Game.player.fuel/100
		fuelGauge.gauge:SetValue(fuel_percent)
		fuelGauge.label:SetValue(fuel_percent)
	end
	refuelButtonRefresh()

	local refuel = function ()
		-- UI button where the player clicks to refuel...
		Game.player:Refuel(1)
		-- ...then we update the cargo list widget...
		cargoListWidget:SetInnerWidget(updateCargoListWidget())

		refuelButtonRefresh()
	end

	refuelButton.button.onClick:Connect(refuel)

	return ui:Expand():SetInnerWidget(
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Margin(5, "HORIZONTAL",
					ui:VBox(10):PackEnd({
						ui:HBox():PackEnd({
							ui:Label(l.CASH..": "):SetFont("HEADING_LARGE"):SetColor({ r = 1.0, g = 1.0, b = 0.0 }),
							ui:Label(string.format("$%.2f", cash)):SetFont("HEADING_LARGE")
						}),
						ui:Grid({50,50},1):SetRow(0, {
							ui:HBox(5):PackEnd({
								ui:Align("LEFT", l.CABINS..":"),
							}),
							ui:HBox(5):PackEnd({
								ui:HBox(10):PackEnd({
									ui:Align("MIDDLE",
										ui:HBox(10):PackEnd({
											cabinGauge,
										})
									),
									ui:VBox():PackEnd({
										cabinUsedLabel,
										cabinFreeLabel,
									}):SetFont("XSMALL"),
								}),
								ui:Margin(10),
							}),
						}),
						ui:Grid({50,10,40},1):SetRow(0, {
							ui:HBox(5):PackEnd({
								ui:Label(l.FUEL..":"),
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
