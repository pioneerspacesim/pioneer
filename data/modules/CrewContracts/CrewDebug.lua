-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module sets up the "Crew" debug settings

local debugView = require 'pigui.views.debug'
local Game = require 'Game'
local ShipDef = require 'ShipDef'
local ui = require 'pigui'
local utils = require 'utils'
local icons = ui.theme.icons
local colors = ui.theme.colors
local Character = require 'Character'
local PlayerState = require 'PlayerState'

-- local PiImage = require 'pigui.libs.image'
-- local upIcon = PiImage.New("icons/market/export-major.png")
-- local downIcon = PiImage.New("icons/market/import-major.png")
-- local iconSize = Vector2(0, ui.getLineHeight())


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
   draw = function()

      ui.text("Run event registered functions within this module:")
      ui.dummy(Vector2(0, 3))

      ui.columns(2)

      local stats = crewlife.recentSystems()
      
      if ui.button("onEnterSystem", Vector2(200, 0)) then
	 crewlife.onEnterSystem(Game.player)
      end

      ui.nextColumn()
      ui.text("n: "..stats.n.." / Mean Pop: "..utils.round(stats.mean_pop, 2).." / Explored: "..stats.explored.." / Legal Status: "..PlayerState:GetLegalStatus())


      ui.columns(1)
      ui.dummy(Vector2(0, 5))
      ui.separator()
      ui.dummy(Vector2(0, 5))

      if ui.button("Hire Crew", Vector2(100, 0)) then
		  if Game.player:CrewNumber() < ShipDef[Game.player.shipId].maxCrew then
			  local newCrewMember = Character.New()
			  Game.player:Enroll(newCrewMember)
			  crewlife.onJoinCrew(Game.player, newCrewMember)
		  end
      end
      ui.sameLine()
      ui.text(Game.player:CrewNumber() .. " of " .. ShipDef[Game.player.shipId].maxCrew .. " hired")

      ui.dummy(Vector2(0, 5))

      local crewid = 0
      for crew in Game.player:EachCrewMember() do
	 if not crew.player then
	    local header = crew.name
	    crewid = crewid + 1
	    
	    if ui.collapsingHeader(header, {}) then
	       ui.columns(2)
	       
	       ui.text("Qualifications")
	       ui.nextColumn()
	       ui.text("Stats")
	       ui.nextColumn()
	       
	       crew.engineering = ui.sliderInt("Engineering" .. "##" .. crewid, crew.engineering, 4, 65)
	       ui.nextColumn()
	       crew.luck = ui.sliderInt("Luck" .. "##" .. crewid, crew.luck, 4, 65)
	       ui.nextColumn()
	       
	       crew.piloting = ui.sliderInt("Piloting" .. "##" .. crewid, crew.piloting, 4, 65)
	       ui.nextColumn()
	       crew.intelligence = ui.sliderInt("Intelligence" .. "##" .. crewid, crew.intelligence, 4, 65)
	       ui.nextColumn()
	       
	       crew.navigation = ui.sliderInt("Navigation" .. "##" .. crewid, crew.navigation, 4, 65)
	       ui.nextColumn()
	       crew.charisma = ui.sliderInt("Charisma" .. "##" .. crewid, crew.charisma, 4, 65)
	       ui.nextColumn()
	       
	       crew.sensors = ui.sliderInt("Sensors" .. "##" .. crewid, crew.sensors, 4, 65)
	       ui.nextColumn()
	       crew.lawfulness = ui.sliderInt("Lawfulness" .. "##" .. crewid, crew.lawfulness, 4, 65)
	       ui.nextColumn()
	       
	       ui.text("Reputation")
	       ui.nextColumn()
	       ui.text("Affinity to Civilization")
	       ui.nextColumn()

	       crew.notoriety = ui.sliderInt("Notoriety" .. "##" .. crewid, crew.notoriety, 4, 65)
	       ui.nextColumn()

	       -- TODO put somewhere upstream
	       if not crew.civaffinity then crew.civaffinity = 1 end
	       crew.civaffinity = ui.sliderInt("Civ Affinity" .. "##" .. crewid, crew.civaffinity, 1, 3)
	       ui.nextColumn()
	       
	       ui.text("Happiness")
	       ui.nextColumn()
	       ui.text("")
	       ui.nextColumn()
	       
	       crew.playerRelationship = ui.sliderInt("Relationship with Captain" .. "##" .. crewid, crew.playerRelationship, 4, 65)

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
		     ui.icon(icons.down, Vector2(ui.getTextLineHeight()), colors.econProfit)
		     ui.sameLine()
		     ui.textColored(colors.econProfit, thought.text)
		  end
	       end

	       ui.nextColumn()
	       _, thought_selected = ui.combo("Thoughts" .. "##" .. crewid, thought_selected, thought_labels)
	       ui.sameLine()
	       local sel_thought = crewlife.thoughts[thought_labels[thought_selected + 1]]
	       local thought_chance = sel_thought.chance * 100
	       if ui.button("Apply Thought" .. "##" .. crewid, Vector2(150, 0)) then
		  crewlife.applyThought(crew, sel_thought)
	       end
	       ui.text("Probability: "..thought_chance.."% / Happiness Adjustment: "..sel_thought.adjustment)
	       
	       ui.dummy(Vector2(0, 40))
	       ui.separator()
	       ui.dummy(Vector2(0, 5))
	       ui.columns(1)

	       if ui.button("Remove CrewMember" .. "##" .. crewid, Vector2(200, 0)) then
		  Game.player:Dismiss(crew)
	       end
	       ui.separator()
	    end
	 end
      end
   end
})

