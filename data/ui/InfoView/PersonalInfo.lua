-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")
local Engine = import("Engine")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local t = Translate:GetTranslator()

local personalInfo = function ()
	local player = PersistentCharacters.player
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

	local genderToggle = SmallLabeledButton.New(t("Toggle male/female"))
	genderToggle.button.onClick:Connect(function ()
		player.female = not player.female
		faceWidget = InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	local generateFaceButton = SmallLabeledButton.New(t("Make new face"))
	generateFaceButton.button.onClick:Connect(function ()
		player.seed = Engine.rand:Integer()
		faceWidget = InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end)

	return
		ui:Grid(2,1)
			:SetColumn(0, {
				ui:Table():AddRows({
					ui:Label(t("Combat")):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("Rating:"), t(player:GetCombatRating()) },
						{ t("Kills:"),  string.format('%d',player.killcount) },
					}),
					"",
					ui:Label(t("Military")):SetFont("HEADING_LARGE"),
					ui:Table():SetColumnSpacing(10):AddRows({
						{ t("ALLEGIANCE"), t('NONE') }, -- XXX
						{ t("Rank:"),      t('NONE') }, -- XXX
					})
				})
			})
			:SetColumn(1, {
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
					:PackEnd(faceWidgetContainer)
			})
end

return personalInfo
