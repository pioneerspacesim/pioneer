-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local SpaceStation = import("SpaceStation")
local utils = import("utils")

local ui = Engine.ui

local bulletinBoard = function (args)
    local station = Game.player:GetDockedWith()
    return ui:Table():AddRows(utils.build_array(utils.map(function (k,v) return k,{v[1]} end, ipairs(SpaceStation.adverts[station]))))
end

return bulletinBoard
