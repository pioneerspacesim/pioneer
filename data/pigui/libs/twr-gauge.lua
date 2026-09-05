-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Shared drawing helpers for the thrust-to-weight ratio (TWR) gauge, so that
-- the same look can be reused by the bottom-right mass/TWR widget and by the
-- single readout on the flight HUD.

local ui = require 'pigui'
local Vector2 = _G.Vector2

local colors = ui.theme.colors

local TwrGauge = {}

-- TWR at the top of the bar (values above this max the bar out)
TwrGauge.MAX_LEVEL = 2.0
-- Below this the fill turns to the "warning" colour
TwrGauge.WARNING_LEVEL = 1.5
-- Below this (i.e. can't lift off) the fill turns to the "danger" colour
TwrGauge.DANGER_LEVEL = 1.0
-- A TWR at or below this blinks
TwrGauge.BLINK_LEVEL = 0.95
TwrGauge.BLINK_PERIOD = 0.8
-- Phase angle for the HUD circular indicator arc centre (6 o'clock).
TwrGauge.HUD_ARC_PHASE = math.pi / 2

-- Text colour for a TWR value. math.huge means infinite (ample) thrust.
function TwrGauge.GetTextColor(twr)
	if twr == nil or twr == math.huge then
		return colors.font
	end
	if twr > TwrGauge.WARNING_LEVEL then
		return colors.font
	elseif twr >= TwrGauge.DANGER_LEVEL then
		return colors.alertYellow
	elseif twr > TwrGauge.BLINK_LEVEL then
		return colors.alertBrightRed
	elseif math.fmod(ui.getTime(), TwrGauge.BLINK_PERIOD) < TwrGauge.BLINK_PERIOD / 2 then
		return colors.alertBrightRed
	end
	return colors.darkGrey
end

-- Arc fill fraction (0..1) for the circular HUD indicator; maxes at MAX_LEVEL.
function TwrGauge.GetHudCircleFraction(twr)
	if twr == nil or twr == math.huge then
		return 1
	end
	return math.clamp(twr / TwrGauge.MAX_LEVEL, 0, 1)
end

-- Outer-ring arc colour for the circular HUD indicator.
function TwrGauge.GetHudArcColor(twr)
	if twr == nil or twr == math.huge or twr > TwrGauge.WARNING_LEVEL then
		return colors.gaugeShield
	elseif twr > TwrGauge.DANGER_LEVEL then
		return colors.alertYellow
	end
	return colors.alertRed
end

-- Draw the sectioned TWR bar (background + coloured fill) into the rectangle
-- [pos, pos + size], both in screen coordinates. A nil twr is treated as
-- infinite and fills the bar completely.
function TwrGauge.DrawBar(pos, size, twr)
	local bar_width = size.x
	local height = size.y

	-- The background has coloured sections for the different TWR levels
	local danger_width = bar_width * TwrGauge.DANGER_LEVEL / TwrGauge.MAX_LEVEL
	local warning_width = bar_width * TwrGauge.WARNING_LEVEL / TwrGauge.MAX_LEVEL

	-- A TWR of math.huge is infinite, so the bar maxes out
	local fraction = (twr == nil or twr == math.huge) and TwrGauge.MAX_LEVEL or math.clamp(twr, 0, TwrGauge.MAX_LEVEL)
	local readout_width = bar_width * fraction / TwrGauge.MAX_LEVEL

	-- Draw the background
	ui.addRectFilled(pos, pos + Vector2(danger_width, height), colors.gaugeBarBackground * 0.8, 0, ui.RoundCornersNone)
	ui.addRectFilled(pos + Vector2(danger_width, 0), pos + Vector2(warning_width, height), colors.gaugeBarBackground * 0.9, 0, ui.RoundCornersNone)
	ui.addRectFilled(pos + Vector2(warning_width, 0), pos + Vector2(bar_width, height), colors.gaugeBarBackground, 0, ui.RoundCornersNone)

	-- Draw the gauge level
	if fraction > TwrGauge.WARNING_LEVEL then
		ui.addRectFilled(pos, pos + Vector2(danger_width, height), colors.gaugeTWR * 0.8, 0, ui.RoundCornersNone)
		ui.addRectFilled(pos + Vector2(danger_width, 0), pos + Vector2(warning_width, height), colors.gaugeTWR * 0.9, 0, ui.RoundCornersNone)
		ui.addRectFilled(pos + Vector2(warning_width, 0), pos + Vector2(readout_width, height), colors.gaugeTWR, 0, ui.RoundCornersNone)
	elseif fraction > TwrGauge.DANGER_LEVEL then
		ui.addRectFilled(pos, pos + Vector2(danger_width, height), colors.gaugeTWRLow * 0.9, 0, ui.RoundCornersNone)
		ui.addRectFilled(pos + Vector2(danger_width, 0), pos + Vector2(readout_width, height), colors.gaugeTWRLow, 0, ui.RoundCornersNone)
	elseif fraction > 0 then
		ui.addRectFilled(pos, pos + Vector2(readout_width, height), colors.gaugeTWRVeryLow, 0, ui.RoundCornersNone)
	end
end

return TwrGauge
