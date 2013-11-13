-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = import("Game")
local Format = import("Format")
local Engine = import("Engine")
local ShipDef = import("ShipDef")

local TabGroup = import("ui/TabGroup")
local InfoGauge = import("ui/InfoGauge")

local lobby           = import("StationView/Lobby")
local bulletinBoard   = import("StationView/BulletinBoard")
local commodityMarket = import("StationView/CommodityMarket")
local shipMarket      = import("StationView/ShipMarket")
local equipmentMarket = import("StationView/EquipmentMarket")
local shipRepairs     = import("StationView/ShipRepairs")
local police          = import("StationView/Police")

local ui = Engine.ui

local tabGroup
ui.templates.StationView = function (args)
	if tabGroup then
		tabGroup:SwitchFirst()
		return tabGroup.widget
	end

	tabGroup = TabGroup.New()

	tabGroup:AddTab({ id = "lobby",           title = "Lobby",            icon = "Info",      template = lobby           })
	tabGroup:AddTab({ id = "bulletinBoard",   title = "Bulletin Board",   icon = "Clipboard", template = bulletinBoard   })
	tabGroup:AddTab({ id = "commodityMarket", title = "Commodity Market", icon = "Cart",      template = commodityMarket })
	tabGroup:AddTab({ id = "shipMarket",      title = "Ship Market",      icon = "Car",       template = shipMarket      })
	tabGroup:AddTab({ id = "equipmentMarket", title = "Equipment Market", icon = "Radio",     template = equipmentMarket })
	tabGroup:AddTab({ id = "shipRepairs",     title = "Ship repairs",     icon = "Tools",     template = shipRepairs     })
	tabGroup:AddTab({ id = "police",          title = "Police",           icon = "Shield",    template = police          })

	local player = Game.player

	local footer =
		ui:Margin(5, "VERTICAL",
			ui:Grid({15,30,30,15},1):SetRow(0, {
				ui:Margin(10, "HORIZONTAL",
					"Cash: "..Format.Money(player:GetMoney())
				),
				ui:Margin(10, "HORIZONTAL",
					ui:HBox():PackEnd({
						"Cargo: ",
						InfoGauge.New({
							formatter = function (v)
								return string.format("%d/%dt", player.usedCargo, ShipDef[player.shipId].capacity-player.usedCapacity+player.usedCargo)
							end
						}),
					})
				),
				ui:Margin(10, "HORIZONTAL",
					ui:HBox():PackEnd({
						"Cabins: ",
						InfoGauge.New({
							formatter = function (v)
								local occupied   = player:GetEquipCount("CABIN", "PASSENGER_CABIN")
								local unoccupied = player:GetEquipCount("CABIN", "UNOCCUPIED_CABIN")
								return string.format("%d/%d", occupied, unoccupied+occupied)
							end
						}),
					})
				),
				ui:Margin(10, "HORIZONTAL",
					ui:Align("RIGHT",
						"Legal status: Clean"
					)
				),
			})
		)

	tabGroup:SetFooter(footer)

	return tabGroup.widget
end

