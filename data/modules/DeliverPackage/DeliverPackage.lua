-- Get the translator function
local t = Translate:GetTranslator()

-- don't produce missions for further than this many light years away
local max_delivery_dist = 20
local hourly_rate = 2000 / (24 * 30)
local AU = 149598000000.0	-- Game uses this approximation

local ads = {}
local missions = {}

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

local function ftim (s, a)
   -- my tests show computer does not brake as hard as it accelerates
   local b = 0.8
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

local function makeInst (class, inst)
   inst = inst or {}
   setmetatable (inst, class)
   class.__index = class
   return inst
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
   self.initClass = nil
end

function LocalCtr:init (station, urgency)
   self.isLocal = true -- needed per instance for serialization
   local nearbystations = Game.system:GetStationPaths()
   local dest = nearbystations[Engine.rand:Integer (1, #nearbystations)]
   if dest == station.path then
      return false
   end
   local dist = station:DistanceTo (Space.GetBody (dest.bodyIndex))
   if dist < 1000 then
      return false
   end
   self.dest = dest
   local acc_f = self.minacc + (self.maxacc - self.minacc) * urgency
   -- Flight time for distance and given acceleration
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
   local reward
   for i, shipname in next, potships do
      local ship = ShipType.GetShipType (shipname)
      local tim_f = ftim (self.dist, fwdacc (ship, self.equip.mass))
      -- crew to pay (only me) at an hourly rate of:
      local salary = (hourly_rate * 1) * tim_f / 3600
      -- Assume we get every 9 hrs a job for the same destination
      local jobs = 1 + math.floor ((tim_t - tim_f) / (3600 * 9))
      local price = salary + self.equip.price / 100 + ship.basePrice / 1000
      -- todo competition
      local profit = 0.3 * price
      local price = price / jobs + profit
      if not reward or price < reward then
	 reward = price
      end
   end
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

function ExternalCtr:shipCosts (ship, equip, nearbysystems, drive)
   local ecmass = equip.mass + equip.dm_diffs[drive]
   local mass = ship.hullMass + ecmass
   local drivepval = Hyperclasses[drive]
   -- calculate path backwards
   print (os.clock().." Checking "..#nearbysystems)
   local path = spath (
      self.dest:GetStarSystem(), Game.system, nearbysystems,
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
   print (os.clock().." Checked "..#nearbysystems)
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
   local salary = (hourly_rate * 1) * tim_f / 3600
   -- Adjust hyperdrive costs within equip.price and let customer pay
   -- for the non-default drive.
   local price = equip.price + equip.dp_diffs[drive] + 
      (equip.dp_diffs[ship.defaultHyperdrive] - equip.dp_diffs[drive]) * 10
   price = salary + equip.wprice + price / 100 + ship.basePrice / 1000 
   -- todo competition
   local profit = 0.3 * price
   return tim_f, price + profit, path
end

function ExternalCtr:init (station, urgency, risk)
   local nearbysystems = Game.system:GetNearbySystems (
      max_delivery_dist, function (s) return #s:GetStationPaths() > 0 end)
   if #nearbysystems == 0 then
      return false
   end
   local nearbysystem = nearbysystems[Engine.rand:Integer (1, #nearbysystems)]
   local nearbystations = nearbysystem:GetStationPaths()
   self.dest = nearbystations[Engine.rand:Integer(1, #nearbystations)]
   local dist = nearbysystem:DistanceTo (Game.system)
   nearbysystems = Game.system:GetNearbySystems (max_delivery_dist)
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
      local t, r, p = self:shipCosts (s, equip, nearbysystems, drive)
      if r and (not self.reward or r < self.reward) then
	 ship, self.due, self.reward, path = s, t, r, p
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
   for i = 1, #path - 1 do
      local c, n, w = path[i], path[i+1]
      if verbose then
	 print (i..":"..c.node.name.."->"..n.node.name.."("..c.state.." "..
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

local Client = {}

function Client:init()
   self.female = Engine.rand:Integer (1) == 1
   self.name = NameGen.FullName (self.female)
   self.seed = Engine.rand:Integer()
end

local function makeAdvert (station)
   local flavours = Translate:GetFlavours ('DeliverPackage')
   local flavour = Engine.rand:Integer (1, #flavours)
   flavours = flavours[flavour]
   local urgency = flavours.urgency
   local risk = flavours.risk
   local ctr = makeInst (flavours.localdelivery == 1 and LocalCtr or
			 flavours.localdelivery ~= 1 and ExternalCtr)

   if not ctr:init (station, urgency, risk) then
      return
   end
   local ad = {
      client   = makeInst (Client),
      board    = station,
      ctr      = ctr,
      flavour  = flavour,
      risk     = risk,
      urgency  = urgency }
   ad = makeInst (Advert, ad)
   ad.client:init()

   ads[station:AddAdvert (ad:desc(), onChat, onDelete)] = ad
   return ad
end

local function onCreateBB (station)
   if station:DistanceTo (Game.player) > 0.02 * AU then
      return
   end
   local num = Engine.rand:Integer(0, math.ceil(Game.system.population))
   for i = 1,num do
      makeAdvert (station)
   end
end

local function onUpdateBB (station)
   local delivery_flavours = Translate:GetFlavours('DeliverPackage')
   for ref, ad in pairs (ads) do
      if ad.ctr.expiry < Game.time or
	 ad.ctr.isLocal and ad.board == station and not ad.ctr:update() then
	 ad.board:RemoveAdvert (ref)
      end
   end
   if station:DistanceTo (Game.player) > 0.02 * AU then
      return
   end
   local ad = makeAdvert (station)
   if ad and station == Game.player:GetDockedWith() then
      UI.ImportantMessage (ad.ctr.isLocal and "Local" or "Non Local")
   end
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
   ads, missions = {}, {}
   if not loaded_data then
      return
   end
   for k, ad in pairs (loaded_data.ads) do
      makeInst (Advert, ad)
      makeInst (ad.isLocal and LocalCtr or ExternalCtr, ad.ctr)
      makeInst (Client, ad.client)
      local ref = ad.board:AddAdvert (ad:desc(), onChat, onDelete)
      ads[ref] = ad
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
