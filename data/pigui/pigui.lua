-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')
local Event = import("Event")
local ShipDef = import("ShipDef")
local Vector = import("Vector")
local Color = import("Color")
local Lang = import("Lang")

local lui = Lang.GetResource("ui-core");
local lc = Lang.GetResource("core");
local lec = Lang.GetResource("equipment-core");

local utils = import("utils")
local pigui = Engine.pigui

local ui = { }

ui.icons_texture = pigui:LoadTextureFromSVG(pigui.DataDirPath({"icons", "icons.svg"}), 16 * 64, 16 * 64)

function ui.window(name, params, fun)
	pigui.Begin(name, params)
	fun()
	pigui.End()
end

function ui.group(fun)
	pigui.BeginGroup()
	fun()
	pigui.EndGroup()
end

function ui.withFont(name, size, fun)
	pigui.PushFont(name, size)
	fun()
	pigui.PopFont()
end

function ui.withStyleColors(styles, fun)
	for k,v in pairs(styles) do
		pigui.PushStyleColor(k, v)
	end
	fun()
	pigui.PopStyleColor(utils.count(styles))
end

pigui.handlers.INIT = function(progress)
	if pigui.handlers and pigui.handlers.init then
		pigui.handlers.init(progress)
	end
end

pigui.handlers.GAME = function(deltat)
	if pigui.handlers and pigui.handlers.game then
		pigui.handlers.game(deltat)
	end
end

pigui.handlers.MAINMENU = function(deltat)
	if pigui.handlers and pigui.handlers.mainMenu then
		pigui.handlers.mainMenu(deltat)
	end
end

ui.registerHandler = function(name, fun)
	pigui.handlers[name] = fun
end

-- Forward selected functions
ui.screenWidth = pigui.screen_width
ui.screenHeight = pigui.screen_height
ui.setNextWindowPos = pigui.SetNextWindowPos
ui.dummy = pigui.Dummy
ui.sameLine = pigui.SameLine
ui.text = pigui.Text
ui.progressBar = pigui.ProgressBar
ui.CalcTextSize = pigui.CalcTextSize

return ui
