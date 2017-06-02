-- Copyright © 2008-2017 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local Space = import("Space")

local ui = Engine.ui

local NavButton = {}

function NavButton.New (text, target)
	local self = {
		button	= ui:SmallButton(),
		label	= ui:Label(text),
		target	= target,
	}
	self.widget = ui:HBox(10):PackEnd({ self.button, self.label })

	if target then
		self.widget.onClick:Connect(function ()
				if self.target:isa("Body") and self.target:IsDynamic() then
					Game.player:SetNavTarget(self.target)
				elseif self.target:IsSameSystem(Game.system.path) then
					if self.target.bodyIndex then
						Game.player:SetNavTarget(Space.GetBody(self.target.bodyIndex))
					end
				else
					Game.player:SetHyperspaceTarget(self.target:GetStarSystem().path)
					-- XXX we should do something useful here
					-- e.g. switch to the sector map or just beep
				end
		end)
	end

	setmetatable(self, {
		__index	= NavButton,
		class	= "UI.NavButton",
	})

	return self
end

return NavButton
