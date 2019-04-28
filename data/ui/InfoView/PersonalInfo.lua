-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Character = import("Character")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local l = Lang.GetResource("ui-core");
local currentFeature = 1
local featureCount = 11
local features = {
	'FEATURE_SPECIES',
	'FEATURE_RACE',
	'FEATURE_GENDER',
	'FEATURE_HEAD',
	'FEATURE_EYES',
	'FEATURE_NOSE',
	'FEATURE_MOUTH',
	'FEATURE_HAIRSTYLE',
	'FEATURE_ACCESSORIES',
	'FEATURE_CLOTHES',
	'FEATURE_ARMOUR',
}

local personalInfo = function ()
	local player = Character.persistent.player

	-- for updating the caption
	local faceWidget = InfoFace.New(player)
	-- for updating the entire face
	local faceWidgetContainer = ui:Margin(0, "ALL", faceWidget)

	local nameEntry = ui:TextEntry(player.name):SetFont("HEADING_LARGE")
	nameEntry.onChange:Connect(function (newName)
		player.name = newName
		faceWidget:UpdateInfo(player)
	end )

	local featureLabel = ui:Margin(0,"TOP", ui:Label(l[features[currentFeature]]):SetFont("HEADING_LARGE"))

	function changeFeatureType (idx)
		currentFeature = (currentFeature + idx) % (featureCount+1)
		if currentFeature < 1 and idx > 0 then currentFeature = 1
		elseif currentFeature < 1 and idx < 0 then currentFeature = featureCount end

		featureLabel:SetInnerWidget(ui:Label(l[features[currentFeature]]):SetFont("HEADING_LARGE"))
	end

	function changeFeature (idx)
		if player.faceDescription == nil or idx == 0 then
			player.faceDescription = {
				--Fit Integer by 2 into an signed int
				FEATURE_SPECIES = Engine.rand:Integer() % 2^31,
				FEATURE_RACE = Engine.rand:Integer() % 2^31,
				FEATURE_GENDER = Engine.rand:Integer() % 2^31,
				FEATURE_HEAD = Engine.rand:Integer() % 2^31,
				FEATURE_EYES = Engine.rand:Integer() % 2^31,
				FEATURE_NOSE = Engine.rand:Integer() % 2^31,
				FEATURE_MOUTH = Engine.rand:Integer() % 2^31,
				FEATURE_HAIRSTYLE = Engine.rand:Integer() % 2^31,
				FEATURE_ACCESSORIES = Engine.rand:Integer() % 2^31,
				FEATURE_CLOTHES = Engine.rand:Integer() % 2^31,
				FEATURE_ARMOUR = 0,
			}
		end

		player.faceDescription[features[currentFeature]] = (player.faceDescription[features[currentFeature]] + idx) % 2^31

		faceWidget = InfoFace.New(player)
		faceWidgetContainer:SetInnerWidget(faceWidget.widget)
	end

	local prevFeatureTypeButton = ui:Button("<"):SetFont("XSMALL")
	prevFeatureTypeButton.onClick:Connect(function () changeFeatureType(-1) end)

	local nextFeatureTypeButton = ui:Button(">"):SetFont("XSMALL")
	nextFeatureTypeButton.onClick:Connect(function () changeFeatureType(1) end)

	local prevFeatureButton = ui:Button(l.PREVIOUS_FACE_FEATURE):SetFont("SMALL")
	prevFeatureButton.onClick:Connect(function () changeFeature(-1) end)

	local nextFeatureButton = ui:Button(l.NEXT_FACE_FEATURE):SetFont("SMALL")
	nextFeatureButton.onClick:Connect(function () changeFeature(1) end)

	local randomFaceButton = ui:Button(l.RANDOM_FACE):SetFont("SMALL")
	randomFaceButton.onClick:Connect(function () changeFeature(0) end)

	if(player.faceDescription == nil) then changeFeature(0) end

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
						{ l.STATUS..":", l[player:GetReputationRating()] },
					})
				})
			})
			:SetColumn(2, {
				ui:VBox(10)
					:PackEnd(ui:Expand("HORIZONTAL", nameEntry))
					:PackEnd(ui:VBox(5):PackEnd({
						ui:HBox(10):PackEnd({
							ui:HBox(10):PackEnd({
								ui:Margin(4,"TOP", ui:Label(l.CHANGE):SetFont("HEADING_LARGE")),
								prevFeatureTypeButton,
								ui:HBox(10):PackEnd({
									ui:Expand("HORIZONTAL", ui:Align("MIDDLE", featureLabel)),
								}),
								nextFeatureTypeButton,
							}),
							ui:HBox(10):PackEnd({
								prevFeatureButton,
								nextFeatureButton,
								randomFaceButton,
							})
						})
					}))
					:PackEnd(ui:Expand("BOTH", faceWidgetContainer))
			})
end

return personalInfo
