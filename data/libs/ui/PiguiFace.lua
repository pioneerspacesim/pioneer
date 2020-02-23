-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local FaceTextureGenerator = require 'PiGui.Modules.Face'
local Character = require 'Character'
local ui = require 'pigui'
local colors = ui.theme.colors
local orbiteer = ui.fonts.orbiteer
local charInfoFlags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar"}

local ensureCharacter = function (character)
	if not (character and (type(character)=='table') and getmetatable(character) and (getmetatable(character).class == 'Character'))
	then
		return Character.New(character)
	end

	return character
end

local PiGuiFace = {}

function PiGuiFace.New (character, style)
	character = ensureCharacter(character)
	style = style or {}
	local faceDesc = character.faceDescription and character.faceDescription or {
		FEATURE_GENDER = character.female and 1 or 0,
		FEATURE_ARMOUR = character.armour and 1 or 0,
	}

	local faceTexGen = FaceTextureGenerator.New(faceDesc, character.seed)
	local piguiFace = {
		faceTextureGenerator = faceTexGen,
		character = character,
		texture = {
			id = faceTexGen.textureId,
			uv = faceTexGen.textureSize,
		},
		style = {
			size = style.size or ui.rescaleUI(Vector2(320, 320), Vector2(1600, 900)),
			windowPadding = style.windowPadding or Vector2(0,0),
			itemSpacing = style.itemSpacing or Vector2(0,0),
			showCharInfo = style.showCharInfo == nil and true or style.showCharInfo,
			charInfoPadding = style.charInfoPadding or ui.rescaleUI(Vector2(24,24), Vector2(1600, 900)),
			charInfoBgColor = style.bgColor or Color(0,0,0,160),
			nameFont = style.nameFont or orbiteer.xlarge,
			titleFont = style.titleFont or orbiteer.large,
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

function PiGuiFace:render ()
	ui.image(self.texture.id, self.style.size, Vector2(0.0, 0.0), self.texture.uv, colors.white)

	if(self.style.showCharInfo) then
		local lastPos = ui.getCursorPos()
		ui.setCursorPos(lastPos - Vector2(0.0, self.style.charInfoHeight + self.style.itemSpacing.y))
		ui.withStyleColorsAndVars({ChildWindowBg = self.style.charInfoBgColor}, {WindowPadding = self.style.charInfoPadding, ItemSpacing = self.style.itemSpacing}, function ()
			ui.child("PlayerInfoDetails", Vector2(self.style.size.x, self.style.charInfoHeight), charInfoFlags, function ()
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
		ui.setCursorPos(lastPos)
	end
end

return PiGuiFace
