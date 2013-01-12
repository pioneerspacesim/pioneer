-- Copyright � 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This is a mission script for long-range expeditions to unexplored systems.  There will be a shorter range, common, version; and a long-range, rare version.  Both versions will be rare
-- compared to delivery missions and such.  The missions will be difficult and will require the player to own a large ship.
--
--
--
--
--
--
-- Get the translator function
local t = Translate:GetTranslator()
-- Get the UI class
local ui = Engine.ui

-- don't produce missions for further than this many light years away
local max_expedition_dist = 150
-- typical time for travel to a system max_expedition_dist away
local typical_travel_time = 2.4 * max_expedition_dist * 24 * 60 * 60
-- typical reward for delivery to a system max_expedition_dist away
local typical_reward = 225 * max_expedition_dist
--Max number of passengers
local max_group = 15


local ads = {}
local missions = {}

--Next two functions are for adding and removing the personnell
local add_passengers = function (group)
	Game.player:RemoveEquip('UNOCCUPIED_CABIN', group)
	Game.player:AddEquip('PASSENGER_CABIN', group)
	passengers = passengers + group
end

local remove_passengers = function (group)
	Game.player:RemoveEquip('PASSENGER_CABIN', group)
	Game.player:AddEquip('UNOCCUPIED_CABIN', group)
	passengers = passengers - group
end


