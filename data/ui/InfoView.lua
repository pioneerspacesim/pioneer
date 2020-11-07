-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local Lang = require 'Lang'
local Event = require 'Event'

local TabView = require 'ui.TabView'

local shipInfo        = require 'ui.InfoView.ShipInfo'
local econTrade       = require 'ui.InfoView.EconTrade'
local missions        = require 'ui.InfoView.Missions'
local crewRoster      = require 'ui.InfoView.CrewRoster'
local orbitalAnalysis = require 'ui.InfoView.OrbitalAnalysis'

local piInfoView = require 'pigui.views.info-view'

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local tabGroup
ui.templates.InfoView = function (args)
	if tabGroup then
		tabGroup:SwitchFirst()
		return tabGroup.widget
	end

	tabGroup = TabView.New()

	tabGroup:AddTab({ id = "shipInfo",        title = l.SHIP_INFORMATION,     icon = "Satellite", template = shipInfo         })
	tabGroup:AddTab({ id = "econTrade",       title = l.ECONOMY_TRADE,      icon = "Cart",      template = econTrade,       })
	tabGroup:AddTab({ id = "missions",        title = l.MISSIONS,             icon = "Star",      template = missions,        })
	tabGroup:AddTab({ id = "crew",            title = l.CREW_ROSTER,          icon = "Agenda",    template = crewRoster,      })
	--tabGroup:AddTab({ id = "orbitalAnalysis", title = l.ORBITAL_ANALYSIS,     icon = "Planet",    template = orbitalAnalysis, })

	tabGroup.header:Hide()
	tabGroup.outerBody:Hide()
	piInfoView.legacyTabView = tabGroup
	return tabGroup.widget
end

Event.Register("onGameEnd", function ()
	tabGroup = nil
end)
