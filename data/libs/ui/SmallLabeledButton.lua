-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = Engine.ui

UI.SmallLabeledButton = {

New = function (text)
	local self = {
		button = ui:SmallButton(),
		label  = ui:Label(text),
	}
	self.widget = ui:HBox(10):PackEnd({ self.button, self.label })

	setmetatable(self, {
		__index = UI.SmallLabeledButton,
		class = "UI.SmallLabeledButton",
	})

	return self
end,

}
