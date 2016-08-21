local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')

local player
local system
local pigui = Engine.pigui

local show_retrograde_indicators = true
local show_nav_distance_with_reticule = true
local show_nav_speed_with_reticule = true
local show_frame_speed_with_reticule = true


local center
local mission_selected
local navball_center
local navball_radius = 80
local reticule_radius = 80

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4
local two_pi = pi * 2

local colors = {
	 darkgreen = {r=0, g=150, b=0},
	 lightgreen = {r=0, g=255, b=0},
	 deltav_total = {r=100,g=100,b=100,a=200},
	 deltav_remaining = {r=250,g=250,b=250},
	 deltav_current = {r=150,g=150,b=150},
	 deltav_maneuver = {r=168,g=168,b=255},
	 darkgrey = {r=150,g=150,b=150},
	 orbital_marker = {r=150,g=150,b=150},
	 lightgrey = {r=200,g=200,b=200},
	 windowbg = {r=0,g=0,b=50,a=200},
	 transparent = {r=0,g=0,b=0,a=0},
	 red = { r=255, g=0, b=0 }
}

local MyFormat = {
	 Distance = function(distance)
			local d = math.abs(distance)
			if d < 1000 then
				 return math.floor(distance), "m"
			end
			if d < 1000*1000 then
				 return string.format("%0.1f", distance / 1000), "km"
			end
			if d < 1000*1000*1000 then
				 return string.format("%0.1f", distance / 1000 / 1000), "Mm"
			end
			return string.format("%0.1f", distance / 1.4960e11), "AU"
	 end
}

local Vector = {}
do
	 local meta = {
			_metatable = "Private metatable",
			_DESCRIPTION = "Vectors in 3D"
	 }

	 meta.__index = meta

	 function meta:__add( v )
			if(type(v) == "number") then
				 return Vector(self.x + v, self.y + v, self.z + v)
			else
				 return Vector(self.x + v.x, self.y + v.y, self.z + v.z)
			end
	 end

	 function meta:__sub( v )
			if(type(v) == "number") then
				 return Vector(self.x - v, self.y - v, self.z - v)
			else
				 return Vector(self.x - v.x, self.y - v.y, self.z - v.z)
			end
	 end

	 function meta:__mul( v )
			if(type(v) == "number") then
				 return Vector(self.x * v, self.y * v, self.z * v)
			else
				 return Vector(self.x * v.x, self.y * v.y, self.z * v.z)
			end
	 end

	 function meta:__div( v )
			if(type(v) == "number") then
				 return Vector(self.x / v, self.y / v, self.z / v)
			else
				 return Vector(self.x / v.x, self.y / v.y, self.z / v.z)
			end
	 end

	 function meta:__tostring()
			return ("<%g, %g, %g>"):format(self.x, self.y, self.z)
	 end

	 function meta:magnitude()
			return math.sqrt( self.x * self.x + self.y * self.y + self.z * self.z)
	 end

	 function meta:normalized()
			local len = math.abs(self:magnitude())
			return Vector(self.x / len, self.y / len, self.z / len)
	 end

	 function meta:left()
			return Vector(-self.y, self.x, 0)
	 end

	 function meta:right()
			return Vector(self.y, -self.x, 0)
	 end

	 function meta:dot(other)
			return self.x * other.x + self.y * other.y + self.z * other.z
	 end

	 function meta:cross(other)
			return Vector(self.y * other.z - self.z * other.y,
										self.x * other.z - self.z * other.x,
										self.x * other.y - self.y * other.x)
	 end

	 setmetatable( Vector, {
										__call = function( V, x ,y ,z ) return setmetatable( {x = x or 0, y = y or 0, z = z or 0}, meta ) end
	 } )
end

Vector.__index = Vector

function map(func, array)
	 local new_array = {}
	 for i,v in ipairs(array) do
			new_array[i] = func(v)
	 end
	 return new_array
end

function print_r ( t )
   local print_r_cache={}
   local function sub_print_r(t,indent)
      if (print_r_cache[tostring(t)]) then
         print(indent.."*"..tostring(t))
      else
         print_r_cache[tostring(t)]=true
         if (type(t)=="table") then
            for pos,val in pairs(t) do
               if (type(val)=="table") then
                  print(indent.."["..pos.."] => "..tostring(t).." {")
                  sub_print_r(val,indent..string.rep(" ",string.len(pos)+8))
                  print(indent..string.rep(" ",string.len(pos)+6).."}")
               elseif (type(val)=="string") then
                  print(indent.."["..pos..'] => "'..val..'"')
               else
                  print(indent.."["..pos.."] => "..tostring(val))
               end
            end
         else
            print(indent..tostring(t))
         end
      end
   end
   if (type(t)=="table") then
      print(tostring(t).." {")
      sub_print_r(t,"  ")
      print("}")
   else
      sub_print_r(t,"  ")
   end
   print()
end
print("****************************** PIGUI *******************************")


local function markerPos(name, distance)
	 local side, dir, pos = pigui.GetHUDMarker(name)
	 local point = center + Vector(dir.x, dir.y) * distance
	 if side == "hidden" then
			return nil
	 end
	 -- don't show if inside reticule
	 if Vector(pos.x - center.x, pos.y - center.y):magnitude() < distance then
			return nil, Vector(dir.x, dir.y), pos, side
	 else
			return point, Vector(dir.x, dir.y), pos, side
	 end
