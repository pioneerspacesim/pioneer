-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Character = import("Character")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");

local personalInfo = function ()
	local player = Character.persistent.player
	local faceFlags = { player.female and "FEMALE" or "MALE" }

	-- for updating the caption
	local faceWidget = InfoFace.New(player)
	-- for updating the entire face
	local faceWidgetContainer = ui:Margin(0, "ALL", faceWidget)

	local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
	nameEntry.onChange:Connect(function (newName)
		player.name = newName
        faceWidget:UpdateInfo(player)
	end )

	local genderToggle = SmallLabeledButton.New(l.TOGGLE_MALE_FEMALE)
	genderToggle.button.onClick:Connect(function ()
		player.female = not player.female
		faceWidget = InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	local generateFaceButton = SmallLabeledButton.New(l.MAKE_NEW_FACE)
	generateFaceButton.button.onClick:Connect(function ()
		player.seed = Engine.rand:Integer()
		faceWidget = InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Label(l.COMBAT):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ l.RATING, l[player:GetCombatRating()] },
						{ l.KILLS,  string.format('%d',player.killcount) },
					}),
					"",
					ui:Label(l.REPUTATION):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ "Status:", l[player:GetReputationRating()] },
					}),
					"",
					ui:Label(l.MILITARY):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ l.ALLEGIANCE, l.NONE }, -- XXX
						{ l.RANK,      l.NONE }, -- XXX
					})
				})
			})
			:SetColumn(2, {
				ui:VBox(10)
					:PackEnd(ui:HBox(10):PackEnd({
						ui:VBox(5):PackEnd({
							ui:Expand("HORIZONTAL", nameEntry),
						}),
						ui:VBox(5):PackEnd({
							genderToggle,
							generateFaceButton,
						})
					}))
					:PackEnd(ui:Expand("BOTH", faceWidgetContainer))
			})
end

return personalInfo
