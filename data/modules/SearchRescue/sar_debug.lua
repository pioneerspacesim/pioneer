-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module sets up the "Search and Rescue" debug settings

local debugView = require 'pigui.views.debug'
local Game = require 'Game'
local ui = require 'pigui'
local arrayTable = require 'pigui.libs.array-table'
local utils = require 'utils'
local icons = ui.theme.icons

local sar_core = require 'modules.SearchRescue.sar_core'
local sar_flavours = require 'modules.SearchRescue.sar_flavours'


local flavour_selected = 0
local flavour_names = {}
local flavour_ids = {}
for _,flavour in pairs(sar_flavours) do
   table.insert(flavour_names, "["..flavour.id.."] "..flavour.description)
   table.insert(flavour_ids, flavour.id)
end

local ad = {feedback = ""}


debugView.registerTab("debug-sar", {
   label = "SAR Debug",
   icon = icons.searchrescue,
   show = function() return Game.system and not Game:InHyperspace() end,
   draw = function ()

      ---------------------------------------------
      -- number of sar ads in the current system --
      ---------------------------------------------
      ui.text("SAR ads in system: " .. utils.count(sar_core.ads))


      -----------------------------------------------------
      -- add sar ad to board of currently docked station --
      -----------------------------------------------------
      if ui.button("Create Ad", Vector2(100, 0)) then
	 if Game.player:IsDocked() then
	    ad = sar_core.makeAdvert(
	       Game.player:GetDockedWith(),
	       flavour_selected + 1,
	       nil
	    )
	 end
      end

      ui.sameLine()

      _, flavour_selected = ui.combo("##flavour_id", flavour_selected, flavour_names)

      ui.text(ad.feedback)
      

      ------------------------------------------------
      -- draw list of all sar ads in current system --
      ------------------------------------------------
      debug_ads = {}
      for ref,ad in pairs(sar_core.ads) do
	 local dist = nil
	 local pickup_comm = 0
	 local deliver_comm = 0
	 
	 if ad.flavour.loctype ~= "FAR_SPACE" then
	    dist = ui.Format.Distance(ad["dist"])
	 else
	    dist = string.format("%.2f ly", ad["dist"])
	 end

	 for _,comm in pairs(ad["pickup_comm"]) do
	    pickup_comm = comm
	 end

	 for _,comm in pairs(ad["deliver_comm"]) do
	    deliver_comm = comm
	 end

	 
	 table.insert(debug_ads, {
	    station = ad["station_local"]:GetSystemBody().name,
	    flavour = ad["flavour"]["id"],
	    loctype = ad["flavour"]["loctype"],
	    urgency = ad["urgency"],
	    dist = dist,
	    due = ui.Format.Date(ad["due"]),
	    shipdef_name = ad["shipdef_name"],
	    pickup_crew = ad["pickup_crew"],
	    pickup_pass = ad["pickup_pass"],
	    pickup_comm = pickup_comm,
	    deliver_crew = ad["deliver_crew"],
	    deliver_pass = ad["deliver_pass"],
	    deliver_comm = deliver_comm,
	    reward = ad["reward"]
	 })
      end
      
      if ui.collapsingHeader("Ads") then
	 arrayTable.draw("sar_adinfo", debug_ads, pairs,
	    {
	       {name = "Station",   key = "station",      string = true},
	       {name = "Flavour",   key = "flavour"                    },
	       {name = "Loc",       key = "loctype",      string = true},
	       {name = "Urgency",   key = "urgency"                    },
	       {name = "Dist",      key = "dist"                       },
	       {name = "Due",       key = "due"                        },
	       {name = "Ship",      key = "shipdef_name", string = true},
	       {name = "GetCrew",   key = "pickup_crew"                },
	       {name = "GetPass",   key = "pickup_pass"                },
	       {name = "GetComm",   key = "pickup_comm"                },
	       {name = "BringCrew", key = "deliver_crew"               },
	       {name = "BringPass", key = "deliver_pass"               },
	       {name = "BringComm", key = "deliver_comm"               },
	       {name = "Reward",    key = "reward"                     }
	 })
      end
   end   
})

