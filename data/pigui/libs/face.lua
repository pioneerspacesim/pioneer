-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = require 'Engine'
local FaceTextureGenerator = require 'PiGui.Modules.Face'
local Character = require 'Character'

local l = require 'Lang'.GetResource("ui-core")
local ui = require 'pigui'
local Vector2 = _G.Vector2
local Color = _G.Color

local colors = ui.theme.colors
local icons = ui.theme.icons
local orbiteer = ui.fonts.orbiteer
local noSavedSettings = ui.WindowFlags {"NoSavedSettings"}
local charInfoFlags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar", "NoSavedSettings", "NoScrollWithMouse"}

local ensureCharacter = function (character)
	if not (character and (type(character)=='table') and getmetatable(character) and (getmetatable(character).class == 'Character'))
	then
		return Character.New(character)
	end

	return character
end

local function rerollFaceDesc(oldDesc)
	return {
		--Fit Integer by 2 into an signed int
		FEATURE_SPECIES = Engine.rand:Integer() % 2^31,
		FEATURE_RACE = Engine.rand:Integer() % 2^31,
		FEATURE_GENDER = oldDesc.FEATURE_GENDER or 0,
		FEATURE_HEAD = Engine.rand:Integer() % 2^31,
		FEATURE_EYES = Engine.rand:Integer() % 2^31,
		FEATURE_NOSE = Engine.rand:Integer() % 2^31,
		FEATURE_MOUTH = Engine.rand:Integer() % 2^31,
		FEATURE_HAIRSTYLE = Engine.rand:Integer() % 2^31,
		FEATURE_ACCESSORIES = Engine.rand:Integer() % 4 == 0 and Engine.rand:Integer() % 2^31 or 0, -- accessory on one out of four faces
		FEATURE_CLOTHES = Engine.rand:Integer() % 2^31,
		FEATURE_ARMOUR = oldDesc.FEATURE_ARMOUR or 0,
	}
end

local PiGuiFace = {}

function PiGuiFace.New (character, style, drawButtons)
	character = ensureCharacter(character)
	style = style or {}
	character.faceDescription = character.faceDescription or rerollFaceDesc {
		FEATURE_GENDER = character.female and 1 or 0,
		FEATURE_ARMOUR = character.armour and 1 or 0,
	}

	local faceTexGen = FaceTextureGenerator.New(character.faceDescription, character.seed)
	local piguiFace = {
		faceGen = faceTexGen,
		character = character,
		drawButtons = drawButtons,
		style = {
			size = style.size or nil,
			itemSpacing = style.itemSpacing or ui.rescaleUI(Vector2(6, 12), Vector2(1600, 900)),
			showCharInfo = style.showCharInfo == nil and true or style.showCharInfo,
			charInfoPadding = style.charInfoPadding or ui.rescaleUI(Vector2(24,24), Vector2(1600, 900)),
			charInfoBgColor = style.bgColor or Color(0,0,0,160),
			nameFont = style.nameFont or orbiteer.heading,
			titleFont = style.titleFont or orbiteer.body,
		}
	}

	piguiFace.style.charInfoHeight = math.ceil(
		piguiFace.style.nameFont.size + (character.title and piguiFace.style.titleFont.size or 0)
		+ piguiFace.style.charInfoPadding.y*2
	)

	setmetatable(piguiFace, {
		__index = PiGuiFace,
		class = "UI.PiGuiFace",
	})

	return piguiFace
end

local faceFeatures = {
	{id = 'FEATURE_SPECIES', icon = icons.sun, tooltip = l.FEATURE_SPECIES},
	{id = 'FEATURE_RACE', icon = icons.planet_grid, tooltip = l.FEATURE_RACE},
	{id = 'FEATURE_GENDER', icon = icons.gender, tooltip = l.FEATURE_GENDER, callback = function(char, value)
		char.female = (value % 2 == 1)
	end},
	{id = 'FEATURE_HEAD', icon = icons.personal, tooltip = l.FEATURE_HEAD},
	{id = 'FEATURE_EYES', icon = icons.view_internal, tooltip = l.FEATURE_EYES},
	{id = 'FEATURE_NOSE', icon = icons.nose, tooltip = l.FEATURE_NOSE},
	{id = 'FEATURE_MOUTH', icon = icons.mouth, tooltip = l.FEATURE_MOUTH},
	{id = 'FEATURE_HAIRSTYLE', icon = icons.hair, tooltip = l.FEATURE_HAIRSTYLE},
	{id = 'FEATURE_ACCESSORIES', icon = icons.accessories, tooltip = l.FEATURE_ACCESSORIES},
	{id = 'FEATURE_CLOTHES', icon = icons.clothes, tooltip = l.FEATURE_CLOTHES},
}

