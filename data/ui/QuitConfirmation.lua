-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")

local ui = Engine.ui
local l = Lang.GetResource("quitconfirmation-core")

local max_flavours = 19

ui.templates.QuitConfirmation = function (args)
	local title        = l.QUIT
	local confirmLabel = l.YES
	local cancelLabel  = l.NO
	local onConfirm    = args.onConfirm
	local onCancel     = args.onCancel

	local message      = string.interp(l["MSG_" .. Engine.rand:Integer(1, max_flavours)],{yes = l.YES, no = l.NO})

	local confirmButton = ui:Button(ui:Label(confirmLabel):SetFont("HEADING_NORMAL"))
	local cancelButton = ui:Button(ui:Label(cancelLabel):SetFont("HEADING_NORMAL"))
	cancelButton.onClick:Connect(onCancel)
	confirmButton.onClick:Connect(onConfirm)

	local dialog =
		ui:ColorBackground(0,0,0,0.5,
			ui:Align("MIDDLE",
				ui:Background(
					ui:Table():SetRowSpacing(10)
						:AddRow(ui:Label(title):SetFont("HEADING_NORMAL"))
						:AddRow(ui:MultiLineText(message))
						:AddRow(ui:Grid(2,1)
									:SetRow(0, {
										ui:Align("LEFT", confirmButton),
										ui:Align("RIGHT", cancelButton),
						}))
				)
			)
		)

	return dialog
end
