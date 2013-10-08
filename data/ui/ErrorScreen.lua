-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")

local ui = Engine.ui
local t = Translate:GetTranslator()

ui.templates.ErrorScreen = function (args)
	local title     = args.title    or t("Error")
	local message   = args.message  or t("An error has occurred")
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
		title = t('Error')
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