end

function show_missions()
	 local windowbg = colors.windowbg
	 local SpaceStation = import("SpaceStation")
	 local Game = import('Game')
	 local Space = import('Space')
	 local Format = import('Format')
	 local station = player:GetDockedWith()
	 if not station then
			return
	 end
	 pigui.PushStyleColor("WindowBg", windowbg)
	 pigui.SetNextWindowSize(Vector(pigui.screen_width / 2, pigui.screen_height / 1.5), "FirstUseEver")
	 pigui.Begin("Missions", {})
	 pigui.Columns(2, "missionscolumns", true)
	 pigui.BeginChild("foo");
	 --	 pigui.BeginGroup()
	 for k,v in pairs(station.adverts[station]) do
			pigui.BeginGroup()
			pigui.Text(v.description)
			pigui.Text(v.payout and Format.Money(v.payout) or "-")
			pigui.SameLine()
			if v.system
			then
				 if v.system.index == Game.system.index
						and v.system.sector.x == Game.system.sector.x
						and v.system.sector.y == Game.system.sector.y
						and v.system.sector.z == Game.system.sector.z
				 then
						pigui.Text(Format.Distance(Space.GetBody(v.body.index):DistanceTo(player)))
				 else
						pigui.Text(v.system:DistanceTo(Game.system) .. "ly")
				 end
			else
				 pigui.Text("-")
			end
			pigui.SameLine()
			pigui.Text(v.deadline and Format.Duration(v.deadline - Game.time) or "-")
			--			pigui.Separator()
			pigui.EndGroup()
			if pigui.IsItemClicked(0) then
				 mission_selected = v
			end
			pigui.Dummy(Vector(0,20))
	 end
	 --	 pigui.EndGroup()
	 pigui.EndChild()
	 pigui.NextColumn()
	 if mission_selected then
			pigui.BeginChild("bar")
			pigui.Columns(2, "desccolumns", false)
			local m = mission_selected
			pigui.PushFont("pionillium", 18)
			pigui.Text(m.description)
			pigui.PopFont()
			pigui.Spacing()
			if m.details then
				 pigui.TextWrapped(m.details)
			end
			pigui.Spacing()
			local distance
			if m.system then
				 if m.system.index == Game.system.index
						and m.system.sector.x == Game.system.sector.x
						and m.system.sector.y == Game.system.sector.y
						and m.system.sector.z == Game.system.sector.z
				 then
						distance = Format.Distance(Space.GetBody(m.body.index):DistanceTo(player))
				 else
						distance = math.floor(m.system:DistanceTo(Game.system) * 10) / 10 .. "ly"
				 end
			end

			pigui.Text("Destination: " .. (m.system and m.system.name or "-") .. ", " .. (m.body and m.body.name or "-") .. " (" .. distance .. ")")
			pigui.Text("Deadline: " .. Format.Date(m.deadline) .. " (" .. (m.deadline and Format.Duration(m.deadline - Game.time) or "-") .. ")")
			pigui.Text("Wage: " .. (m.payout and Format.Money(m.payout) or "-"))
			pigui.Spacing()

			pigui.NextColumn()
			pigui.Dummy(Vector(100,100))
			pigui.Text("Image")
			pigui.PushFont("pionillium", 18)
			pigui.Text(m.client or "Anonymous")
			pigui.PopFont()
			pigui.PushItemWidth(-1)
			if pigui.Button("OK, I'll do it.") then
				 print("cool")
			end
			if pigui.Button("Hang up") then
				 mission_selected = nil
				 print("then not")
			end
		  pigui.PopItemWidth()
			pigui.EndChild()
	 end
	 pigui.End()
	 pigui.PopStyleColor(1)
end

local selected

local function show_settings()
	 pigui.Begin("Settings", {})
	 show_retrograde_indicators = pigui.Checkbox("Show retrograde indicators", show_retrograde_indicators);
	 show_nav_distance_with_reticule = pigui.Checkbox("Show nav distance with reticule", show_nav_distance_with_reticule);
	 show_nav_speed_with_reticule = pigui.Checkbox("Show nav speed with reticule", show_nav_speed_with_reticule);
	 show_frame_speed_with_reticule = pigui.Checkbox("Show frame speed with reticule", show_frame_speed_with_reticule);
	 pigui.End()
end

-- ratio is 1.0 for full, 0.0 for empty
local function deltav_gauge(ratio, center, radius, color, thickness)
	 if ratio < 0 then
			ratio = 0
	 end
	 if ratio > 0 and ratio < 0.001 then
			ratio = 0.001
	 end
	 if ratio > 1 then
			ratio = 1
	 end
	 pigui.PathArcTo(center, radius + thickness / 2, pi_2 + pi_4, pi_2 + pi_4 + pi_2 * ratio, 64)
	 pigui.PathStroke(color, false, thickness)
end

