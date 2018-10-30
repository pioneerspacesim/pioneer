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
local Player = import("Player")
local utils = import("utils")
local Vector = import("Vector")

local InfoFace = import("ui/InfoFace")
local NavButton = import("ui/NavButton")

local l = Lang.GetResource("module-collectdata")

local ui = Engine.ui
local missions = {}
local ads = {}

local scienceLabs = {
  "INTERGALACTIC_RESEARCH_FOUNDATION",
  "EXTRATERESTIAL_SCIENCE_LABS",
  "EARTH_FEDERATION_OF_SCIENCE",
  "INDEPENDENT_SCIENTISTS"
}

local createRequestMessage = function(ad)
  return "The "..ad.scienceLab.." wants you to collect data from "..ad.planetToGoTo.name..
  " and bring it back to "..ad.location:GetSystemBody().name.."."
end

local onChat = function(form, ref, option)
  local ad = ads[ref]

  form:Clear()

  if option == -1 then
    form:Close()
    return
  end

  form:SetFace(ad.client)
  form:AddNavButton(ad.planetToGoTo.path)

  if option == 0 then
    form:SetMessage(createRequestMessage(ad))
  elseif option == 1 then
    form:RemoveAdvertOnClose()
    ads[ref] = nil

    local mission = {
      type = "DataCollection",
      client = ad.client,
      location = ad.location,
      planetToGoTo = ad.planetToGoTo,
      reward = ad.reward,
      --due = ad.due,
      scienceLab = ad.scienceLab
    }
    table.insert(missions, Mission.New(mission))
    form:SetMessage(l.THANKS_I_CANT_WAIT_FOR_THE_RESULTS)
    return
  end

  form:AddOption(l.OK_AGREED, 1)
end

local makeAdvert = function(station)
  local possiblePlanets = Game.system:GetBodyPaths()
  local planetToGoTo
  while (not planetToGoTo) or (planetToGoTo.type=="STARPORT_SURFACE") or (planetToGoTo.type=="STARPORT_ORBITAL") do
    local planetPathToGoTo = possiblePlanets[Engine.rand:Integer(1, #possiblePlanets)]
    planetToGoTo = planetPathToGoTo:GetSystemBody()
  end
  local ad = {
    client = Character.New(),
    --due = Game.time + 1000000,
    location = station.path,
    planetToGoTo = planetToGoTo,
    reward = 400,
    scienceLab = l[scienceLabs[Engine.rand:Integer(1, #scienceLabs)]]
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
end

local onShipDocked = function(player, station)
	if not player:IsPlayer() then return end

	for ref, mission in pairs(missions) do
    if mission.location == station.path then
      if mission.collectedData then
        Comms.ImportantMessage(l.VERY_GOOD_I_WILL_SEND_THIS_DATA_DIRECTLY_TO_THE..mission.scienceLab.."!", mission.client.name)
        player:AddMoney(mission.reward)
        mission:Remove()
  			missions[ref] = nil
      end
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
  local collectDataButton = ui:HBox(10):PackEnd({
    ui:Label("Collect data"),
    ui:SmallButton()
  })
  collectDataButton.onClick:Connect(function()
    local planetRadius = mission.planetToGoTo.radius
    local distanceToPlanet = Game.player:DistanceTo(mission.planetToGoTo.body)
    if distanceToPlanet<planetRadius*2 then
      Comms.Message("Data from "..mission.planetToGoTo.name.." collected")
      mission.collectedData = true
    else
      Comms.Message("Not close enough to "..mission.planetToGoTo.name.." to collect data.")
    end
  end)
  return ui:Grid(2, 1)
    :SetColumn(0, {
      ui:VBox(10):PackEnd({
          ui:MultiLineText(createRequestMessage(mission)),
          NavButton.New("Set to be inspected planet as target", mission.planetToGoTo.path),
          NavButton.New("Set station to bring the data to as target", mission.location),
          collectDataButton
        })
      }
    )
		:SetColumn(1, {
			ui:VBox(10):PackEnd(InfoFace.New(mission.client))
		})
end

Event.Register("onCreateBB", onCreateBB)
Event.Register("onEnterSystem", onEnterSystem)
Event.Register("onShipDocked", onShipDocked)
Event.Register("onGameStart", onGameStart)

Mission.RegisterType("DataCollection", l.DATACOLLECTION, onClick)

Serializer:Register("CollectData", serialize, unserialize)
