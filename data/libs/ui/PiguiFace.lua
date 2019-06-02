-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Face = import 'UI.PiGui.Face'
local ui = import 'pigui/pigui.lua'
local colors = ui.theme.colors
local orbiteer = ui.fonts.orbiteer

local testCharacter = function (character)
	if not (character and (type(character)=='table') and (getmetatable(character).class == 'Character'))
	then
		error ('Character object expected')
	end
end

local getStyleVar = function(style, key, defaultValue)
	return (style and (type(style)=='table')) and style[key] or defaultValue
end

local PiGuiFace = {}

function PiGuiFace.New (character, style)
	testCharacter(character)

	local faceDesc = character.faceDescription and character.faceDescription or {
		FEATURE_GENDER = character.female and 1 or 0,
		FEATURE_ARMOUR = character.armour and 1 or 0,
	}

	local piguiFace = {
		widget = Face.New(faceDesc, character.seed),
		character = character
	}

	piguiFace.texture = {
		id = piguiFace.widget.textureId,
		uv = piguiFace.widget.textureSize
	}

	piguiFace.style = {
		windowPadding = getStyleVar(style, 'windowPadding', Vector2(0,0)),
		itemSpacing = getStyleVar(style, 'itemSpacing', Vector2(0,0)),
		bgColor = getStyleVar(style, 'bgColor', Color(0,0,0,160)),
		nameFont = getStyleVar(style, 'nameFont', orbiteer.xlarge),
		titleFont = getStyleVar(style, 'titleFont', orbiteer.large),
	}

	piguiFace.style.infoDetailsHeight = math.ceil(
			piguiFace.style.nameFont.size + piguiFace.style.titleFont.size
			+ getStyleVar(style, 'infoDetailsPadding',48) * (ui.screenHeight / 1200)
	)

	piguiFace.Draw = function(self, faceSize)
		ui.image(self.texture.id, faceSize, Vector2(0.0, 0.0), self.texture.uv, colors.white)

		local lastPos = ui.getCursorPos()
		ui.setCursorPos(lastPos - Vector2(0.0, self.style.infoDetailsHeight + self.style.itemSpacing.y))
		ui.withStyleColors({ChildWindowBg = self.style.bgColor}, function()
			ui.withStyleVars({WindowPadding = self.style.windowPadding, ItemSpacing = self.style.itemSpacing}, function()
				ui.child("PlayerInfoDetails", Vector2(faceSize.x, self.style.infoDetailsHeight), {"AlwaysUseWindowPadding", "NoScrollbar"}, function ()
					ui.withFont(self.style.nameFont.name, self.style.nameFont.size, function()
						ui.text(self.character.name)
					end)
					ui.withFont(self.style.titleFont.name, self.style.titleFont.size, function()
						ui.text(self.character.title or '')
					end)
				end)
			end)
		end)
		ui.setCursorPos(lastPos)
	end

	setmetatable(piguiFace, {
		__index = PiGuiFace,
		class = "UI.PiGuiFace",
	})

	return piguiFace
end

return PiGuiFace
