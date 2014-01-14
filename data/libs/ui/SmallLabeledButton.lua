-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")

local ui = Engine.ui

local SmallLabeledButton = {}

function SmallLabeledButton.New (text)
	local self = {
		button = ui:SmallButton(),
		label  = ui:Label(text),
	}
	self.widget = ui:HBox(10):PackEnd({ self.button, self.label })

	setmetatable(self, {
		__index = SmallLabeledButton,
		class = "UI.SmallLabeledButton",
	})

	return self
end

return SmallLabeledButton