local function drawWithUnit(leftCenter, number, unit, color, centerIsActuallyRight, prefix)
	 local magic_number = 14 -- magic number to offset the unit to baseline-align with the number. Should be fixed somehow :-/
	 pigui.PushFont("pionillium",36)
	 local numbersize = pigui.CalcTextSize(number)
	 pigui.PopFont()
	 pigui.PushFont("pionillium",18)
	 local unitsize = pigui.CalcTextSize(unit)
	 local prefixsize = prefix and pigui.CalcTextSize(prefix) or Vector(0,0)
	 pigui.PopFont()

	 if centerIsActuallyRight then
			leftCenter.x = leftCenter.x - numbersize.x - unitsize.x - prefixsize.x - 3 + (prefix and -3 or 0)
	 end
	 if prefix then
			pigui.PushFont("pionillium", 18)
			local topLeft = leftCenter + Vector(0, magic_number - numbersize.y / 2)
			pigui.AddText(topLeft, color, prefix)
			pigui.PopFont()
	 end

	 pigui.PushFont("pionillium", 36)
	 local topLeft = leftCenter + Vector(prefixsize.x + (prefix and 3 or 0), - numbersize.y / 2)
	 pigui.AddText(topLeft, color, number)
	 --	 pigui.AddQuad(topLeft, topLeft + Vector(0,numbersize.y), topLeft + Vector(numbersize.x, numbersize.y), topLeft + Vector(numbersize.x, 0), colors.darkgrey, 1.0)
	 pigui.PopFont()

	 pigui.PushFont("pionillium", 18)
	 local xtopLeft = topLeft + Vector(numbersize.x + 3, magic_number)
	 pigui.AddText(xtopLeft, color, unit)
	 --	 pigui.AddQuad(xtopLeft, xtopLeft + Vector(0,unitsize.y), xtopLeft + Vector(unitsize.x, unitsize.y), xtopLeft + Vector(unitsize.x, 0), colors.darkgrey, 1.0)
	 pigui.PopFont()
end

local function show_navball()
	 pigui.AddCircle(navball_center, navball_radius, colors.lightgrey, 128, 1.0)
	 pigui.AddText(Vector(navball_center.x, navball_center.y + navball_radius + 5), colors.lightgrey, "R: 100km")
	 local thickness = 10
	 local deltav_max = player:GetMaxDeltaV()
	 local deltav_remaining = player:GetRemainingDeltaV()
	 local dvr = deltav_remaining / deltav_max
	 local deltav_maneuver = 0.0
	 local dvm = deltav_maneuver / deltav_max
	 local deltav_current = player:GetCurrentDeltaV()
	 local dvc = deltav_current / deltav_max
	 -- -- debugging
	 -- pigui.Begin("DeltaV", {})
	 -- pigui.Text("delta v max: " .. deltav_max)
	 -- pigui.Text("delta v remaining: " .. deltav_remaining)
	 -- pigui.Text("delta v current: " .. deltav_current)
	 -- pigui.End()
	 deltav_gauge(1.0, navball_center, navball_radius + 5, colors.deltav_total, thickness)
	 if dvr > 0 then
			deltav_gauge(dvr, navball_center, navball_radius + 5, colors.deltav_remaining, thickness)
	 end
	 if dvm > 0 then
			deltav_gauge(dvm, navball_center, navball_radius + 5 + thickness / 4, colors.deltav_maneuver, thickness / 2)
	 end
	 if dvc > 0 then
			deltav_gauge(dvc, navball_center, navball_radius + 5 + thickness, colors.deltav_current, thickness)
	 end
	 -- pigui.AddText(navball_center + Vector(- navball_radius - 150, -50), colors.lightgrey, 'dvc: ' .. deltav_current)
	 -- pigui.AddText(navball_center + Vector(- navball_radius - 150, -30), colors.lightgrey, 'dvr: ' .. deltav_remaining)
	 -- pigui.AddText(navball_center + Vector(- navball_radius - 150, -10), colors.lightgrey, 'dvm: ' .. deltav_max)
	 --   local spd,unit = MyFormat.Distance(deltav_current)
	 --	 drawWithUnit(navball_center + Vector(- navball_radius * 1.25, - navball_radius / 2), spd, unit .. "/s", colors.lightgrey, true)
	 local spd,unit = MyFormat.Distance(deltav_remaining)
	 drawWithUnit(navball_center + Vector(- navball_radius * 1.5, 0), spd, unit .. "/s", colors.lightgrey, true, "Δv")
	 pigui.AddText(navball_center + Vector(- navball_radius * 1.5 - 100, 20), colors.lightgrey, math.floor(dvr*100) .. "%")
end

local function square(x)
	 return x * x
end

local function cube(x)
	 return x * x * x
end

local function show_debug()
end

