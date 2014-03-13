-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local ui = Engine.ui

local MessageBox = {}

function MessageBox.Message (args)
	if type(args) == 'string' then
		args = { message = args }
	end

	local text = ui:MultiLineText(args.message)

	local layer = ui:NewLayer(
		ui:ColorBackground(0,0,0,0.5,
			ui:Align("MIDDLE",
				ui:Background(
					text
				)
			)
		)
	)

	layer.onClick:Connect(function () ui:DropLayer() end)
	layer:AddShortcut("enter")
end

function MessageBox.OK (args)
	if type(args) == 'string' then
		args = { message = args }
	end

	local text = ui:MultiLineText(args.message)

	local okButton = ui:Button("OK")
	okButton.onClick:Connect(function ()
		ui:DropLayer()
	end)
	okButton:AddShortcut("enter")

	ui:NewLayer(
		ui:ColorBackground(0,0,0,0.5,
			ui:Align("MIDDLE",
				ui:Background(
					ui:VBox(10)
						:PackEnd(ui:Align("MIDDLE", text))
						:PackEnd(ui:Align("MIDDLE", okButton))
				)
			)
		)
	)
end

return MessageBox