function PiGuiFace:changeFeature(featureId, amt, callback)
	local char = self.character

	char.faceDescription[featureId] = (char.faceDescription[featureId] + amt) % 2^31
	if callback then callback(char, char.faceDescription[featureId]) end
	self.faceGen = FaceTextureGenerator.New(char.faceDescription, char.seed)
end

local pigui = Engine.pigui
local font = ui.fonts.pionillium.medium
local iconSize = Vector2(font.size * 2.3, font.size * 2.3)
local buttonSize = iconSize
local function faceGenButton(self, feature)
	local bg_style = ui.theme.buttonColors.transparent
	local fg_color = ui.theme.colors.grey
	if ui.iconButton(ui.theme.icons.time_backward_1x, buttonSize, "##<" .. feature.id, bg_style, fg_color) then
		self:changeFeature(feature.id, -1, feature.callback)
	end
	ui.sameLine()

	pigui.PushID(feature.id)
	ui.icon(feature.icon, iconSize, colors.white)
	pigui.PopID()
	if pigui.IsItemHovered() then pigui.SetTooltip(feature.tooltip) end

	ui.sameLine()
	if ui.iconButton(ui.theme.icons.time_forward_1x, buttonSize, '##>' .. feature.id, bg_style, fg_color) then
		self:changeFeature(feature.id, 1, feature.callback)
	end

end

local facegenSpacing = Vector2(font.size * 0.3, font.size * 0.3)
local facegenSize = Vector2(buttonSize.x * 2 + iconSize.x + facegenSpacing.x * 2, (buttonSize.y + facegenSpacing.y) * (#faceFeatures + 1) - facegenSpacing.y)
local buttonSpaceSize = Vector2(facegenSpacing.x * 2 + buttonSize.x * 2 + iconSize.x, iconSize.y)
local inputTextPadding = ui.rescaleUI(Vector2(18, 18))

function PiGuiFace:renderFaceGenButtons(can_random)
	local char = self.character
	ui.withStyleVars({ItemSpacing = facegenSpacing}, function()
		local numFeatures = #faceFeatures
		if can_random then numFeatures = numFeatures + 1 end
		facegenSize.y = (buttonSize.y + facegenSpacing.y) * numFeatures - facegenSpacing.y
		ui.child("FaceGen", facegenSize, {'AlwaysAutoResize'}, function()
			for _, v in ipairs(faceFeatures) do
				faceGenButton(self, v)
			end
			if can_random and (ui.iconButton(icons.random, buttonSpaceSize, l.RANDOM_FACE, nil, nil, 0, iconSize)) then
				char.faceDescription = rerollFaceDesc(char.faceDescription)
				self.faceGen = FaceTextureGenerator.New(char.faceDescription, char.seed)
			end
		end)
	end)
end

function PiGuiFace:render()
	local char = self.character
	if not self.drawButtons then
		self:renderFaceDisplay()
		return
	end

	ui.child("CharacterInfoDetails" .. tostring(char), self.style.size or Vector2(0, 0), noSavedSettings, function()
		ui.withFont(self.style.nameFont, function()
			ui.withStyleVars({FramePadding = inputTextPadding}, function()

				ui.pushItemWidth(ui.getColumnWidth())
				local text, entered = ui.inputText("##name", char.name, {})

				if entered then
					char.name = text
				end

			end)
		end)

		local lastPos = ui.getCursorPos()
		local itemSpacing = self.style.itemSpacing

		ui.child("Face", Vector2(ui.getColumnWidth() - (facegenSize.x + itemSpacing.x), 0), {}, function()
			self:renderFaceDisplay()
		end)

		ui.setCursorPos(lastPos)
		ui.sameLine(0, itemSpacing.x)

		self:renderFaceGenButtons(true)

	end)
end

function PiGuiFace.getFaceGenButtonsSize()
	return facegenSize
end

function PiGuiFace:renderFaceDisplay ()
	local lastPos = ui.getCursorPos()
	local region = self.style.size or ui.getContentRegion()
	local size = math.min(region.x, region.y)

	ui.image(self.faceGen.textureId, Vector2(size), Vector2(0.0, 0.0), self.faceGen.textureSize, colors.white)

	if(self.style.showCharInfo) then

		ui.setCursorPos(lastPos + Vector2(0.0, size - self.style.charInfoHeight))
		local styles = {WindowPadding = self.style.charInfoPadding, ItemSpacing = self.style.itemSpacing}

		ui.withStyleColorsAndVars({ChildBg = self.style.charInfoBgColor}, styles, function ()

			ui.child("PlayerInfoDetails", Vector2(size, self.style.charInfoHeight), charInfoFlags, function ()
				ui.withFont(self.style.nameFont.name, self.style.nameFont.size, function()
					ui.text(self.character.name)
				end)

				if self.character.title then
					ui.withFont(self.style.titleFont.name, self.style.titleFont.size, function()
						ui.text(self.character.title)
					end)
				end
			end)

		end)

	end
end

return PiGuiFace
