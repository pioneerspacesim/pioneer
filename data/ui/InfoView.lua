-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")

local TabGroup = import("ui/TabGroup")

local shipInfo        = import("InfoView/ShipInfo")
local personalInfo    = import("InfoView/PersonalInfo")
local econTrade       = import("InfoView/EconTrade")
local missions        = import("InfoView/Missions")
local crewRoster      = import("InfoView/CrewRoster")
local orbitalAnalysis = import("InfoView/OrbitalAnalysis")

local ui = Engine.ui
local t = Translate:GetTranslator()

local tabGroup
ui.templates.InfoView = function (args)
	if tabGroup then
		tabGroup:SwitchFirst()
		return tabGroup.widget
	end

	tabGroup = TabGroup.New()

	tabGroup:AddTab({ id = "shipInfo",        title = t("Ship Information"),     icon = "Satellite", template = shipInfo         })
	tabGroup:AddTab({ id = "personalInfo",    title = t("Personal Information"), icon = "User",      template = personalInfo     })
	tabGroup:AddTab({ id = "econTrade",       title = t("Economy & Trade"),      icon = "Cart",      template = econTrade,       })
	tabGroup:AddTab({ id = "missions",        title = t("MISSIONS"),             icon = "Star",      template = missions,        })
	tabGroup:AddTab({ id = "crew",            title = t("Crew Roster"),          icon = "Agenda",    template = crewRoster,      })
	--tabGroup:AddTab({ id = "orbitalAnalysis", title = t("Orbital Analysis"),     icon = "Planet",    template = orbitalAnalysis, })

	return tabGroup.widget
end