-- based on http://space.stackexchange.com/questions/1904/how-to-programmatically-calculate-orbital-elements-using-position-velocity-vecto and https://github.com/RazerM/orbital/blob/0.7.0/orbital/utilities.py#L252
local function show_orbit()
	 pigui.Begin("Orbit", {})
	 -- pigui.Text("Self")
	 -- pigui.Text("Time: " .. Game.time)
	 local o_eccentricity, o_semimajoraxis, o_inclination, o_period, o_time_at_apoapsis, o_apoapsis, o_time_at_periapsis, o_periapsis = player:GetOrbit()
	 -- local p = player:GetPosition()
	 -- local pos = Vector(p.x, p.y, p.z)
	 -- pigui.Text("pos: " .. pos:magnitude() .. ", " .. math.floor(pos.x) .. "/" .. math.floor(pos.y) .. "/" .. math.floor(pos.z))
	 -- local v = player:GetVelocity()
	 -- local vel = Vector(v.x, v.y, v.z)
	 -- pigui.Text("vel: " .. vel:magnitude() .. ", " .. math.floor(vel.x) .. "/" .. math.floor(vel.y) .. "/" .. math.floor(vel.z))
	 -- local G = 6.674e-11
	 -- local M = player:GetFrame():GetMass()
	 -- pigui.Text("frame mass: " .. math.floor(M))
	 -- local mu = G * M
	 -- local eccentricity_vector = (pos * (square(vel:magnitude()) - mu / pos:magnitude()) - vel * (pos:dot(vel))) / mu
	 -- local eccentricity = eccentricity_vector:magnitude()
	 -- pigui.Text("eccentricity: " .. eccentricity)
	 pigui.Text("o eccentricity: " .. o_eccentricity)
	 --	 local specific_orbital_energy = square(vel:magnitude()) / 2 - mu / pos:magnitude()
	 --	 pigui.Text("specific orbital energy: " .. specific_orbital_energy)
	 --	 local semimajoraxis = -mu / 2 * specific_orbital_energy
	 -- local semimajoraxis = 1 / (2 / pos:magnitude() - square(vel:magnitude()) / mu) -- based on http://orbitsimulator.com/formulas/OrbitalElements.html
	 -- pigui.Text("semi-major axis: " .. Format.Distance(semimajoraxis))
	 pigui.Text("o semi-major axis: " .. Format.Distance(o_semimajoraxis))
	 -- local angular_momentum = pos:cross(vel)
	 -- local inclination = math.acos(angular_momentum.z / angular_momentum:magnitude())
	 -- pigui.Text("inclination: " .. math.floor(inclination / two_pi * 360))
	 pigui.Text("O inclination: " .. math.floor(o_inclination / two_pi * 360))
	 -- local periapsis = semimajoraxis * (1 - eccentricity)
	 -- local apoapsis = semimajoraxis * (1 + eccentricity)
	 -- pigui.Text("Periapsis: " .. Format.Distance(periapsis) .. ", Apoapsis: " .. Format.Distance(apoapsis))
	 local frame = player:GetFrame():GetSystemBody()
	 local pa = Vector(o_periapsis.x, o_periapsis.y, o_periapsis.z):magnitude()
	 local aa = Vector(o_apoapsis.x, o_apoapsis.y, o_apoapsis.z):magnitude()
	 pigui.Text("O Periapsis: " .. Format.Distance(pa) .. " (" .. Format.Distance(pa - frame.radius) .. "), O Apoapsis: " .. Format.Distance(aa) .. " (" .. Format.Distance(aa - frame.radius) .. ")")
	 -- 	 local Ex = 1 - pos:magnitude() / semimajoraxis
	 -- 	 local Ey = vel:dot(pos) / math.sqrt(semimajoraxis * mu)
	 -- 	 --	 local eccentric_anomaly = math.atan(Ey, Ex) -- WRONG
	 -- 	 -- based on http://space.stackexchange.com/questions/16891/how-to-calculate-the-time-to-apoapsis-periapsis-given-the-orbital-elements
	 -- 	 local period = two_pi * math.sqrt(cube(semimajoraxis) / mu)
	 -- 	 local true_anomaly = math.acos(eccentricity_vector:dot(pos) / (eccentricity * pos:magnitude()))
	 -- 	 -- local mean_anomaly = eccentric_anomaly - eccentricity * math.sin(eccentric_anomaly)  -- WRONG
	 -- 	 -- local eccentric_anomaly = math.atan(math.sqrt(1 - square(eccentricity)) * math.sin(true_anomaly), eccentricity + math.cos(true_anomaly))
	 -- 	 -- eccentric_anomaly = math.fmod(eccentric_anomaly, two_pi)
	 -- 	 -- works, but strange
	 -- 	 if pos:dot(vel) < 0 then
	 -- 			true_anomaly = two_pi - true_anomaly
	 -- 	 end
	 -- 	 local eccentric_anomaly = math.fmod(math.acos((eccentricity + math.cos(true_anomaly)) / (1 + eccentricity * math.cos(true_anomaly))), two_pi)
	 -- 	 if true_anomaly > pi then
	 -- 			eccentric_anomaly = two_pi - eccentric_anomaly
	 -- 	 end
	 -- 	 local mean_anomaly = math.fmod(eccentric_anomaly - eccentricity * math.sin(eccentric_anomaly), two_pi)
	 -- 	 local n = period / two_pi
	 -- 	 local to_periapsis = math.fmod(two_pi - mean_anomaly, two_pi) * n
	 -- 	 local to_apoapsis = math.fmod(two_pi + pi - mean_anomaly, two_pi) * n
	 -- --	 pigui.Text("Period: " .. Format.Duration(period) .. ", true anom: " .. math.floor(true_anomaly / two_pi * 360) .. ", ecc anom: " .. math.floor(eccentric_anomaly / two_pi * 360) .. ", mean anom: " .. math.floor(mean_anomaly / two_pi * 360))
	 -- 	 pigui.Text("Period: " .. Format.Duration(period) .. ", mean anom: " .. math.floor(mean_anomaly / two_pi * 360))
	 pigui.Text("O Period: " .. (o_period and Format.Duration(o_period) or "none"))
	 --	 pigui.Text("Time to periapsis: " .. Format.Duration(to_periapsis) .. ", time to apoapsis: " .. Format.Duration(to_apoapsis))
	 pigui.Text("Time to O periapsis: " .. Format.Duration(o_time_at_periapsis) .. ", O time to apoapsis: " .. Format.Duration(o_time_at_apoapsis))
	 pigui.Text("Has Atmosphere: " .. (frame.hasAtmosphere and "yes" or "no"))
	 pigui.Text("Atmosphere radius: " .. frame.atmosphereRadius)
	 pigui.End()
