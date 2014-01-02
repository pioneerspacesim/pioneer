-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

ui.templates.ErrorScreen = function (args)
	local title     = args.title    or l.ERROR
	local message   = args.message  or l.AN_ERROR_HAS_OCCURRED
	local onOk      = args.onOk     or function (name) end

	local okButton = ui:Button(ui:Label("Ok"):SetFont("HEADING_NORMAL"))
	okButton.onClick:Connect(onOk)

	local dialog =
		ui:ColorBackground(0,0,0,0.5,
			ui:Align("MIDDLE",
				ui:Background(
					ui:Table():SetRowSpacing(10)
						:AddRow(ui:Label(title):SetFont("HEADING_NORMAL"))
						:AddRow(message)
						:AddRow(okButton)
				)
			)
		)

	return dialog
end

local ErrorScreen = {}

ErrorScreen.ShowError = function (title, message)
	if message == nil then
		message = title
		title = l.ERROR
	end

	ui:NewLayer(
		ui.templates.ErrorScreen({
			title    = title,
			message  = message,
			onOk     = function () ui:DropLayer() end,
		})
	)
end

return ErrorScreen
