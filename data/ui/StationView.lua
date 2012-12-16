-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")

local TabGroup = import("ui/TabGroup")

local lobby           = import("StationView/Lobby")
local shipyard        = import("StationView/Shipyard")
local commodityMarket = import("StationView/CommodityMarket")
local bulletinBoard   = import("StationView/BulletinBoard")
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
	tabGroup:AddTab({ id = "shipyard",        title = "Shipyard",         icon = "Tools",     template = shipyard        })
	tabGroup:AddTab({ id = "commodityMarket", title = "Commodity Market", icon = "Cart",      template = commodityMarket })
	tabGroup:AddTab({ id = "bulletinBoard",   title = "Bulletin Board",   icon = "Clipboard", template = bulletinBoard   })
	tabGroup:AddTab({ id = "police",          title = "Police",           icon = "Shield",    template = police          })

	return tabGroup.widget
end

