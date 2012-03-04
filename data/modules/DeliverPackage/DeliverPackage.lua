-- Get the translator function
local t = Translate:GetTranslator()

local hourlyRate = 500 / (24 * 30)
local basePriceScale = 1 / 160
local AU = 149598000000.0	-- Game uses this approximation

local ads = {}
local missions = {}
local updates

local NearBySystems = {}

-- For given number determine the radius of a sphere containing that
-- many systems. Store them in two tables.
function NearBySystems:init()
   if not next (Game.system:GetStationPaths()) then
      return
   end
   local maxSystems, radius, limit, all = 40, 30.0, 30.0
   while true do
      -- The initial call of GetNearbySystems is really slow if radius 60
      all = Game.system:GetNearbySystems (radius)
      local n = #all
      if n > maxSystems then
	 limit, radius = radius, radius * 0.5
      elseif n < maxSystems then
	 if limit - radius < 1 then
	    break
	 end
	 radius = (radius + limit) * 0.5
      else
	 break
      end
   end
   local inhabited = all and Game.system:GetNearbySystems (
      radius, function (s) return #s:GetStationPaths() > 0 end)
   print ("Sysinf: "..radius.." "..limit.." "..
	  (inhabited and #inhabited or 0).."/"..(all and #all or 0))
   if inhabited then
      self.all, self.inhabited = all, inhabited
      return self
   end
end

local nearbySystems

-- Shortest path, need better version like A*.
local function spath (beg, dst, nodes, fdist)
   local unvisited = { [beg] = beg, [dst] = dst }
   local dists = { [beg] = { prev = nil, dist = 0 } }
   local od = {}
   for i, n in next, nodes do
      unvisited[n] = n
   end
   local curr
   while true do
      local cdh
      for n, dh in next, dists do
	 if not cdh or dh.dist < cdh.dist then
	    curr, cdh = n, dh
	 end
      end
      if not cdh then
	 return nil
      end
      unvisited[curr], od[curr], dists[curr] = nil, dists[curr], nil
      if curr == dst then
	 break
      end
      for i, n in next, unvisited do
	 local d, state = fdist (curr, n, cdh.state)
	 if d then
	    local dh = dists[n]
	    if not dh or dh.dist > cdh.dist + d then
	       dists[n] = {
		  prev = curr, dist = cdh.dist + d, state = state }
	    end
	 end
      end
   end
   local i, path = 1, {}
   while true do		-- build reversed path
      local p = od[curr]
      path[i], p, p.prev, p.node = p, p.prev, nil, curr
      if curr == beg then
	 break
      end
      curr = p
      i = i + 1
   end
   return path
end

-- Stolen from source, should be part of the API!
local function calcHyperspaceRange (hyperclass, totalmass)
   return 200.0 * hyperclass * hyperclass / (totalmass * 0.6)
end

local function hyperspaceDuration (hyperclass, lydist, totalmass)
   return
      ((lydist * lydist * 0.5) /
       (calcHyperspaceRange (hyperclass, totalmass) * hyperclass)) * 
      (60.0 * 60.0 * 24.0 * math.sqrt (totalmass));
end

-- simplified version of hyperspaceDuration.
local function estimateDuration (lydist, totalmass)
   return lydist * lydist * totalmass * math.sqrt (totalmass)
end

local Hyperclasses =
{
   DRIVE_CLASS1 = 1,
   DRIVE_CLASS2 = 2,
   DRIVE_CLASS3 = 3,
   DRIVE_CLASS4 = 4,
   DRIVE_CLASS5 = 5,
   DRIVE_CLASS6 = 6,
   DRIVE_CLASS7 = 7,
   DRIVE_CLASS8 = 8,
   DRIVE_CLASS9 = 9
}

local function fwdacc (ship, ecmass)
   return math.abs (ship:GetLinearThrust ("FORWARD") /
		    (ship.hullMass + ecmass) / 1000)
end

-- Estimated intra system flight time for given way and acceleration.
local function ftim (s, a)
   -- my tests show computer does not brake as hard as it accelerates
   local b = 0.6 -- version 18 0.8
   return (1+b) * math.sqrt (s / (b*(1+b)) / (a/2)) + 3600
end

local function roundup (x, y)
   return math.floor ((x + y - 1) / y) * y
end

local function validateCapacity (ship, slots)
   for slot, cap in next, slots do
      if ship:GetEquipSlotCapacity (slot) < cap then
	 return false
      end
   end
   return true
end

-- Return minimal and maximal acceleration of all ships
local function shipMinMax (ecmass, slots)
   ships = ShipType.GetShipTypes (
      "SHIP", function (ship) return validateCapacity (ship, slots) end)
   local minacc, maxacc, min, max
   for i, shipname in next, ships do
      local ship = ShipType.GetShipType (shipname)
      local acc = fwdacc (ship, ecmass)
      if not min or minacc > acc then
	 minacc = acc
	 min = ship
      end
      if not max or maxacc < acc then
	 maxacc = acc
	 max = ship
      end
   end
   return minacc, maxacc
end

local function accelerationClasses (ecmass, slots)
   local tab = {}
   ships = ShipType.GetShipTypes (
      "SHIP", function (ship) return validateCapacity (ship, slots) end)
   local n = 0
   for i, shipname in next, ships do
      local ship = ShipType.GetShipType (shipname)
      n = n + 1
      tab[n] = fwdacc (ship, ecmass)
   end
   table.sort (tab)
   local lo, g5 = 1, 5 * 9.81
   for i = 2, n do
      if tab[i] - tab[lo] > g5 then
	 lo = lo + 1; tab[lo] = tab[i]
      end
   end
   for i = lo + 1, n do
      tab[i] = nil
   end
   n = #tab
   return function (urgency)
      return tab[urgency < 1 and math.floor (urgency * n) + 1 or n] end
end

local function makeInst (class, inst)
   inst = inst or {}
   setmetatable (inst, class)
   class.__index = class
   return inst
end

local function systemInit()
   nearbySystems = makeInst (NearBySystems):init()
   updates = {}
end

local EquipParms = {}

function EquipParms:initClass()
   local mdiffs, pdiffs = {}, {}
   local cls1 = EquipType.GetEquipType ("DRIVE_CLASS1")
   for n, i in next, Hyperclasses do
      local e = EquipType.GetEquipType (n)
      mdiffs[n] = e.mass - cls1.mass
      pdiffs[n] = e.basePrice - cls1.basePrice
   end
   self.dm_diffs = mdiffs
   self.dp_diffs = pdiffs
   self.initClass = nil
end

function EquipParms:init (equip)
   local mass, price, wprice, slots = 0, 0, 0, {}
   for name, count in next, equip do
      e = EquipType.GetEquipType (name)
      if e.slot == "LASER" or e.slot == "MISSILE" then
	 wprice = wprice + e.basePrice
      else
	 price = price + e.basePrice
      end
      mass = mass + e.mass
      local x = slots[e.slot]
      slots[e.slot] = not x and count or x + count
   end
   self.mass, self.price, self.slots, self.wprice = mass, price, slots, wprice
   return self
end

local LocalCtr = {}

function LocalCtr:initClass()
   self.equip = makeInst (EquipParms):init {
      AUTOPILOT = 1,
      ATMOSPHERIC_SHIELDING = 1,
      SCANNER = 1 }
   -- Btw, GetShipTypes does not work on file level.
   self.minacc, self.maxacc = shipMinMax (self.equip.mass, self.equip.slots)
   self.selectAcceleration = accelerationClasses (
      self.equip.mass, self.equip.slots)
   self.initClass = nil
end

function LocalCtr:init (station, urgency)
   self.isLocal = true -- needed per instance for serialization
   local nearbyStations = Game.system:GetStationPaths()
   local dest = nearbyStations[Engine.rand:Integer (1, #nearbyStations)]
   if dest == station.path then
      return false
   end
   local dist = station:DistanceTo (Space.GetBody (dest.bodyIndex))
   if dist < 1000 then
      return false
   end
   self.dest = dest
   local acc_f = self.selectAcceleration (urgency)
   -- Flight time for distance and given acceleration
   self.urgency = urgency
   self.dist = dist
   local tim_f = ftim (self.dist, acc_f)
   -- Total time
   local tim_t = tim_f + math.min (math.max (15 * 60, 0.08 * tim_f), 36 * 3600)
   local expiry = tim_t - ftim (self.dist, self.maxacc)
   self.expiry = Game.time + Engine.rand:Number (10*60, expiry)
   self.due = roundup (Game.time + tim_t, 900)
   return self:update()
end

function LocalCtr:update()
   local tim_t = self.due - Game.time
   local potships = ShipType.GetShipTypes (
      "SHIP",
      function (ship)
	 return
	    ftim (self.dist, fwdacc (ship, self.equip.mass)) <= tim_t and
	    validateCapacity (ship, self.equip.slots)
      end)
   local reward, name
   for i, shipname in next, potships do
      local ship = ShipType.GetShipType (shipname)
      local tim_f = ftim (self.dist, fwdacc (ship, self.equip.mass))
      -- Assume we get every 9 hrs a job for the same destination
      local jobs = 1 + math.floor ((tim_t - tim_f) / (3600 * 9))
      -- crew to pay (only me) at an hourly rate + daily ship costs
      local price = tim_f / 3600 * (
	 (hourlyRate * 1) + (
	    self.equip.price / 250 + ship.basePrice * basePriceScale) / 24)
      -- todo competition
      local profit = 0.1 * price
      local price = price / jobs + profit
      if not reward or price < reward then
	 reward = price; name = shipname
	 break
      end
   end
   if self.urgency then
      print ("Local "..name.." "..self.urgency)
      self.urgency = nil
   end
   self.new = nil
   self.reward = reward and roundup (reward, 10)
   return self.reward ~= nil
end

local ExternalCtr = {}

function ExternalCtr:initClass()
   -- equip by risk (see onEnterSystem)
   self.equip07 = makeInst (EquipParms):init { -- Three ships possible
      DRIVE_CLASS1 = 1,
      AUTOPILOT = 1, ATMOSPHERIC_SHIELDING = 1, SCANNER = 1,
      MISSILE_GUIDED = 2,
      MISSILE_SMART = 2,
      PULSECANNON_4MW = 1 }
   self.equip04 = makeInst (EquipParms):init { -- Two pirate ships possible
      DRIVE_CLASS1 = 1,
      AUTOPILOT = 1, ATMOSPHERIC_SHIELDING = 1, SCANNER = 1,
      MISSILE_GUIDED = 2,
      MISSILE_SMART = 1,
      PULSECANNON_2MW = 1 }
   self.equip02 = makeInst (EquipParms):init { -- One pirate ship possible
      DRIVE_CLASS1 = 1,
      AUTOPILOT = 1, ATMOSPHERIC_SHIELDING = 1, SCANNER = 1,
      MISSILE_GUIDED = 2,
      PULSECANNON_1MW = 1 }
   self.equip00 = makeInst (EquipParms):init {
      DRIVE_CLASS1 = 1,
      AUTOPILOT = 1, ATMOSPHERIC_SHIELDING = 1, SCANNER = 1 }
   self.initClass = nil
end

function ExternalCtr:shipCosts (ship, equip, systems, drive)
   local ecmass = equip.mass + equip.dm_diffs[drive]
   local mass = ship.hullMass + ecmass
   local drivepval = Hyperclasses[drive]
   -- calculate path backwards
   local path = spath (
      self.dest:GetStarSystem(), Game.system, systems,
      function (c, n, tfuel)
	 tfuel = tfuel or 0
	 local fuel, dist, max = 1, c:DistanceTo (n)
	 for x = 1, 30 do
	    if ecmass + tfuel + fuel > ship.capacity then
	       return nil
	    end
	    max = calcHyperspaceRange (drivepval, mass + tfuel + fuel)
	    if dist > max then
	       return nil
	    end
	    local f = math.ceil (drivepval * drivepval * dist / max)
	    if f <= fuel then
	       return estimateDuration (dist, mass + tfuel + fuel), tfuel + fuel
	    end
	    fuel = f
	 end
	 return nil
      end)
   if not path then
      return
   end
   local tim_f = 0
   path[1].tim_f = tim_f
   for i = 1, #path - 1 do
      local c, n = path[i], path[i+1]
      tim_f = tim_f + hyperspaceDuration (
	 drivepval, c.node:DistanceTo (n.node), mass + c.state)
      n.tim_f = tim_f
   end
   local acc = fwdacc (ship, ecmass)
   -- add flight time in destination system
   tim_f = tim_f + ftim (11.5 * AU, acc)
   -- Adjust hyperdrive costs within equip.price and let customer pay
   -- for the non-default drive.
   local price = equip.price + equip.dp_diffs[drive] + 
      (equip.dp_diffs[ship.defaultHyperdrive] - equip.dp_diffs[drive]) * 10
   price =  equip.wprice + tim_f / 3600 * (
      (hourlyRate * 1) + (price / 250 + ship.basePrice * basePriceScale) / 24)
   -- todo competition
   local profit = 0.1 * price
   return tim_f, price + profit, path
end

function ExternalCtr:init (station, urgency, risk)
   local systems = nearbySystems and nearbySystems.inhabited
   if not systems then
      return false
   end
   local nearbySystem = systems[Engine.rand:Integer (1, #systems)]
   systems = nearbySystems.all
   do
      local stations = nearbySystem:GetStationPaths()
      self.dest = stations[Engine.rand:Integer(1, #stations)]
   end
   local dist = nearbySystem:DistanceTo (Game.system)
   local equip =
      risk >= 0.7 and self.equip07 or
      risk >= 0.4 and self.equip04 or
      risk >= 0.2 and self.equip02 or self.equip00
   -- TODO: improve hyperdrive selection, should depend on mass too.
   local minimal_drive =
      dist < 5  and "DRIVE_CLASS1" or
      dist < 15 and "DRIVE_CLASS2" or "DRIVE_CLASS3"
   local potships = ShipType.GetShipTypes (
      "SHIP",
      function (ship)
	 if ship.capacity > 100 then
	    return nil
	 end
	 local class = Hyperclasses[ship.defaultHyperdrive]
	 local drive = class and
	    (class > Hyperclasses[minimal_drive]
	     and ship.defaultHyperdrive or minimal_drive)
	 return
	    drive and
	    equip.mass + equip.dm_diffs[drive] + 1 < ship.capacity and
	    validateCapacity (ship, equip.slots)
      end)
   local ship, path
   for i, shipname in next, potships do
      local s = ShipType.GetShipType (shipname)
      local drive =
	 Hyperclasses[s.defaultHyperdrive] > Hyperclasses[minimal_drive]
	 and s.defaultHyperdrive or minimal_drive
      local t, r, p = self:shipCosts (s, equip, systems, drive)
      if r and (not self.reward or r < self.reward) then
	 ship, self.due, self.reward, path = s, t, r, p
	 break
      end
   end
   if not ship then
      return false
   end
   self.reward = roundup (self.reward * (1 + 4 * risk), 100)
   self.due = roundup (self.due + Game.time, 3600)
   local verbose = (station == Game.player:GetDockedWith())
   if verbose then
      print (ship.name.." "..Format.Date (self.due))
   end
   for i = 2, #path do
      local p, c, w = path[i-1], path[i]
      if verbose then
	 print ((i-1)..":"..p.node.name.."->"..c.node.name.."("..p.state.." "..
		Format.Date (c.tim_f + Game.time)..")")
      end
   end
   self.expiry = self.due - 5*60*60*24
   return true
end

local function onChat (form, ref, option)
   local delivery_flavours = Translate:GetFlavours('DeliverPackage')
   local ad = ads[ref]

   form:Clear()

   if option == -1 then
      form:Close()
      return
   end

   if option == 0 then
      form:SetFace (ad.client)

      local sys   = ad.ctr.dest:GetStarSystem()
      local sbody = ad.ctr.dest:GetSystemBody()

      local introtext = string.interp(
	 delivery_flavours[ad.flavour].introtext, {
	    name     = ad.client.name,
	    cash     = Format.Money(ad.ctr.reward),
	    starport = sbody.name,
	    system   = sys.name,
	    sectorx  = ad.ctr.dest.sectorX,
	    sectory  = ad.ctr.dest.sectorY,
	    sectorz  = ad.ctr.dest.sectorZ, })

      form:SetMessage(introtext)

   elseif option == 1 then
      form:SetMessage(delivery_flavours[ad.flavour].whysomuchtext)
   elseif option == 2 then
      form:SetMessage(t("It must be delivered by ")..Format.Date(ad.ctr.due))
   elseif option == 4 then
      if ad.risk <= 0.1 then
	 form:SetMessage(t("I highly doubt it."))
      elseif ad.risk > 0.1 and ad.risk <= 0.3 then
	 form:SetMessage(t("Not any more than usual."))
      elseif ad.risk > 0.3 and ad.risk <= 0.6 then
	 form:SetMessage(
	    t("This is a valuable package, you should keep your eyes open."))
      elseif ad.risk > 0.6 and ad.risk <= 0.8 then
	 form:SetMessage(t("It could be dangerous, "..
			   "you should make sure you're adequately prepared."))
      elseif ad.risk > 0.8 and ad.risk <= 1 then
	 form:SetMessage(t("This is very risky, "..
			   "you will almost certainly run into resistance."))
      end
   elseif option == 3 then
      form:RemoveAdvertOnClose()
      ads[ref] = nil
      local mission = {
	 type	 = t("Delivery"),
	 client	 = ad.client.name,
	 location = ad.ctr.dest,
	 risk	 = ad.risk,
	 reward	 = ad.ctr.reward,
	 start   = Game.time,
	 due	 = ad.ctr.due,
	 flavour = ad.flavour
      }

      local mref = Game.player:AddMission(mission)
      missions[mref] = mission

      form:SetMessage(t("Excellent. "..
			"I will let the recipient know you are on your way."))
      form:AddOption(t('HANG_UP'), -1)
      return
   end

   form:AddOption(t("Why so much money?"), 1)
   form:AddOption(t("How soon must it be delivered?"), 2)
   form:AddOption(t("Will I be in any danger?"), 4)
   form:AddOption(t("Could you repeat the original request?"), 0)
   form:AddOption(t("Ok, agreed."), 3)
   form:AddOption(t('HANG_UP'), -1)
end

local function onDelete (ref)
   ads[ref] = nil
end

local Advert = {}

function Advert:desc()
   local text = Translate:GetFlavours ('DeliverPackage')[self.flavour].adtext
   return string.interp (text, {
			    system = self.ctr.dest:GetStarSystem().name,
			    cash = Format.Money (self.ctr.reward),
			    starport = self.ctr.dest:GetSystemBody().name })
end

local function heavy_duty_check (flavours, station, bbcreate)
   return station:DistanceTo (Game.player) > 0.02 * AU or
      -- the spath calculation is a way too slow for lua.
      flavours.localdelivery == 0 and
      bbcreate and station ~= Game.player:GetDockedWith()
end

local function makeAdvert (station, bbcreate)
   local flavours = Translate:GetFlavours ('DeliverPackage')
   local flavour = Engine.rand:Integer (1, #flavours)
   flavours = flavours[flavour]
   if (heavy_duty_check (flavours, station, bbcreate)) then
      return false
   end
   local urgency = flavours.urgency
   local risk = flavours.risk
   local ctr = makeInst (flavours.localdelivery == 1 and LocalCtr or
			 flavours.localdelivery ~= 1 and ExternalCtr)
   if not ctr:init (station, urgency, risk) then
      return
   end
   local ad = {
      client   = Character.New(),
      board    = station,
      ctr      = ctr,
      flavour  = flavour,
      risk     = risk,
      urgency  = urgency }
   ad = makeInst (Advert, ad)

   ads[station:AddAdvert (ad:desc(), onChat, onDelete)] = ad
   return ad
end

local function onCreateBB (station)
   updates[station] = Game.time
   local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
   for i = 1,num do
      makeAdvert (station, true)
   end
end

local function onUpdateBB (station)
   if not updates[station] or Game.time - updates[station] < 3600 then
      return
   end
   updates[station] = Game.time
   local delivery_flavours = Translate:GetFlavours('DeliverPackage')
   for ref, ad in pairs (ads) do
      if ad.ctr.expiry < Game.time or
	 ad.ctr.isLocal and ad.board == station and not ad.ctr:update() then
	 ad.board:RemoveAdvert (ref)
      end
   end
   makeAdvert (station, false)
end

local function spawnPirates (n, ships, mission)
   local ship
   while n > 0 do
      n = n - 1
      local shipname = ships[Engine.rand:Integer (1, #ships)]
      local shiptype = ShipType.GetShipType (shipname)
      local default_drive = shiptype.defaultHyperdrive
      local max_laser_size = shiptype.capacity -
	 EquipType.GetEquipType (default_drive).mass
      local lasers = EquipType.GetEquipTypes (
	 "LASER",
	 function (e,et)
	    return
	       et.mass <= max_laser_size and
	       string.sub (e,0,11) == 'PULSECANNON'
	 end)
      local laser = lasers[Engine.rand:Integer(1, #lasers)]
      ship = Space.SpawnShipNear (shipname, Game.player, 50, 100)
      ship:AddEquip (default_drive)
      ship:AddEquip (laser)
      ship:AIKill (Game.player)
   end
   if ship then
      local pirate_greeting =
	 string.interp (t ('PIRATE_TAUNTS')[
			   Engine.rand:Integer(1,#(t('PIRATE_TAUNTS')))],
			   { client = mission.client,
			     location = mission.location,})
	 UI.ImportantMessage(pirate_greeting, ship.label)
   end
end

local function onEnterSystem (player)
   if not player:IsPlayer() then
      return
   end
   systemInit()
   local flavours = Translate:GetFlavours ('DeliverPackage')
   local syspath = Game.system.path
   local shiptypes
   for ref, mission in pairs (missions) do
      if not mission.status and mission.location:IsSameSystem (syspath) then
	 local risk = flavours[mission.flavour].risk
	 local ships = 0
	 -- Add some random luck
	 local riskmargin = Engine.rand:Number (-0.3, 0.3)
	 if risk >= (1 + riskmargin) then
	    ships = 3
	 elseif risk >= (0.7 + riskmargin) then
	    ships = 2
	 elseif risk >= (0.5 + riskmargin) or
	    -- if there is some risk and still no ships, flip a tricoin
	    risk >= 0.2 and Engine.rand:Integer(2) == 1 then
	    ships = 1
	 end
	 if not shiptypes then
	    shiptypes = ShipType.GetShipTypes (
	       "SHIP",
	       function (t) return t.hullMass >= 100 and t.hullMass <= 400 end)
	 end
	 if #shiptypes > 0 then
	    spawnPirates (ships, shiptypes, mission)
	 end
      end
      if not mission.status and Game.time > mission.due then
	 mission.status = 'FAILED'
	 player:UpdateMission (ref, mission)
      end
   end
end

local function onShipDocked (player, station)
   if not player:IsPlayer() then
      return
   end
   local delivery_flavours = Translate:GetFlavours ('DeliverPackage')
   for ref, mission in pairs (missions) do
      if mission.location == station.path then
	 if Game.time > mission.due then
	    UI.ImportantMessage (
	       delivery_flavours[mission.flavour].failuremsg, mission.client)
	    local f = 1.0 - delivery_flavours[mission.flavour].urgency -
	       (Game.time - mission.due) / (mission.due - mission.start)
	    local reward = f > 0 and roundup (mission.reward * f, 10) or 1 --;)
	    player:AddMoney (reward)
	 else
	    UI.ImportantMessage (
	       delivery_flavours[mission.flavour].successmsg, mission.client)
	    player:AddMoney (mission.reward)
	 end
	 player:RemoveMission(ref)
	 missions[ref] = nil
      elseif not mission.status and Game.time > mission.due then
	 mission.status = 'FAILED'
	 player:UpdateMission (ref, mission)
      end
   end
end

local loaded_data

local function onGameStart ()
   if LocalCtr.initClass then
      EquipParms:initClass()
      LocalCtr:initClass()
      ExternalCtr:initClass()
   end
   systemInit()
   ads, missions = {}, {}
   if not loaded_data then
      return
   end
   station = Game.player:GetDockedWith()
   if station then -- onCreateBB will not be called
      updates[station] = Game.time
   end
   for k, ad in pairs (loaded_data.ads) do
      makeInst (Advert, ad)
      makeInst (ad.ctr.isLocal and LocalCtr or ExternalCtr, ad.ctr)
      local ref = ad.board:AddAdvert (ad:desc(), onChat, onDelete)
      ads[ref] = ad
      updates[ad.board] = Game.time
   end
   for k, mission in pairs (loaded_data.missions) do
      local mref = Game.player:AddMission (mission)
      missions[mref] = mission
   end
   loaded_data = nil
end

local function serialize ()
   return { ads = ads, missions = missions }
end

local function unserialize (data)
   loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onUpdateBB:Connect(onUpdateBB)
EventQueue.onEnterSystem:Connect(onEnterSystem)
EventQueue.onShipDocked:Connect(onShipDocked)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("DeliverPackage", serialize, unserialize)
