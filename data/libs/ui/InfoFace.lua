local ui = Engine.ui

local testCharacter = function (character)
	if not (character and (type(character)=='table') and (getmetatable(character).class == 'Character'))
	then
		error ('Character object expected')
	end
end

local setFaceInfo = function (face, character)
	face:SetInnerWidget(
		ui:Align("BOTTOM_LEFT"):SetInnerWidget(
			ui:Expand("HORIZONTAL"):SetInnerWidget(
				ui:Gradient({r=0.1,g=0.1,b=0.1,a=0.8}, {r=0.0,g=0.0,b=0.1,a=0.0}, "HORIZONTAL"):SetInnerWidget(
					ui:Margin(20):SetInnerWidget(ui:VBox():PackEnd({
						ui:Label(character.name):SetFont("HEADING_NORMAL"),
						ui:Label((character.title or '')):SetFont("HEADING_SMALL"),
					}))
				)
			)
		)
	)
end

UI.InfoFace = {

New = function (character)
	testCharacter(character)

	local faceFlags = {
		character.female and "FEMALE" or "MALE",
		character.armor and "ARMOUR",
	}

	local self = {
		widget = UI.Game.Face.New(ui, faceFlags, character.seed)
	}

	setFaceInfo(self.widget, character)

	setmetatable(self, {
		__index = UI.InfoFace,
		class = "UI.InfoFace",
	})

	return self
end,

UpdateInfo = function (self, character)
	testCharacter(character)
	setFaceInfo(self.widget, character)
end,

}
