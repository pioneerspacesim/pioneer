-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")

local ui = Engine.ui

local InfoGauge = {}

function InfoGauge.New (args)
	args = args or {}

	local formatter = args.formatter or function (v) return string.format("%.2f / 1.00", v) end
	local label = args.label or ui:Label(formatter(0))

	local warningLevel = args.warningLevel
	local criticalLevel = args.criticalLevel
	local levelAscending = args.levelAscending

	local gauge = ui:Gauge()
	local box = ui:HBox(5):PackEnd({ gauge, label })

	if warningLevel   ~= nil then gauge:SetWarningLevel(warningLevel) end
	if criticalLevel  ~= nil then gauge:SetCriticalLevel(criticalLevel) end
	if levelAscending ~= nil then gauge:SetLevelAscending(levelAscending) end

    local self = {
        widget    = box,
		gauge     = gauge,
		label     = label,
        formatter = formatter,
    }

	setmetatable(self, {
		__index = InfoGauge,
		class = "UI.InfoGauge",
	})

	return self
end

function InfoGauge.SetValue (self, v)
	self.gauge:SetValue(v)
	self.label:SetText(self.formatter(v))
end

return InfoGauge