end
local function show_nav_window()
	 	 -- ******************** Navigation Window ********************
	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.PushStyleColor("WindowBg", colors.windowbg)
	 pigui.Begin("Navigation", {})
	 pigui.Columns(2, "navcolumns", false)
	 local body_paths = system:GetBodyPaths()
	 -- create intermediate structure
	 local data = map(function(system_path)
				 local system_body = system_path:GetSystemBody()
				 local body = Space.GetBody(system_body.index)
				 local distance = player:DistanceTo(body)
				 return { systemBody = system_body, body = body, distance = distance, name = system_body.name }
										end,
			body_paths)
	 -- sort by distance
	 table.sort(data, function(a,b) return a.distance < b.distance end)
	 -- display
	 for key,data in pairs(data) do
			if(pigui.Selectable(data.name, selected == data.body, {"SpanAllColumns"})) then
				 selected = data.body
				 player:SetNavTarget(data.body)
			end
			pigui.NextColumn()
			pigui.Text(Format.Distance(data.distance))
			pigui.NextColumn()
	 end
	 pigui.End()
	 pigui.PopStyleColor(1)
end

pigui.handlers.HUD = function(delta)
	 player = Game.player
	 system = Game.system
	 center = Vector(pigui.screen_width/2, pigui.screen_height/2)
	 navball_center = Vector(center.x, pigui.screen_height - 25 - navball_radius)
	 local windowbg = colors.windowbg
	 -- transparent full-size window, no inputs
	 pigui.SetNextWindowPos(Vector(0, 0), "Always")
	 pigui.SetNextWindowSize(Vector(pigui.screen_width, pigui.screen_height), "Always")
	 pigui.PushStyleColor("WindowBg", colors.transparent)
	 pigui.Begin("HUD", {"NoTitleBar","NoInputs","NoMove","NoResize","NoSavedSettings","NoFocusOnAppearing","NoBringToFrontOnFocus"})
	 -- ******************** Ship Directional Markers ********************
	 local size=8
	 local side, dir, pos = pigui.GetHUDMarker("forward")
	 local pos_fwd = pos
	 local side_fwd = side
	 local dir_fwd = Vector(dir.x, dir.y)
	 local show_extra_reticule = true
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos - Vector(size,0), pos + Vector(size,0), colors.lightgrey, 3.0)
			pigui.AddLine(pos - Vector(0,size), pos + Vector(0,size), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("backward")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos - Vector(size,size), pos + Vector(size,size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(size,-size), pos + Vector(-size,size), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("left")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("right")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(-size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("up")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("down")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_extra_reticule = false
			end
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("normal")
	 if side == "onscreen" then
			local factor = 2
			local lineLeft = pos + Vector(-1,-1) * size
			local lineRight = pos + Vector(1,-1) * size
			local triCenter = pos + Vector(0,1) * (size / factor)
			local triLeft = lineLeft + Vector(0, 1) * (size / factor)
			local triRight = lineRight + Vector(0, 1) * (size / factor)
			pigui.AddLine(lineLeft, lineRight, colors.orbital_marker, 2.0)
			pigui.AddLine(triLeft, triCenter , colors.orbital_marker, 2.0)
			pigui.AddLine(triRight, triCenter , colors.orbital_marker, 2.0)
	 end

	 local side, dir, pos = pigui.GetHUDMarker("anti_normal")
	 if side == "onscreen" then
			local factor = 2
			local lineLeft = pos + Vector(-1,1) * size
			local lineRight = pos + Vector(1,1) * size
			local triCenter = pos + Vector(0,-1) * (size / factor)
			local triLeft = lineLeft + Vector(0, -1) * (size / factor)
			local triRight = lineRight + Vector(0, -1) * (size / factor)
			pigui.AddLine(lineLeft, lineRight, colors.orbital_marker, 2.0)
			pigui.AddLine(triLeft, triCenter , colors.orbital_marker, 2.0)
			pigui.AddLine(triRight, triCenter , colors.orbital_marker, 2.0)
	 end

	 local side, dir, pos = pigui.GetHUDMarker("radial_in")
	 if side == "onscreen" then
			local factor = 6
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			local leftCenter = pos + Vector(-1, 0) * (size / factor)
			local rightCenter = pos + Vector(1, 0) * (size / factor)
			pigui.AddLine(leftTop, leftBottom, colors.orbital_marker, 2.0)
			pigui.AddLine(leftBottom, leftCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(leftTop, leftCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(rightTop, rightBottom, colors.orbital_marker, 2.0)
			pigui.AddLine(rightBottom, rightCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(rightTop, rightCenter, colors.orbital_marker, 2.0)
	 end

	 local side, dir, pos = pigui.GetHUDMarker("radial_out")
	 if side == "onscreen" then
			local factor = 5
			local leftTop = pos + Vector(-1 * size / factor, 1 * size)
			local rightTop = pos + Vector(1 * size / factor, 1 * size)
			local leftBottom = pos + Vector(-1 * size / factor, -1 * size)
			local rightBottom = pos + Vector(1 * size / factor, -1 * size)
			local leftCenter = pos + Vector(-1, 0) * size
			local rightCenter = pos + Vector(1, 0) * size
			pigui.AddLine(leftTop, leftBottom, colors.orbital_marker, 2.0)
			pigui.AddLine(leftBottom, leftCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(leftTop, leftCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(rightTop, rightBottom, colors.orbital_marker, 2.0)
			pigui.AddLine(rightBottom, rightCenter, colors.orbital_marker, 2.0)
			pigui.AddLine(rightTop, rightCenter, colors.orbital_marker, 2.0)
	 end

	 -- ******************** Reticule ********************

	 if show_extra_reticule then
			-- center of screen marker, small circle
			pigui.AddCircleFilled(center, 2, colors.lightgrey, 8)
			-- pointer to forward, small triangle
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:left()):normalized() * size / 1.7, colors.lightgrey, 1.5)
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:right()):normalized() * size / 1.7, colors.lightgrey, 1.5)
	 end
	 -- navigation circle
	 pigui.AddCircle(center, reticule_radius, colors.lightgrey, 128, 2.0)

	 local navTarget = player:GetNavTarget()
	 if navTarget then
			-- target name
			pigui.PushFont("pionillium", 12)
			local leftTop = Vector(center.x + reticule_radius / 2 * 1.3, center.y - reticule_radius)
			pigui.AddText(leftTop, colors.darkgreen, "Target")
			pigui.PopFont()
			leftTop = leftTop + Vector(20,20)
			pigui.PushFont("pionillium", 18)
			pigui.AddText(leftTop, colors.lightgreen, navTarget.label)
			pigui.PopFont()

			local speed = pigui.GetVelocity("nav_prograde")
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			drawWithUnit(Vector(center.x + reticule_radius + 10, center.y), spd, unit .. "/s", colors.lightgreen)

			local distance = player:DistanceTo(navTarget)
			local dist,unit = MyFormat.Distance(distance)
			drawWithUnit(Vector(center.x + reticule_radius / 2 * 1.7, center.y + reticule_radius / 2 * 1.7), dist, unit, colors.lightgreen)
			local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
			pigui.PushFont("pionillium", 18)
			pigui.AddText(Vector(center.x + reticule_radius / 2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgreen, "~" .. Format.Distance(brakeDist))
			pigui.PopFont()
	 end

	 local frame = player:GetFrame()
	 if frame then
			pigui.PushFont("pionillium", 12)
			local size = pigui.CalcTextSize("rel-to")
			local rightTop = Vector(center.x - reticule_radius / 2 * 1.3 - size.x, center.y - reticule_radius)
			pigui.AddText(rightTop, colors.darkgrey, "rel-to")
			pigui.PopFont()
			pigui.PushFont("pionillium", 18)
			local size = pigui.CalcTextSize(frame.label)
			rightTop = rightTop + Vector(-size.x,20)
			pigui.AddText(rightTop, colors.darkgrey, frame.label)
			pigui.PopFont()

			local distance = player:DistanceTo(frame)
			local dist,unit = MyFormat.Distance(distance)
			drawWithUnit(Vector(center.x - reticule_radius / 2 * 1.7, center.y + reticule_radius / 2 * 1.7), dist, unit, colors.lightgrey, true)

			local brakeDist = player:GetDistanceToZeroV("frame", "retrograde")
			pigui.PushFont("pionillium", 18)
			pigui.AddText(Vector(center.x - reticule_radius /2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgrey, "~" .. Format.Distance(brakeDist))
			pigui.PopFont()

			local speed = pigui.GetVelocity("frame_prograde")
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			drawWithUnit(Vector(center.x - reticule_radius - 10, center.y), spd, unit .. "/s", colors.lightgrey, true)

			-- ******************** GPS data ********************
			do
				 local pressure, density = frame:GetAtmosphericState()
				 if pressure > 0.001 then
						drawWithUnit(Vector(center.x - 100, center.y + reticule_radius + 50), math.floor(pressure*100)/100, "atm", colors.darkgrey, false, "Pressure: ")
				 end
			end
			do
				 local alt, vspd, lat, lon = player:GetGPS()
				 if alt and alt < 10000000 then
						local altitude,unit = MyFormat.Distance(alt)
						local offset = 28
						drawWithUnit(Vector(center.x - 100, center.y + reticule_radius + 50 + offset), altitude, unit, colors.darkgrey, false, "Altitude: ")
						local vspeed,unit = MyFormat.Distance(vspd)
						drawWithUnit(Vector(center.x - 100, center.y + reticule_radius + 50 + offset * 2), vspeed, unit .. "/s", colors.darkgrey, false, "Vertical Speed: ")
						drawWithUnit(Vector(center.x - 100, center.y + reticule_radius + 50 + offset * 3), lat, "", colors.darkgrey, false, "Lat: ")
						drawWithUnit(Vector(center.x - 100, center.y + reticule_radius + 50 + offset * 4), lon, "", colors.darkgrey, false, "Lon: ")
				 end
			end

			-- thrusters
			pigui.Begin("Thrusters", {})
			local thrust = player:GetThrusterState()
			-- y = 1 -> up
			-- z = -1 -> fwd
			-- x = -1 -> left
			local thrust_forward = (thrust.z < 0) and math.abs(thrust.z) * math.abs(player:GetAccel("forward")) or 0
			local thrust_backward = (thrust.z > 0) and math.abs(thrust.z) * math.abs(player:GetAccel("backward")) or 0
			local thrust_left = (thrust.x < 0) and math.abs(thrust.x) * math.abs(player:GetAccel("left")) or 0
			local thrust_right = (thrust.x > 0) and math.abs(thrust.x) * math.abs(player:GetAccel("right")) or 0
			local thrust_up = (thrust.y > 0) and math.abs(thrust.y) * math.abs(player:GetAccel("up")) or 0
			local thrust_down = (thrust.y < 0) and math.abs(thrust.y) * math.abs(player:GetAccel("down")) or 0
			pigui.Text("Thrust: " .. thrust.x .. "/" .. thrust.y .. "/" .. thrust.z)
			pigui.Text("Accel Forward: " .. player:GetAccel("forward"))
			pigui.Text("Accel Backward: " .. player:GetAccel("backward"))
			pigui.Text("Accel Up: " .. player:GetAccel("up"))
			pigui.Text("Accel Down: " .. player:GetAccel("down"))
			pigui.Text("Accel Left: " .. player:GetAccel("left"))
			pigui.Text("Accel Right: " .. player:GetAccel("right"))
			pigui.Text("Thrust Forward: " .. thrust_forward)
			pigui.Text("Thrust Backward: " .. thrust_backward)
			pigui.Text("Thrust Up: " .. thrust_up)
			pigui.Text("Thrust Down: " .. thrust_down)
			pigui.Text("Thrust Left: " .. thrust_left)
			pigui.Text("Thrust Right: " .. thrust_right)
			local total_thrust = Vector(thrust_forward - thrust_backward,thrust_up - thrust_down, thrust_left - thrust_right):magnitude()
			pigui.Text("Total thrust: " .. total_thrust)
			pigui.End()
			-- ******************** Frame Prograde marker ********************
			local pos,dir,point,side = markerPos("frame_prograde", reticule_radius - 10)
			local color = colors.orbital_marker
			if pos then
				 local size = 4
				 local left = pos + Vector(-1,0) * size
				 local right = pos + Vector(1,0) * size
				 local top = pos + Vector(0,1) * size
				 local bottom = pos + Vector(0,-1) * size
				 pigui.AddQuad(left, top, right, bottom, color, 1.0)
			end
			if side == "onscreen" and point then
				 local size = 12
				 local left = point + Vector(-1,0) * size
				 local right = point + Vector(1,0) * size
				 local top = point + Vector(0,1) * size
				 local bottom = point + Vector(0,-1) * size
				 pigui.AddQuad(left, top, right, bottom, color, 3.0)
			end
			-- ******************** Away from Frame marker ********************
		  local pos,dir,point,side = markerPos("away_from_frame", reticule_radius - 10)
			local color = colors.orbital_marker
			if pos then
				 local size = 4
				 pigui.AddCircle(pos, size, color, 8, 1.0)
			end
			if side == "onscreen" and point then
				 local size = 12
				 pigui.AddCircle(point, size, color, 32, 3.0)
			end
			-- ******************** Frame Retrograde marker ********************
			local pos,dir,point,side = markerPos("frame_retrograde", reticule_radius - 10)
			local color = colors.orbital_marker
			if pos and show_retrograde_indicators then
				 local size = 3
				 local leftTop = pos + Vector(-1,1) * size
				 local rightTop = pos + Vector(1,1) * size
				 local leftBottom = pos + Vector(-1,-1) * size
				 local rightBottom = pos + Vector(1,-1) * size
				 pigui.AddLine(leftTop, rightBottom, color, 1.0)
				 pigui.AddLine(leftBottom, rightTop, color, 1.0)
			end
			if side == "onscreen" and point then
				 local size = 12
				 local leftTop = point + Vector(-1,1) * size
				 local rightTop = point + Vector(1,1) * size
				 local leftBottom = point + Vector(-1,-1) * size
				 local rightBottom = point + Vector(1,-1) * size
				 pigui.AddLine(leftTop, rightBottom, color, 3.0)
				 pigui.AddLine(leftBottom, rightTop, color, 3.0)
			end
	 end
	 -- ******************** NavTarget Prograde marker ********************
	 local pos,dir,point,side = markerPos("nav_prograde", reticule_radius - 10)
	 local color = colors.lightgreen
	 if pos then
			local size = 4
			local left = pos + Vector(-1,0) * size
			local right = pos + Vector(1,0) * size
			local top = pos + Vector(0,1) * size
			local bottom = pos + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, color, 1.0)
	 end
	 if side == "onscreen" and point then
			local size = 12
			local left = point + Vector(-1,0) * size
			local right = point + Vector(1,0) * size
			local top = point + Vector(0,1) * size
			local bottom = point + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, color, 3.0)
	 end
	 -- ******************** NavTarget Retrograde marker ********************
	 local pos,dir,point,side = markerPos("nav_retrograde", reticule_radius - 10)
	 local color = colors.lightgreen
	 if pos and show_retrograde_indicators then
			local size = 3
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			pigui.AddLine(leftTop, rightBottom, color, 1.0)
			pigui.AddLine(leftBottom, rightTop, color, 1.0)
	 end
	 if side == "onscreen" and point then
			local size = 12
			local leftTop = point + Vector(-1,1) * size
			local rightTop = point + Vector(1,1) * size
			local leftBottom = point + Vector(-1,-1) * size
			local rightBottom = point + Vector(1,-1) * size
			pigui.AddLine(leftTop, rightBottom, color, 3.0)
			pigui.AddLine(leftBottom, rightTop, color, 3.0)
	 end
	 -- ******************** NavTarget marker ********************
	 local pos,dir,point,side = markerPos("nav", reticule_radius + 5)
	 if pos then
			local size = 9
			local left = dir:left() * size + pos
			local right = dir:right() * size + pos
			local top = dir * size * 2 + pos
			pigui.AddTriangleFilled(left, right, top, colors.lightgrey)
	 end
	 local color = colors.lightgreen
	 if side == "onscreen" and point then
			local size = 16
			local leftTop = point + Vector(-1,1) * size
			local rightTop = point + Vector(1,1) * size
			local leftBottom = point + Vector(-1,-1) * size
			local rightBottom = point + Vector(1,-1) * size
			pigui.AddQuad(leftTop, leftBottom, rightBottom, rightTop, color, 3.0)
			local mp = pigui.GetMousePos()
			if (Vector(mp.x,mp.y) - point):magnitude() < 30 then
				 pigui.SetTooltip("Nav target\nThis shows the current navigational target")
			end
	 end
	 -- ******************** Frame indicator ********************
	 local pos,dir = markerPos("frame", reticule_radius + 5)
	 if pos then
			local size = 6
			local left = dir:left() * 4 + pos
			local right = dir:right() * 4 + pos
			local top = dir * 8 + pos
			pigui.AddTriangleFilled(left, right, top, colors.darkgrey)
	 end

	 show_navball()
	 -- ******************** Bodies on Screen ********************
	 local body_groups = {}
	 local cutoff = 5
	 for key,data in pairs(system:GetBodyPaths()) do
			local system_body = data:GetSystemBody()
			local body = Space.GetBody(system_body.index)
			if body then
				 local pos = body:GetProjectedScreenPosition()
				 if pos then
						local group = nil
						for _,bodies in pairs(body_groups) do
							 for p,b in pairs(bodies) do
									if Vector(p.x-pos.x,p.y-pos.y):magnitude() < cutoff then
										 group = bodies
									end
							 end
						end
						if group then
							 group[pos] = body
						else
							 local t = {}
							 t[pos] = body
							 body_groups[pos] = t
						end
				 end
			end
	 end
	 local labels
	 for pos,group in pairs(body_groups) do
			local size = 5
			pigui.AddCircleFilled(pos, size, colors.lightgrey, 8)
			local mp = pigui.GetMousePos()
			labels = {}
			for p,body in pairs(group) do
				 table.insert(labels, body)
			end
			table.sort(labels, function (a,b) return a.label < b.label end)
			local label = table.concat(map(function (a) return a.label end, labels), "\n")
			local show_tooltip = false
			if (Vector(mp.x - pos.x,mp.y - pos.y)):magnitude() < size then
				 if pigui.IsMouseReleased(0) then
						if #labels == 1 then
							 player:SetNavTarget(labels[1])
						else
							 pigui.OpenPopup("navtarget" .. label)
						end
				 else
						show_tooltip = true
				 end
			end
			if pigui.BeginPopup("navtarget" .. label) then
				 for _,body in pairs(labels) do
						if pigui.Selectable(body.label, false, {}) then
							 player:SetNavTarget(body)
						end
				 end
				 pigui.EndPopup()
			else
				 if show_tooltip then
						pigui.SetTooltip(label)
				 end
			end
	 end
	 -- ******************** directional spaceship markers ********************
	 do
			local vel = player:GetOrientedVelocity()
			local max = math.max(math.abs(vel.x),(math.max(math.abs(vel.y), math.abs(vel.z))))
			local size = 15
			local thickness = 5
			local velocity_center = Vector(50,50)

			if max < size then
				 max = size
			end

			local vx,vy,vz = vel.x/max, -vel.y/max, vel.z/max

			pigui.AddText(velocity_center + Vector(-13,-size*1.5-8), colors.darkgrey, "L")
			pigui.AddText(velocity_center + Vector(-13,size*1.5), colors.darkgrey, "R")
			pigui.AddLine(velocity_center + Vector(-10,0), velocity_center + Vector(-10, vx * size), colors.lightgrey, thickness)
			pigui.AddText(velocity_center + Vector(-3,-size*1.5-8), colors.darkgrey, "U")
			pigui.AddText(velocity_center + Vector(-3,size*1.5), colors.darkgrey, "D")
			pigui.AddLine(velocity_center + Vector(0,0), velocity_center + Vector(0, vy * size), colors.lightgrey, thickness)
			pigui.AddText(velocity_center + Vector(7,-size*1.5-8), colors.darkgrey, "F")
			pigui.AddText(velocity_center + Vector(7,size*1.5), colors.darkgrey, "B")
			pigui.AddLine(velocity_center + Vector(10,0), velocity_center + Vector(10, vz * size), colors.lightgrey, thickness)
	 end
	 pigui.End()
	 pigui.PopStyleColor(1);

	 show_nav_window()
	 --	 show_settings()
	 -- Missions, these should *not* be part of the regular HUD
	 --	 show_missions()
	 show_orbit()
end
