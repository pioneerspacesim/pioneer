-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module sets up the "Crew" debug settings

local debugView = require 'pigui.views.debug'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local ui = require 'pigui'
local arrayTable = require 'pigui.libs.array-table'
local utils = require 'utils'
local icons = ui.theme.icons
local colors = ui.theme.colors
local Ship = require 'Ship'
local Character = require 'Character'

-- local PiImage = require 'pigui.libs.image'
-- local upIcon = PiImage.New("icons/market/export-major.png")
-- local downIcon = PiImage.New("icons/market/import-major.png")
-- local iconSize = Vector2(0, ui.getLineHeight())


-- local crewcontracts = require 'modules.CrewContracts.CrewContracts'
local crewlife = require 'modules.CrewContracts.CrewLife'

local thought_selected = 0
local thought_labels = {}
for thought_name, _ in pairs(crewlife.thoughts) do
   table.insert(thought_labels, thought_name)
end


debugView.registerTab("debug-crew", {
   label = "Crew Debug",
   icon = icons.taxi,
   show = function() return Game.system and not Game:InHyperspace() end,
   draw = function ()

      if ui.button("Hire Crew", Vector2(100, 0)) then
	 if Game.player:CrewNumber() < ShipDef[Game.player.shipId].maxCrew then
            Game.player:Enroll(Character.New())
	 end
      end
      ui.sameLine()
      ui.text(Game.player:CrewNumber().." of "..ShipDef[Game.player.shipId].maxCrew.." hired")

        for crew in Game.player:EachCrewMember() do
	   if not crew.player then
	      local header = crew.name
	      
	      if ui.collapsingHeader(header, {}) then
		 ui.columns(2)
		 
		 ui.text("Qualifications")
		 ui.nextColumn()
		 ui.text("Stats")
		 ui.nextColumn()
		 
		 crew.engineering = ui.sliderInt("Engineering", crew.engineering, 4, 65)
		 ui.nextColumn()
		 crew.luck = ui.sliderInt("Luck", crew.luck, 4, 65)
		 ui.nextColumn()
		 
		 crew.piloting = ui.sliderInt("Piloting", crew.piloting, 4, 65)
		 ui.nextColumn()
		 crew.intelligence = ui.sliderInt("Intelligence", crew.intelligence, 4, 65)
		 ui.nextColumn()
		 
		 crew.navigation = ui.sliderInt("Navigation", crew.navigation, 4, 65)
		 ui.nextColumn()
		 crew.charisma = ui.sliderInt("Charisma", crew.charisma, 4, 65)
		 ui.nextColumn()
		 
		 crew.sensors = ui.sliderInt("Sensors", crew.sensors, 4, 65)
		 ui.nextColumn()
		 crew.lawfulness = ui.sliderInt("Lawfulness", crew.lawfulness, 4, 65)
		 ui.nextColumn()
		 
		 ui.text("Reputation")
		 ui.nextColumn()
		 ui.text("Affinity to Civilization")
		 ui.nextColumn()

		 crew.notoriety = ui.sliderInt("Notoriety", crew.notoriety, 4, 65)
		 ui.nextColumn()

		 -- TODO put somewhere upstream
		 if not crew.civaffinity then crew.civaffinity = 1 end
		 crew.civaffinity = ui.sliderInt("Civ Affinity", crew.civaffinity, 1, 3)
		 ui.nextColumn()
		 
		 ui.text("Happiness")
		 ui.nextColumn()
		 ui.text("")
		 ui.nextColumn()
		 
		 crew.playerRelationship = ui.sliderInt("Relationship with Captain", crew.playerRelationship, 4, 65)

		 ui.columns(1)
		 ui.dummy(Vector2(0, 5))
		 ui.separator()
		 ui.dummy(Vector2(0, 5))

		 ui.columns(2)
		 ui.text("Memories")
		 ui.nextColumn()
		 ui.text("")
		 ui.nextColumn()
		 for i, thought in pairs(crew.memories) do
		    if thought.adjustment < 0 then
		       ui.icon(icons.up, Vector2(ui.getTextLineHeight()), colors.econLoss)
		       ui.sameLine()
		       ui.textColored(colors.econLoss, thought.text)
		    else
		       ui.icon(icons.down, Vector2(ui.getTextLineHeight()), colors.econProfit, "")
		       ui.sameLine()
		       ui.textColored(colors.econProfit, thought.text)
		    end
		 end
		 ui.nextColumn()
		 _, thought_selected = ui.combo("##Thoughts", thought_selected, thought_labels)
		 ui.sameLine()
		 if ui.button("Apply Thought", Vector2(150, 0)) then
		    thought = crewlife.thoughts[thought_labels[thought_selected + 1]]
		    crewlife.applyThought(crew, thought)
		 end
		 
		 ui.dummy(Vector2(0, 40))
		 ui.separator()
		 ui.dummy(Vector2(0, 5))
		 ui.columns(1)

		 if ui.button("Remove Crew", Vector2(100, 0)) then
		    Game.player:Dismiss(crew)
		 end
		 ui.separator()
	      end
	   end
	end
   end
})
	 