--This is the form to create the bulletin board conversations. To add option, simply write an if option == # then <dosomething>. It will inform the player 
--if their ship is too small, or lacks enough passenger cabins.
local onChat = function (form, ref, option)
	local expedition_flavours = Translate:GetFlavours('Expedition')
	local ad = ads[ref]
	
		form:Clear()
		
	local corporation = t('CORPORATIONS')[Engine.rand:Integer(1,#(t('CORPORATIONS')))]	
	local element = t('ELEMENTS')[Engine.rand:Integer(1,#(t('ELEMENTS')))]
		
	local adtext = string.interp(expedition_flavours[ad.flavour], {
		name     = ad.client.name,
		cash     = Format.Money(ad.reward),
		system   = sys.name,
		sectorx  = ad.location.sectorX,
		sectory  = ad.location.sectorY,
		sectorz  = ad.location.sectorZ,
		dist     = string.format("%.2f", ad.dist),
	})

	if option == -1 then
		form:Close()
		return
	end
	
		--Set the character face
	if option == 0 then
		form:SetFace(ad.client)
			--Define the ad form location for the mission form.
		local sys   = ad.location:GetStarSystem()
		local sbody = ad.location:GetSystemBody()
		
			--The introduction text?
		local introtext = string.interp(expedition_flavours[ad.flavour].introtext, {
			name     = ad.client.name,
			cash     = Format.Money(ad.reward),
			system   = sys.name,
			group  = ad.group,
			sectorx  = ad.location.sectorX,
			sectory  = ad.location.sectorY,
			sectorz  = ad.location.sectorZ,
			dist     = string.format("%.2f", ad.dist),
		})

		form:SetMessage(introtext)
		
	elseif option == 1 then
	
	
		
	--Description of the expedition goal
	elseif option == 2 then
		local expeditiontext = string.interp(expedition_flavours[ad.flavour].expeditiontext, {
			corporation = corporation,
			passengers   = ad.group,
			months   =  math.ceil(ad.due / 60 / 60 / 24 / 30),
			element   = element,
			})
				
		form:SetMessage(expeditiontext) 
				

					
					
				
	-- Player accepts. Check if player has enough cabin space and large enough ship. If not, then tell player and hang up.
	elseif option == 3 then
		local capacity = Game.player:GetEquipSlotCapacity('CABIN')
		local Playerstats = Game.player:GetStats()
		if capacity < ad.group or Game.player:GetEquipCount('CABIN', 'UNOCCUPIED_CABIN') < ad.group then
			form:SetMessage(t("You do not have enough cabin space on your ship."))
			form:AddOption(t('HANG_UP'), 1)
		elseif Playerstats.totalMass < 120 then
			form:SetMessage(t("You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t."))
			form:AddOption(t('HANG_UP'), 1)
			return
		end

		add_passengers(ad.group)

		form:RemoveAdvertOnClose()

		ads[ref] = nil

		--Define and insert the table for this mission into players mission table.
		local mission = {
			type	 = "Expedition",
			client	 = ad.client,
			start    = ad.station.path,
			location = ad.location,
			risk	 = ad.risk,
			reward	 = ad.reward,
			due	 = ad.due,
			group	 = ad.group,
			flavour	 = ad.flavour
		}

		table.insert(missions,Mission.New(mission))

		form:SetMessage(t("Excellent."))
		form:AddOption(t('HANG_UP'), -1)

		return --Find out the deadline
	elseif option == 4 then
		if expedition_flavours[ad.flavour].planetarysurvey == 1 then
			form:SetMessage(t("We need you to return by  ")..Format.Date(ad.due))
		else
			form:SetMessage(t("We need to return with the data by ")..Format.Date(ad.due))
		end

		--Will there be any danger?
	elseif option == 5 then
		local dangertext = string.interp(expedition_flavours[ad.flavour].danger, {
			corporation = corporation,
			})
			
		form:SetMessage(dangertext)
		
		
	end	
		--Set all the options from the main screen
		
	if expedition_flavours[ad.flavour].planetarysurvey == 0 then
		form:AddOption(t("What kind of expedition are we talking about?"), 2)
	else 
		form:AddOption(t("Give me the specifics."), 2)
	end	
	form:AddOption(t("What is the timeline?"), 4)
	form:AddOption(t("Will there be any danger?"), 5)
	form:AddOption(t("Could you repeat the original request?"), 0)
	form:AddOption(t("Ok, agreed."), 3)
	form:AddOption(t('HANG_UP'), 1)
end


local onDelete = function (ref)
	ads[ref] = nil
end

--Get the actual objectives and all that for the mission.
local nearbysystems
local makeAdvert = function (station)
	local reward, due, location, unexploredsystem, dist
	local expedition_flavours = Translate:GetFlavours('Expedition')
	local client = Character.New({title = expedition_flavours[flavour].scientist})
	local flavour = Engine.rand:Integer(1,#expedition_flavours)
	local urgency = expedition_flavours[flavour].urgency
	local risk = expedition_flavours[flavour].risk

	if expedition_flavours[flavour].planetarysurvey == 1 then
		group = 0
		nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return  s:DistanceTo(Game.system) >= 20 and s:DistanceTo(Game.system) <= 60 and s.explored == false and #s:GetSystemBodies() ~= 0  end)
		if #nearbysystems == 0 then return end
		local nearbysystem
		local nearbybodies = Game.system:GetBodyPaths()
		location = nearbybodies[Engine.rand:Integer(1,#nearbybodies)]
		reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((30*24*60*60) * (Engine.rand:Number(2,4) - urgency))
	else
		group = Engine.rand:Integer(5,max_group)
		nearbysystems = Game.system:GetNearbySystems(max_delivery_dist, function (s) return s:DistanceTo(Game.system) >= 100 and s.explored == false and #s:GetSystemBodies() ~= 0 end)
		if #nearbysystems == 0 then return end
		location = nearbysystems[Engine.rand:Integer(1,#nearbysystems)]
		dist = location:DistanceTo(Game.system)
		reward = ((dist / max_delivery_dist) * typical_reward * (1+risk) * (1.5+urgency) * Engine.rand:Number(0.8,1.2))
		due = Game.time + ((dist / max_delivery_dist) * typical_travel_time * (1.5-urgency) * Engine.rand:Number(0.9,1.1))
	end

	local ad = {
		station		= station,
		flavour		= flavour,
		client		= client,
		location		= location,
		dist            = dist,
		group    = group,
		due		= due,
		risk		= risk,
		urgency		= urgency,
		reward		= reward,
		isfemale	= isfemale,
		faceseed	= Engine.rand:Integer(),
	}


	ad.desc = string.interp(expedition_flavours[flavour].adtext, {
		system	= location.name,
		cash	= Format.Money(ad.reward),
	})

	local ref = station:AddAdvert(ad.desc, onChat, onDelete)
	ads[ref] = ad
end

local onCreateBB = function (station)
	local num
	if expedition_flavours[ad.flavour].planetarysurvey == 1 then
		num = Engine.rand:Integer(0, math.ceil(Game.system.population) / 2 )
	else 
		num = Engine.rand:Integer(0, math.ceil(Game.system.population) / 3	)
	end
	for i = 1,num do
		makeAdvert(station)
	end
end

local onUpdateBB = function (station)
	local expedition_flavours = Translate:GetFlavours('DeliverPackage')
	for ref,ad in pairs(ads) do
		if expedition_flavours[ad.flavour].planetarysurvey == 0
			and ad.due < Game.time + 10*60*60*24 then -- five day timeout for inter-system
			ad.station:RemoveAdvert(ref)
		elseif expedition_flavours[ad.flavour].planetarysurvey == 1
			and ad.due < Game.time + 5*60*60*24 then -- two day timeout for single, short ones.
			ad.station:RemoveAdvert(ref)
		end
	end
	if Engine.rand:Integer(12*60*60) < 60*60 then -- roughly once every twelve hours
		makeAdvert(station)
	end
end



local onEnterSystem = function (player)
	if (not player:IsPlayer()) then return end
	local expedition_flavours = Translate:GetFlavours('Expedition')

	local syspath = Game.system.path
	

	for ref,mission in pairs(missions) do
	if mission.status and mission.location:IsSameSystem(syspath) then
		local sysbodies = function ()
			local bodies = Game.system:GetBodyPaths ()
			
			
			
								return 
		local atmosphere = bodies[Engine.rand:Integer(1, #bodies)]:GetSystemBody.superType
		if 
		
		
		
		
		
		
		if not mission.status and mission.location:IsSameSystem(syspath) then
			local risk = expedition_flavours[mission.flavour].risk
			local ships = 0

			local riskmargin = Engine.rand:Number(-0.3,0.3) -- Add some random luck
			if risk >= (1 + riskmargin) then ships = 3
			elseif risk >= (0.7 + riskmargin) then ships = 2
			elseif risk >= (0.5 + riskmargin) then ships = 1
			end

			if ships < 1 and risk > 0 and Engine.rand:Integer(math.ceil(1/risk)) == 1 then ships = 1 end

			local shiptypes = ShipType.GetShipTypes('SHIP', function (t)
				local mass = t.hullMass
				return mass >= 80 and mass <= 200
			end)
			if #shiptypes == 0 then return end

			local ship

			while ships > 0 do
				ships = ships-1

				if Engine.rand:Number(1) <= risk then
					local shipid = shiptypes[Engine.rand:Integer(1,#shiptypes)]
					local shiptype = ShipType.GetShipType(shipid)
					local default_drive = shiptype.defaultHyperdrive

					local max_laser_size = shiptype.capacity - EquipType.GetEquipType(default_drive).mass
					local lasers = EquipType.GetEquipTypes('LASER', function (e,et)
						return et.mass <= max_laser_size and string.sub(e,0,11) == 'PULSECANNON'
					end)
					local laser = lasers[Engine.rand:Integer(1,#lasers)]

					ship = Space.SpawnShipNear(shipid, Game.player, 50, 100)
					ship:AddEquip(default_drive)
					ship:AddEquip(laser)
					ship:AddEquip('SHIELD_GENERATOR', math.ceil(risk * 3))
					if Engine.rand:Number(2) <= risk then
						ship:AddEquip('LASER_COOLING_BOOSTER')
					end
					if Engine.rand:Number(3) <= risk then
						ship:AddEquip('SHIELD_ENERGY_BOOSTER')
					end
					ship:AIKill(Game.player)
				end
			end

			if ship then
				local pirate_greeting = string.interp(t('PIRATE_TAUNTS')[Engine.rand:Integer(1,#(t('PIRATE_TAUNTS')))], { client = mission.client.name,})
				Comms.ImportantMessage(pirate_greeting, ship.label)
			end
		end

		if not mission.status and Game.time > mission.due then
			mission.status = 'FAILED'
			Comms.ImportantMessage(expedition_flavours[mission.flavour].wherearewe, mission.client.name)
		end
	end
end
