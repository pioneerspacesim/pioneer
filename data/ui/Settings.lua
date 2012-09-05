-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")
local Lang = import("Lang")

local ui = Engine.ui
local l = Lang.GetDictionary()

local return_to_menu = ui:Button():SetInnerWidget(ui:Label(l.RETURN_TO_MENU))
return_to_menu.onClick:Connect(function () ui:SetInnerWidget(ui.templates.MainMenu()) end)

local settings =
	ui:Background():SetInnerWidget(ui:Margin(30):SetInnerWidget(
		ui:VBox(10):PackEnd({
			ui:Label(
				"Sorry, the settings page is undergoing rennovation work at the moment.\n" ..
				"Please edit settings in ~/.pioneer/config.ini"),
			ui:HBox(5):PackEnd({ui:CheckBox(), ui:Label("Test")}),
			return_to_menu
		})
	))

ui.templates.Settings = function (args) return settings end
