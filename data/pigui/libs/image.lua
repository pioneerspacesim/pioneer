-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Image = require 'PiGui.Modules.Image'
local ui = require 'pigui'
local colors = ui.theme.colors

local PiImage = {}

function PiImage.New (filename)

	local piImage = {
		widget = Image.New(filename)
	}

	piImage.texture = {
		id = piImage.widget.id,
		size = piImage.widget.size,
		xy = piImage.widget.size.x / piImage.widget.size.y,
		uv = piImage.widget.uv
	}

	piImage.Draw = function(self, size)
		if (size.x == 0 and size.y == 0) then size = self.texture.size
		elseif (size.y == 0) then size.y = size.x / self.texture.xy
		elseif (size.x == 0) then size.x = size.y * self.texture.xy
		end

		ui.image(self.texture.id, size, Vector2(0.0, 0.0), self.texture.uv, colors.white)
	end

	setmetatable(piImage, {
		__index = PiImage,
		class = "UI.PiImage",
	})

	return piImage
end

return PiImage
