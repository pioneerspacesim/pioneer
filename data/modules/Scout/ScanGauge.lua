-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game   = require 'Game'
local Lang   = require 'Lang'
local ui     = require 'pigui'

local gauges = require 'pigui.modules.flight-ui.gauges'

local icons = ui.theme.icons
local colors = ui.theme.colors

local ls = Lang.GetResource('module-scout')

gauges.registerGauge(10, {
	value = function()
		local scanMgr = Game.player:GetComponent("ScanManager")

		local scan = scanMgr and scanMgr:GetActiveScan()
		if not scan then return nil end

		local completion = scan.coverage / scan.targetCoverage
		return completion * 100.0
	end,
	unit = '%', format = '%.2f', min = 0, max = 100,
	icon = icons.scanner, color = colors.gaugeScanner,
	tooltip = ls.HUD_SCAN_PROGRESS
})
