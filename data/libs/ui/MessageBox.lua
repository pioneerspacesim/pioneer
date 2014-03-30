-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local ui = Engine.ui

local MessageBox = {}

local function setupLayerAnim (clickWidget)
	local layer = ui.layer

	local anim = ui:NewAnimation({
		widget = layer,
		type = "IN",
		easing = "LINEAR",
		target = "OPACITY",
		duration = 0.1,
	})
	ui:Animate(anim)

	local clicked = false
	clickWidget.onClick:Connect(function ()
		if clicked then return end
		clicked = true

		anim:Finish()

		ui:Animate({
			widget = layer,
			type = "OUT",
			easing = "LINEAR",
			target = "OPACITY",
			duration = 0.1,
			callback = function () ui:DropLayer() end,
		})
	end)
end

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

	layer:AddShortcut("enter")

	setupLayerAnim(layer)
end

function MessageBox.OK (args)
	if type(args) == 'string' then
		args = { message = args }
	end

	local text = ui:MultiLineText(args.message)

	local okButton = ui:Button("OK")
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

	setupLayerAnim (okButton)
end

return MessageBox
