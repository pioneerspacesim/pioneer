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
		ui:Grid({1,3,1}, {1,3,1})
			:SetCell(1,1,
				ui:Background(ui:VBox(10)
					:PackEnd(ui:Label(title):SetFont("HEADING_NORMAL"))
					:PackEnd(ui:Label(message))
					:PackEnd(okButton)
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

	local innerWas = ui.innerWidget
	ui:SetInnerWidget(
		ui.templates.ErrorScreen({
			title    = title,
			message  = message,
			onOk     = function () ui:SetInnerWidget(innerWas) end,
		})
	)
end

return ErrorScreen
