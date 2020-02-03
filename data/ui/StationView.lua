-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import("Game")
local Engine = import("Engine")
local Lang = import("Lang")
local Event = import("Event")

local TabView = import("ui/TabView")
local InfoGauge = import("ui/InfoGauge")

local piStationView = import("pigui/views/station-view")

local l = Lang.GetResource("ui-core")

local lobby           = import("ui/StationView/Lobby")
local bulletinBoard   = import("ui/StationView/BulletinBoard")
local commodityMarket = import("ui/StationView/CommodityMarket")
local shipMarket      = import("ui/StationView/ShipMarket")
local equipmentMarket = import("ui/StationView/EquipmentMarket")
local shipRepairs     = import("ui/StationView/ShipRepairs")
local police          = import("ui/StationView/Police")

local ui = Engine.ui

local tabGroup
ui.templates.StationView = function (args)
	if tabGroup then
		tabGroup:SwitchFirst()
		return tabGroup.widget
	end

	tabGroup = TabView.New()

	local player = Game.player

	local function createCashLabel()
		local cashLabel = ui:NumberLabel("MONEY")
		cashLabel:Bind("value", player, "cash")
		return cashLabel
	end

	local function cargoUpdate (fields)
		fields["gauge"]:SetUpperValue(player.totalCargo)
		fields["gauge"]:SetValue(player.usedCargo)
		fields["usedLabel"]:SetText(string.interp(l.CARGO_T_USED, { amount = player.usedCargo }))
		fields["freeLabel"]:SetText(string.interp(l.CARGO_T_FREE, { amount = player.totalCargo-player.usedCargo }))
	end

	local function capacityUpdate (fields)
		local totalCapacity = player.usedCapacity + player.freeCapacity
		fields["gauge"]:SetUpperValue(totalCapacity)
		fields["gauge"]:SetValue(player.usedCapacity)
		fields["usedLabel"]:SetText(string.interp(l.CARGO_T_USED, { amount = player.usedCapacity }))
		fields["freeLabel"]:SetText(string.interp(l.CARGO_T_FREE, { amount = player.freeCapacity }))
	end

	local function cabinUpdate (fields)
		local cap = player.cabin_cap or 0
		local count = player:GetEquipCountOccupied("cabin")
		fields["gauge"]:SetUpperValue(count)
		fields["gauge"]:SetValue(count-cap)
		fields["usedLabel"]:SetText(string.interp(l.CABIN_USED, { amount = count-cap }))
		fields["freeLabel"]:SetText(string.interp(l.CABIN_FREE, { amount = cap}))
	end

	local function createGauge(text, connectTo, funcUpdate)
		local gauge = ui:Gauge()
		local usedLabel = ui:Label("")
		local freeLabel = ui:Label("")
		local fields = {}
		fields["gauge"] = gauge;
		fields["usedLabel"] = usedLabel;
		fields["freeLabel"] = freeLabel;
		local function innerFuncUpdate()
			funcUpdate(fields);
		end
		for k,v in pairs(connectTo) do
			player:Connect(v, innerFuncUpdate)
		end
		innerFuncUpdate()

		return ui:Margin(10, "HORIZONTAL",
			ui:HBox(10):PackEnd({
				ui:Align("MIDDLE",
					ui:HBox(10):PackEnd({
						text..":",
						gauge,
					})
				),
				ui:VBox():PackEnd({
					usedLabel,
					freeLabel,
				}):SetFont("XSMALL"),
			})
		)
	end

	local function createFooter(type)
		type = type or "default"

		local leftGauge = nil
		if type == "default" then
			leftGauge = createGauge(l.CARGO, { "usedCargo", "totalCargo" }, cargoUpdate)
		elseif type == "equip" then
			leftGauge = createGauge(l.CAPACITY, { "usedCapacity", "freeCapacity" }, capacityUpdate)
		end

		return ui:Margin(15, "TOP",
			ui:Margin(5, "VERTICAL",
				ui:Grid({15,30,30,15},1):SetRow(0, {
					ui:Margin(10, "HORIZONTAL",
						ui:HBox():PackEnd({
							l.CASH..": ",
							createCashLabel(),
						})
					),
					leftGauge,
					createGauge(l.CABINS, { "cabin_cap" }, cabinUpdate),
					ui:Margin(10, "HORIZONTAL",
						ui:Align("RIGHT",
							l.LEGAL_STATUS..": "..l.CLEAN
						)
					),
				})
			)
		)
	end

	local footerDefault = createFooter()
	local footerEquip = createFooter("equip")

	tabGroup:AddTab({ id = "lobby",           title = l.LOBBY,            icon = "Info",       template = lobby,           footer = footerDefault})
	tabGroup:AddTab({ id = "bulletinBoard",   title = l.BULLETIN_BOARD,   icon = "Clipboard",  template = bulletinBoard,   footer = footerDefault})
	tabGroup:AddTab({ id = "commodityMarket", title = l.COMMODITY_MARKET, icon = "Cart",       template = commodityMarket, footer = footerDefault})
	tabGroup:AddTab({ id = "shipMarket",      title = l.SHIP_MARKET,      icon = "Rocketship", template = shipMarket,      footer = footerDefault})
	tabGroup:AddTab({ id = "equipmentMarket", title = l.EQUIPMENT_MARKET, icon = "Radio",      template = equipmentMarket, footer = footerEquip})
	tabGroup:AddTab({ id = "shipRepairs",     title = l.SHIP_REPAIRS,     icon = "Tools",      template = shipRepairs,     footer = footerDefault})
	tabGroup:AddTab({ id = "police",          title = l.POLICE,           icon = "Shield",     template = police,          footer = footerDefault})

	tabGroup:SetFooter(footerDefault)

	tabGroup.header:Hide()
	tabGroup.outerBody:Hide()
	piStationView.legacyTabView = tabGroup
	return tabGroup.widget
end

Event.Register("onGameEnd", function ()
	tabGroup = nil
end)
