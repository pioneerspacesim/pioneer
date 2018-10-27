-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Space = import("Space")
local Comms = import("Comms")
local Event = import("Event")
local Mission = import("Mission")
local Format = import("Format")
local Serializer = import("Serializer")
local Character = import("Character")
local Equipment = import("Equipment")
local ShipDef = import("ShipDef")
local Ship = import("Ship")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")
local NavButton = import("ui/NavButton")

local l = Lang.GetResource("module-collectdata")

local ui = Engine.ui
local missions = {}
local ads = {}

-- create a list of science labs
local scienceLabs = {}
for labName, _ in pairs(l.SCIENCELABS) do
  table.insert(scienceLabs, labName)
end

local onChat = function (form, ref, option)
  local ad = ads[ref]

  form:Clear()

  if option == -1 then
    form:Close()
    return
  end

  form:SetFace(ad.client)

  if option == 0 then
    form:SetMessage(ad.scienceLab.." wants you to collect data from "..ad.location)
  elseif option == 1 then
    form:RemoveAdvertOnClose()
    ads[ref] = nil

    local mission = {
      type = "DataCollection",
      client = ad.client,
      location = ad.location,
      reward = ad.reward,
      due = ad.due
    }
    table.insert(missions, Mission.New(mission))
    form:SetMessage(l.THANKS_I_CANT_WAIT_FOR_THE_RESULTS)
    return
  end

  form:AddOption(l.OK_AGREED, 1)
end

function makeAdvert(station)
  local ad = {
    client = Character.New(),
    due = Game.time + 1000000,
    location = Game.system:GetNearbySystems(100)[1],
    reward = 400,
    scienceLab = l[scienceLabs[Engine.rand:Integer(#scienceLabs)]]
  }
  local ref = station:AddAdvert({
    description = "Collect data for us!",
		icon = "delivery",
		onChat = onChat,
		onDelete = onDelete,
		isEnabled = isEnabled
  })
  ads[ref] = ad
  return ad
end

local isEnabled = function ()
  return true
end

local onDelete = function(ref)
	ads[ref] = nil
end

local onCreateBB = function(station)
  assert(makeAdvert(station))
end

local onEnterSystem = function(player)
  print("player entered system")
end

local onShipDocked = function(player, station)
	if not player:IsPlayer() then return end

	for _, mission in pairs(missions) do
    if mission.startingStation == station then
      print("correct station")
    end
	end
end

local loaded_data

local onGameStart = function()
	missions = {}
  ads = {}

  if not loaded_data or not loaded_data.ads then return end

  for k,ad in pairs(loaded_data.ads) do
    local ref = ad.station:AddAdvert({
      description = ad.desc,
      icon        = "delivery_urgent",
      onChat      = onChat,
      onDelete    = onDelete,
      isEnabled   = isEnabled })
    ads[ref] = ad
  end

  missions = loaded_data.missions

	loaded_data = nil
end

local serialize = function()
	return {
    ads = ads, missions = missions
  }
end

local unserialize = function(data)
	loaded_data = data
end

local onClick = function(mission)

end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)

Mission.RegisterType("DataCollection", l.DATACOLLECTION, onClick)

Serializer:Register("CollectData", serialize, unserialize)
