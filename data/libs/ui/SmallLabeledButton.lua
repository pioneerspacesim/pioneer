local ui = Engine.ui

UI.SmallLabeledButton = {

New = function (text,font)
	local self = {
		button = ui:SmallButton(),
		label  = ui:Label(text),
	}
	if font and type(font) == "string" then self.label = ui:Label(text):SetFont(font) end
	self.widget = ui:HBox(10):PackEnd({ self.button, self.label })

	setmetatable(self, {
		__index = UI.SmallLabeledButton,
		class = "UI.SmallLabeledButton",
	})

	return self
end,

}
