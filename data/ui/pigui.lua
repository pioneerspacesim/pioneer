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
local standard_gravity = 9.80665

local keys = {
	 f1 = 58,
	 f2 = 59,
	 f3 = 60,
	 f4 = 61
}

local colors = {
	 darkgreen = {r=0, g=150, b=0},
	 lightgreen = {r=0, g=255, b=0},
	 deltav_total = {r=100,g=100,b=100,a=200},
	 deltav_remaining = {r=250,g=250,b=250},
	 deltav_current = {r=150,g=150,b=150},
	 deltav_maneuver = {r=168,g=168,b=255},
	 darkgrey = {r=150,g=150,b=150},
	 darkergrey = {r=50,g=50,b=50},
	 orbital_marker = {r=150,g=150,b=150},
	 lightgrey = {r=200,g=200,b=200},
	 windowbg = {r=0,g=0,b=50,a=200},
	 transparent = {r=0,g=0,b=0,a=0},
	 lightred = { r=255, g=150, b=150},
	 red = { r=255, g=0, b=0 },
	 combat_target = { r=200, g=100, b=100 },
	 maneuver = { r=163, g=163, b=255 }
}

local fontsizes = {
	 large = { size = 30, offset = 10 },
	 medium = { size = 18, offset = 4 },
	 small = { size = 12, offset = 3 }
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


-- ****************************** PIGUI *******************************


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

local function drawWithUnit(leftCenter, number, numberfontsize, numbercolor, unit, unitfontsize, unitcolor, centerIsActuallyRight, prefix, prefixfontsize, prefixcolor)
	 local magic_number = numberfontsize.offset -- magic number to offset the unit to baseline-align with the number. Should be fixed somehow :-/
	 pigui.PushFont("pionillium",numberfontsize.size)
	 local numbersize = pigui.CalcTextSize(number)
	 pigui.PopFont()
	 pigui.PushFont("pionillium",unitfontsize.size)
	 local unitsize = pigui.CalcTextSize(unit)
	 local prefixsize = prefix and pigui.CalcTextSize(prefix) or Vector(0,0)
	 pigui.PopFont()
	 local totalsize = Vector(numbersize.x + unitsize.x + prefixsize.x + 3 + (prefix and 3 or 0), 0)
	 if centerIsActuallyRight then
			leftCenter.x = leftCenter.x - numbersize.x - unitsize.x - prefixsize.x - 3 + (prefix and -3 or 0)
	 end
	 if prefix then
			pigui.PushFont("pionillium", prefixfontsize.size)
			local topLeft = leftCenter + Vector(0, magic_number - numbersize.y / 2)
			pigui.AddText(topLeft, prefixcolor, prefix)
			pigui.PopFont()
	 end

	 pigui.PushFont("pionillium", numberfontsize.size)
	 local topLeft = leftCenter + Vector(prefixsize.x + (prefix and 3 or 0), - numbersize.y / 2)
	 pigui.AddText(topLeft, numbercolor, number)
	 --	 pigui.AddQuad(topLeft, topLeft + Vector(0,numbersize.y), topLeft + Vector(numbersize.x, numbersize.y), topLeft + Vector(numbersize.x, 0), colors.darkgrey, 1.0)
	 pigui.PopFont()

	 pigui.PushFont("pionillium", unitfontsize.size)
	 local xtopLeft = topLeft + Vector(numbersize.x + 3, magic_number)
	 pigui.AddText(xtopLeft, unitcolor, unit)
	 --	 pigui.AddQuad(xtopLeft, xtopLeft + Vector(0,unitsize.y), xtopLeft + Vector(unitsize.x, unitsize.y), xtopLeft + Vector(unitsize.x, 0), colors.darkgrey, 1.0)
	 pigui.PopFont()
	 return totalsize
end

local function xdrawWithUnit(center, number, unit, color, centerIsActuallyRight, prefix)
	 return drawWithUnit(center, number, fontsizes.large, color, unit, fontsizes.medium, color, centerIsActuallyRight, prefix, fontsizes.medium, color)
end

local function show_navball()
	 pigui.AddCircle(navball_center, navball_radius, colors.lightgrey, 128, 1.0)
	 pigui.AddText(Vector(navball_center.x, navball_center.y + navball_radius + 5), colors.lightgrey, "R: 100km")
	 local thickness = 10
	 local deltav_max = player:GetMaxDeltaV()
	 local deltav_remaining = player:GetRemainingDeltaV()
	 local dvr = deltav_remaining / deltav_max
	 local deltav_maneuver = player:GetManeuverSpeed() or 0
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

	 -- delta-v remaining
	 local spd,unit = MyFormat.Distance(deltav_remaining)
	 drawWithUnit(navball_center + Vector(- navball_radius * 1.5, 0)
								, spd , fontsizes.large , colors.lightgrey
								, unit .. "/s" , fontsizes.medium , colors.darkgrey
								, true
								, "Δv" , fontsizes.medium , colors.darkgrey)

	 pigui.AddText(navball_center + Vector(- navball_radius * 1.5 - 100, 20), colors.lightgrey, math.floor(dvr*100) .. "%")
	 if deltav_maneuver > 0 then
	 		local spd,unit = MyFormat.Distance(deltav_maneuver)
	 		drawWithUnit(navball_center + Vector(- navball_radius * 1.5, 50)
	 								 , spd , fontsizes.large , colors.maneuver
	 								 , unit .. "/s" , fontsizes.medium , colors.maneuver
	 								 , true
	 								 , "mΔv" , fontsizes.medium , colors.maneuver)
	 end


	 -- ******************** Orbital stats ********************
	 local o_eccentricity, o_semimajoraxis, o_inclination, o_period, o_time_at_apoapsis, o_apoapsis, o_time_at_periapsis, o_periapsis = player:GetOrbit()
	 local aa = Vector(o_apoapsis.x, o_apoapsis.y, o_apoapsis.z):magnitude()
	 local pa = Vector(o_periapsis.x, o_periapsis.y, o_periapsis.z):magnitude()
	 local frame = player:GetFrame()
	 local frame_radius = frame and frame:GetSystemBody().radius or 0
	 -- apoapsis
	 if not player:IsDocked() then
			local right_upper = navball_center + Vector(navball_radius * 1.2, -navball_radius * 0.9)
			local aa_d = aa - frame_radius
			local dist_apo, unit_apo = MyFormat.Distance(aa_d)
			if aa_d > 0 then
				 local xsize = drawWithUnit(right_upper
																		, dist_apo, fontsizes.medium, colors.lightgrey
																		, unit_apo, fontsizes.small, colors.darkgrey
																		, false
																		, "a", fontsizes.small, colors.darkgrey)
				 pigui.AddText(right_upper + xsize + Vector(10,-5), (o_time_at_apoapsis < 30 and colors.lightgreen or colors.lightgrey), "t-" .. Format.Duration(o_time_at_apoapsis))
			end
	 end
	 -- altitude
	 local alt, vspd, lat, lon = player:GetGPS()
	 local right_upper = navball_center + Vector(navball_radius * 1.3, -navball_radius * 0.6)
	 if alt and alt < 10000000 then
			local altitude,unit = MyFormat.Distance(alt)
			local xsize = drawWithUnit(right_upper
																 , altitude, fontsizes.large, colors.lightgrey
																 , unit, fontsizes.medium, colors.darkgrey
																 , false
																 , "alt", fontsizes.medium, colors.darkgrey)
			local vspeed, unit = MyFormat.Distance(vspd)
			drawWithUnit(right_upper + xsize + Vector(10, fontsizes.medium.offset)
									 , (vspd > 0 and "+" or "") .. vspeed, fontsizes.medium, (vspd < 0 and colors.lightred or colors.lightgreen)
									 , unit .. "/s", fontsizes.small, colors.darkgrey
									 , false)
	 end
	 -- periapsis
	 if not player:IsDocked() then
			local right_upper = navball_center + Vector(navball_radius * 1.4, -navball_radius * 0.25)
			local pa_d = pa - frame_radius
			local dist_per, unit_per = MyFormat.Distance(pa_d)
			if pa_d ~= 0 then
				 local xsize = drawWithUnit(right_upper
																		, dist_per, fontsizes.medium, (pa - frame_radius < 0 and colors.lightred or colors.lightgrey)
																		, unit_per, fontsizes.small, colors.darkgrey
																		, false
																		, "p", fontsizes.small, colors.darkgrey)
				 pigui.AddText(right_upper + xsize + Vector(10,-5), (o_time_at_periapsis < 30 and colors.lightgreen or colors.lightgrey), "t-" .. Format.Duration(o_time_at_periapsis))
			end
	 end
	 -- inclination, eccentricity
	 if not player:IsDocked() then
			local right_upper = navball_center + Vector(navball_radius * 1.5, -navball_radius * 0)
			local xsize = drawWithUnit(right_upper
																 , math.floor(o_inclination / two_pi * 360), fontsizes.medium, colors.lightgrey
																 , "°", fontsizes.medium, colors.lightgrey
																 , false
																 , "inc", fontsizes.small, colors.lightgrey)
			drawWithUnit(right_upper + xsize + Vector(10)
									 , math.floor(o_eccentricity * 100) / 100, fontsizes.medium, colors.lightgrey
									 , "", fontsizes.medium, colors.lightgrey
									 , false
									 , "ecc", fontsizes.small, colors.lightgrey)
	 end
	 -- latitude, longitude
	 local right_upper = navball_center + Vector(navball_radius * 1.2, navball_radius * 0.8)
	 if lat then
			drawWithUnit(right_upper
									 , lat, fontsizes.medium, colors.lightgrey
									 , "", fontsizes.small, colors.lightgrey
									 , false
									 , "Lat:", fontsizes.small, colors.darkgrey)
	 end
	 if lon then
			drawWithUnit(right_upper + Vector(0, 20)
									 , lon, fontsizes.medium, colors.lightgrey
									 , "", fontsizes.small, colors.lightgrey
									 , false
									 , "Lon:", fontsizes.small, colors.darkgrey)
	 end
	 -- pressure
	 local right_upper = navball_center + Vector(navball_radius * 1.4, navball_radius * 0.3)
	 if frame then
			local pressure, density = frame:GetAtmosphericState()
			if pressure and pressure > 0.001 then
				 drawWithUnit(right_upper
											, math.floor(pressure*100)/100, fontsizes.medium, colors.lightgrey
											, "atm", fontsizes.small, colors.lightgrey
											, false
											, "pr", fontsizes.small, colors.darkgrey)
			end
	 end
	 -- gravity
	 local g = player:GetGravity()
	 local grav = Vector(g.x, g.y, g.z):magnitude() / standard_gravity
	 if grav > 0.01 then
			local gravity = string.format("%0.2f", grav)
			local right_upper = navball_center + Vector(navball_radius * 1.4, navball_radius * 0.5)
			drawWithUnit(right_upper
									 , gravity, fontsizes.medium, colors.lightgrey
									 , "g", fontsizes.small, colors.lightgrey
									 , false
									 , "grav", fontsizes.small, colors.darkgrey)
	 end
end

local radial_nav_target = nil

local selected_combat = nil
local function show_contacts()
	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.Begin("Contacts", {})
	 pigui.Columns(2, "contactcolumns", false)
	 local bodies = Space.GetBodies(function (body) return body:IsShip() and player:DistanceTo(body) < 100000000 end)
	 table.sort(bodies, function(a,b) return player:DistanceTo(a) < player:DistanceTo(b) end)
	 for _,body in pairs(bodies) do
			if(pigui.Selectable(body.label, selected_combat == body, {"SpanAllColumns"})) then
				 player:SetCombatTarget(body)
				 selected_combat = body
			end
			pigui.NextColumn()
			local distance = player:DistanceTo(body)
			pigui.Text(Format.Distance(distance))
			pigui.NextColumn()
	 end
	 pigui.End()
end

local radial_actions = {
	 dock = function(body) player:AIDockWith(body); player:SetNavTarget(body) end,
	 fly_to = function(body) player:AIFlyTo(body); player:SetNavTarget(body)  end,
	 low_orbit = function(body) player:AIEnterLowOrbit(body); player:SetNavTarget(body)  end,
	 medium_orbit = function(body) player:AIEnterMediumOrbit(body); player:SetNavTarget(body)  end,
	 high_orbit = function(body) player:AIEnterHighOrbit(body); player:SetNavTarget(body)  end,
	 clearance = function(body) player:RequestDockingClearance(body); player:SetNavTarget(body)  end,
	 radial_in = function(body) print("implement radial_in") end,
	 radial_out = function(body) print("implement radial_out") end,
	 normal = function(body) print("implement normal") end,
	 anti_normal = function(body) print("implement anti_normal") end,
	 prograde = function(body) print("implement prograde") end,
	 retrograde = function(body) print("implement retrograde") end
}

local function show_radial_menu()
	 local radial_menu_center = pigui.GetMouseClickedPos(1)
	 local radial_menu_size = 100
	 if radial_menu_center.x < radial_menu_size then
			radial_menu_center.x = radial_menu_size
	 end
	 if radial_menu_center.y < radial_menu_size then
			radial_menu_center.y = radial_menu_size
	 end
	 if radial_menu_center.x > (pigui.screen_width - radial_menu_size) then
			radial_menu_center.x = (pigui.screen_width - radial_menu_size)
	 end
	 if radial_menu_center.y > (pigui.screen_height - radial_menu_size) then
			radial_menu_center.y = (pigui.screen_height - radial_menu_size)
	 end
	 local items = {}
	 local actions = {}
	 local function addItem(item, action)
			table.insert(items, item)
			table.insert(actions, action)
	 end

	 if radial_nav_target then
			local typ = radial_nav_target.superType
			if typ == "STARPORT" then
				 addItem("Docking Clearance", "clearance")
				 addItem("Auto-Dock", "dock")
			end
			addItem("Fly to", "fly_to")
			if typ == "STAR" or typ == "ROCKY_PLANET" or typ == "GAS_GIANT" then
				 addItem("Low Orbit", "low_orbit")
				 addItem("Medium Orbit", "medium_orbit")
				 addItem("High Orbit", "high_orbit")
			end
			addItem("Hold Radial in", "radial_in")
			addItem("Hold radial out", "radial_out")
			addItem("Hold normal", "normal")
			addItem("Hold anti-normal", "anti_normal")
			addItem("Hold prograde", "prograde")
			addItem("Hold retrograde", "retrograde")
	 end
	 local n = pigui.RadialMenu(radial_menu_center, "##piepopup", items)
	 if n >= 0 then
			local action = actions[n + 1]
			radial_actions[action](radial_nav_target)
	 end
end
local function show_nav_window()
	 -- ******************** Navigation Window ********************
	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.Begin("Navigation", {})
	 pigui.Columns(2, "navcolumns", false)
	 local body_paths = system:GetBodyPaths()
	 -- create intermediate structure
	 local data = {}
	 for _,system_path in pairs(body_paths) do
			local system_body = system_path:GetSystemBody()
			if system_body then
				 local body = Space.GetBody(system_body.index)
				 if body then
						local distance = player:DistanceTo(body)
						table.insert(data, { systemBody = system_body, body = body, distance = distance, name = system_body.name })
				 end
			end
	 end
	 -- sort by distance
	 table.sort(data, function(a,b) return a.distance < b.distance end)
	 -- display
	 for key,data in pairs(data) do
			pigui.BeginGroup()
			if(pigui.Selectable(data.name, selected == data.body, {"SpanAllColumns"})) then
				 selected = data.body
				 player:SetNavTarget(data.body)
			end
			pigui.NextColumn()
			pigui.Text(Format.Distance(data.distance))
			pigui.NextColumn()
			pigui.EndGroup()
			if pigui.IsItemClicked(1) then
				 pigui.OpenPopup("##piepopup")
				 radial_nav_target = data.body
			end
	 end
	 show_radial_menu()
	 pigui.End()
end

local function draw_thrust_fwd(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step/2, x+step,y+3*step, x-step,y+3*step, x-step,y+step/2, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x-step,y), Vector(x+step,y+3*step*ratio), false)
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step/2, x+step,y+3*step, x-step,y+3*step, x-step,y+step/2, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function draw_thrust_bwd(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x+step,y-step/2, x+step,y-2*step, x-step,y-2*step, x-step,y-step/2, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x-step,y-2*step*ratio), Vector(x+step,y), false)
	 pigui.AddConvexPolyFilled( {x,y, x+step,y-step/2, x+step,y-2*step, x-step,y-2*step, x-step,y-step/2, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function draw_thrust_down(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x+step,y-step, x+step,y-2*step, x-step,y-2*step, x-step,y-step, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x-step,y-2*step*ratio), Vector(x+step,y), false)
	 pigui.AddConvexPolyFilled( {x,y, x+step,y-step, x+step,y-2*step, x-step,y-2*step, x-step,y-step, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function draw_thrust_up(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step, x+step,y+2*step, x-step,y+2*step, x-step,y+step, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x-step,y), Vector(x+step,y+2*step*ratio), false)
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step, x+step,y+2*step, x-step,y+2*step, x-step,y+step, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function draw_thrust_left(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x-step,y+step, x-2*step,y+step, x-2*step,y-step, x-step,y-step, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x-2*step*ratio,y-step), Vector(x,y+step), false)
	 pigui.AddConvexPolyFilled( {x,y, x-step,y+step, x-2*step,y+step, x-2*step,y-step, x-step,y-step, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function draw_thrust_right(position, step, ratio)
	 local x = position.x
	 local y = position.y
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step, x+2*step,y+step, x+2*step,y-step, x+step,y-step, x,y}, colors.darkergrey, true)
	 pigui.PushClipRect(Vector(x,y-step), Vector(x+2*step*ratio,y+step), false)
	 pigui.AddConvexPolyFilled( {x,y, x+step,y+step, x+2*step,y+step, x+2*step,y-step, x+step,y-step, x,y}, colors.lightgrey, true)
	 pigui.PopClipRect()
end

local function show_thrust()
	 local thrust = player:GetThrusterState()
	 local thrust_forward = (thrust.z < 0) and math.abs(thrust.z) * math.abs(player:GetAccel("forward")) or 0
	 local thrust_backward = (thrust.z > 0) and math.abs(thrust.z) * math.abs(player:GetAccel("backward")) or 0
	 local thrust_left = (thrust.x > 0) and math.abs(thrust.x) * math.abs(player:GetAccel("left")) or 0
	 local thrust_right = (thrust.x < 0) and math.abs(thrust.x) * math.abs(player:GetAccel("right")) or 0
	 local thrust_up = (thrust.y > 0) and math.abs(thrust.y) * math.abs(player:GetAccel("up")) or 0
	 local thrust_down = (thrust.y < 0) and math.abs(thrust.y) * math.abs(player:GetAccel("down")) or 0
	 local total_thrust = Vector(thrust_forward - thrust_backward,thrust_up - thrust_down, thrust_left - thrust_right):magnitude()
	 local total_g = string.format("%0.1f", total_thrust / standard_gravity)
	 local position = Vector(200, pigui.screen_height - 100)
	 local xsize = drawWithUnit(position
															, total_g, fontsizes.large, colors.lightgrey
															, "G", fontsizes.medium, colors.lightgrey)
	 local low_thrust_power = player:GetLowThrustPower()
	 position = position + Vector(xsize.x / 2, -50)
	 local thickness = 12.0
	 pigui.AddCircle(position, 17, colors.darkergrey, 128, thickness)
	 pigui.PathArcTo(position, 17, 0, two_pi * low_thrust_power, 64)
	 pigui.PathStroke(colors.lightgrey, false, thickness)
	 local mp = pigui.GetMousePos()
	 if (Vector(mp.x,mp.y) - position):magnitude() < 17 and pigui.IsMouseReleased(1) then
			pigui.OpenPopup("thrustsettings")
	 end
	 if pigui.BeginPopup("thrustsettings") then
			local settings = { 1, 5, 10, 25, 50, 75 }
			for _,setting in pairs(settings) do
				 if pigui.Selectable("Set low thrust to " .. setting .. "%", false, {}) then
						player:SetLowThrustPower(setting / 100)
				 end
			end
			pigui.EndPopup()
	 end
	 local size = pigui.CalcTextSize(math.floor(low_thrust_power * 100))
	 pigui.AddText(position - Vector(size.x/2, size.y/2), colors.lightgrey, math.floor(low_thrust_power * 100))
	 local time_position = Vector(30, pigui.screen_height - 70)
	 local year, month, day, hour, minute, second = Game.GetDateTime()
	 pigui.PushFont("pionillium", 30)
	 local months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
	 local ymd = string.format("%04d %s. %02d ", year, months[month], day)
	 local hms = string.format("%02d:%02d:%02d", hour, minute, second)
	 local size = pigui.CalcTextSize(ymd)
	 pigui.AddText(time_position, colors.darkgrey, ymd)
	 pigui.AddText(time_position + Vector(size.x, 0), colors.lightgrey, hms)
	 pigui.PopFont()
	 local position = Vector(100, pigui.screen_height - 150)
	 local step = 12
	 local distance = 2
	 draw_thrust_fwd(position + Vector(step * 5, distance), step, thrust.z < 0 and math.abs(thrust.z) or 0)
	 draw_thrust_bwd(position + Vector(step * 5,-distance), step, thrust.z > 0 and math.abs(thrust.z) or 0)
	 draw_thrust_up(position + Vector(0, distance), step, thrust.y > 0 and math.abs(thrust.y) or 0)
	 draw_thrust_down(position + Vector(0, -distance), step, thrust.y < 0 and math.abs(thrust.y) or 0)
	 draw_thrust_left(position + Vector(-distance, 0), step, thrust.x > 0 and math.abs(thrust.x) or 0)
	 draw_thrust_right(position + Vector(distance, 0), step, thrust.x < 0 and math.abs(thrust.x) or 0)
end

local function show_debug_orbit()
	 pigui.Begin("Orbit", {})
	 local o_eccentricity, o_semimajoraxis, o_inclination, o_period, o_time_at_apoapsis, o_apoapsis, o_time_at_periapsis, o_periapsis = player:GetOrbit()
	 pigui.Text("o eccentricity: " .. o_eccentricity)
	 pigui.Text("o semi-major axis: " .. Format.Distance(o_semimajoraxis))
	 pigui.Text("O inclination: " .. math.floor(o_inclination / two_pi * 360))
	 local frame = player:GetFrame():GetSystemBody()
	 local pa = Vector(o_periapsis.x, o_periapsis.y, o_periapsis.z):magnitude()
	 local aa = Vector(o_apoapsis.x, o_apoapsis.y, o_apoapsis.z):magnitude()
	 pigui.Text("O Periapsis: " .. Format.Distance(pa) .. " (" .. Format.Distance(pa - frame.radius) .. "), O Apoapsis: " .. Format.Distance(aa) .. " (" .. Format.Distance(aa - frame.radius) .. ")")
	 pigui.Text("O Period: " .. (o_period and Format.Duration(o_period) or "none"))
	 pigui.Text("Time to O periapsis: " .. Format.Duration(o_time_at_periapsis) .. ", O time to apoapsis: " .. Format.Duration(o_time_at_apoapsis))
	 pigui.Text("Has Atmosphere: " .. (frame.hasAtmosphere and "yes" or "no"))
	 pigui.Text("Atmosphere radius: " .. frame.atmosphereRadius)
	 pigui.End()
end

local function show_debug_thrust()
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
	 pigui.Text("Total thrust: " .. total_thrust / standard_gravity .. "g")
	 pigui.End()
end

local function show_debug_temp()
	 pigui.Begin("Temperatures", {})
	 pigui.Text("Hull Temperature: " .. player:GetHullTemperature())
	 pigui.Text("Gun 0 Temperature: " .. player:GetGunTemperature(0))
	 pigui.Text("Gun 1 Temperature: " .. player:GetGunTemperature(1))
	 pigui.Text("Hull percent: " .. player:GetHullPercent())
	 pigui.Text("Shields percent: " .. player:GetShieldsPercent())
	 pigui.End()
end

local function show_debug_gravity()
	 pigui.Begin("Gravity", {})
	 local g = player:GetGravity()
	 local gr = Vector(g.x, g.y, g.z)
	 pigui.Text("Gravity: " .. string.format("%0.2f", gr:magnitude() / standard_gravity) .. ", " .. g.x .. "/" .. g.y .. "/" .. g.z)
	 pigui.End()
end


local should_show_hud = true
local cam_types = { "internal", "external", "sidereal" }
local current_cam_type = 1
local function handle_global_keys()
	 if pigui.IsKeyReleased(keys.f1) then -- ShipCpanel.cpp:317
			if Game.GetView() == "world" then
				 current_cam_type = current_cam_type + 1
				 if current_cam_type > #cam_types then
						current_cam_type = 1
				 end
				 Game.SetWorldCamType(cam_types[current_cam_type])
			else
				 Game.SetView("world")
				 current_cam_type = 1
				 Game.SetWorldCamType(cam_types[current_cam_type])
				 should_show_hud = true
			end
	 end
	 if pigui.IsKeyReleased(keys.f2) then
			Game.SetView("sector")
			should_show_hud = false
	 end
	 if pigui.IsKeyReleased(keys.f3) then
			Game.SetView("info")
			should_show_hud = false
	 end
	 if pigui.IsKeyReleased(keys.f4) and player:IsDocked() then
			Game.SetView("space_station")
			should_show_hud = false
	 end
end

local function show_ships_on_screen()
	 local bodies = Space.GetBodies(function (body) return body:IsShip() and player:DistanceTo(body) < 100000000 end)
	 for _,body in pairs(bodies) do
			local pos = body:GetProjectedScreenPosition()
			local size = 8
			if pos then
				 pigui.AddCircleFilled(pos, size, colors.lightgreen, 8)
				 pigui.AddText(pos + Vector(size*1.5, -size/2), colors.lightgreen, body.label)
			end
	 end
end

local function circle_segments(size)
	 -- just guessing, feel free to change
	 if size < 5 then
			return 8
	 elseif size < 20 then
			return 16
	 elseif size < 50 then
			return 32
	 elseif size < 100 then
			return 64
	 else
			return 128
	 end
end

local function show_bodies_on_screen()
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
				 if pigui.IsMouseClicked(1) and #labels == 1 then
						pigui.OpenPopup("##piepopup")
						radial_nav_target = labels[1]
				 end
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
			show_radial_menu()

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
end

local icons = {
	 cross = function(pos, size, col, thickness)
			pigui.AddLine(pos - Vector(size,size), pos + Vector(size,size), col, thickness)
			pigui.AddLine(pos + Vector(size,-size), pos + Vector(-size,size), col, thickness)
	 end,
	 plus = function(pos, size, col, thickness)
			pigui.AddLine(pos - Vector(size,0), pos + Vector(size,0), col, thickness)
			pigui.AddLine(pos - Vector(0,size), pos + Vector(0,size), col, thickness)
	 end,
	 normal = function(pos, size, col, thickness)
			local factor = 2
			local lineLeft = pos + Vector(-1,-1) * size
			local lineRight = pos + Vector(1,-1) * size
			local triCenter = pos + Vector(0,1) * (size / factor)
			local triLeft = lineLeft + Vector(0, 1) * (size / factor)
			local triRight = lineRight + Vector(0, 1) * (size / factor)
			pigui.AddLine(lineLeft, lineRight, col, thickness)
			pigui.AddLine(triLeft, triCenter , col, thickness)
			pigui.AddLine(triRight, triCenter , col, thickness)
	 end,
	 anti_normal = function(pos, size, col, thickness)
			local factor = 2
			local lineLeft = pos + Vector(-1,1) * size
			local lineRight = pos + Vector(1,1) * size
			local triCenter = pos + Vector(0,-1) * (size / factor)
			local triLeft = lineLeft + Vector(0, -1) * (size / factor)
			local triRight = lineRight + Vector(0, -1) * (size / factor)
			pigui.AddLine(lineLeft, lineRight, col, thickness)
			pigui.AddLine(triLeft, triCenter , col, thickness)
			pigui.AddLine(triRight, triCenter , col, thickness)
	 end,
	 radial_out = function(pos, size, col, thickness)
			local factor = 6
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			local leftCenter = pos + Vector(-1, 0) * (size / factor)
			local rightCenter = pos + Vector(1, 0) * (size / factor)
			pigui.AddLine(leftTop, leftBottom, col, thickness)
			pigui.AddLine(leftBottom, leftCenter, col, thickness)
			pigui.AddLine(leftTop, leftCenter, col, thickness)
			pigui.AddLine(rightTop, rightBottom, col, thickness)
			pigui.AddLine(rightBottom, rightCenter, col, thickness)
			pigui.AddLine(rightTop, rightCenter, col, thickness)
	 end,
	 radial_in = function(pos, size, col, thickness)
			local factor = 5
			local leftTop = pos + Vector(-1 * size / factor, 1 * size)
			local rightTop = pos + Vector(1 * size / factor, 1 * size)
			local leftBottom = pos + Vector(-1 * size / factor, -1 * size)
			local rightBottom = pos + Vector(1 * size / factor, -1 * size)
			local leftCenter = pos + Vector(-1, 0) * size
			local rightCenter = pos + Vector(1, 0) * size
			pigui.AddLine(leftTop, leftBottom, col, thickness)
			pigui.AddLine(leftBottom, leftCenter, col, thickness)
			pigui.AddLine(leftTop, leftCenter, col, thickness)
			pigui.AddLine(rightTop, rightBottom, col, thickness)
			pigui.AddLine(rightBottom, rightCenter, col, thickness)
			pigui.AddLine(rightTop, rightCenter, col, thickness)
	 end,
	 disk = function(pos, size, col, thickness)
			local segments = circle_segments(size)
			pigui.AddCircleFilled(pos, size, col, segments)
	 end,
	 circle = function(pos, size, col, thickness)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
	 end,
	 diamond = function(pos, size, col, thickness)
			local left = pos + Vector(-1,0) * size
			local right = pos + Vector(1,0) * size
			local top = pos + Vector(0,1) * size
			local bottom = pos + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, col, thickness)
	 end,
	 square = function(pos, size, col, thickness)
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			pigui.AddQuad(leftTop, leftBottom, rightBottom, rightTop, col, thickness)
	 end,
	 bullseye = function(pos, size, col, thickness)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
			pigui.AddCircleFilled(pos, size / 2, col, segments)
	 end,
	 emptyBullseye = function(pos, size, col, thickness)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
			pigui.AddCircle(pos, size / 2, col, segments, thickness)
	 end
}

local function show_marker(name, painter, color, show_in_reticule)
	 local pos,dir,point,side = markerPos(name, reticule_radius - 10)
	 if pos and show_in_reticule then
			local size = 4
			painter(pos, size, color, 1.0)
	 end
	 if side == "onscreen" and point then
			local size = 12
			painter(point, size, color, 3.0)
	 end
end

local function show_hud()
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
	 local show_forward_direction_in_reticule = true
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			icons.plus(pos, size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("backward")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			icons.cross(pos, size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("left")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("right")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(-size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("up")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("down")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end
	 -- ******************** Reticule ********************

	 if show_forward_direction_in_reticule then
			-- center of screen marker, small circle
			icons.disk(center, 2, colors.lightgrey)
			-- pointer to forward, small triangle
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:left()):normalized() * size / 1.7, colors.lightgrey, 1.5)
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:right()):normalized() * size / 1.7, colors.lightgrey, 1.5)
	 end
	 -- navigation circle
	 icons.circle(center, reticule_radius, colors.lightgrey, 2.0)

	 -- ******************** Nav Target speed / distance ********************
	 local navTarget = player:GetNavTarget()
	 if navTarget then
			-- target name
			pigui.PushFont("pionillium", 12)
			local leftTop = Vector(center.x + reticule_radius / 2 * 1.3, center.y - reticule_radius)
			pigui.AddText(leftTop, colors.darkgreen, "Nav Target")
			pigui.PopFont()
			leftTop = leftTop + Vector(20,20)
			pigui.PushFont("pionillium", 18)
			pigui.AddText(leftTop, colors.lightgreen, navTarget.label)
			pigui.PopFont()

			local speed = pigui.GetVelocity("nav_prograde")
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			xdrawWithUnit(Vector(center.x + reticule_radius + 10, center.y), spd, unit .. "/s", colors.lightgreen)

			local distance = player:DistanceTo(navTarget)
			local dist,unit = MyFormat.Distance(distance)
			xdrawWithUnit(Vector(center.x + reticule_radius / 2 * 1.7, center.y + reticule_radius / 2 * 1.7), dist, unit, colors.lightgreen)
			local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
			pigui.PushFont("pionillium", 18)
			pigui.AddText(Vector(center.x + reticule_radius / 2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgreen, "~" .. Format.Distance(brakeDist))
			pigui.PopFont()
	 end

	 -- ******************** Maneuver speed ********************
	 local spd = player:GetManeuverSpeed()
	 if spd then
			pigui.PushFont("pionillium", 30)
			local leftTop = Vector(center.x - 30, center.y - reticule_radius * 1.1)
			local speed,unit = MyFormat.Distance(spd)
			xdrawWithUnit(leftTop, speed, unit .. "/s", colors.maneuver)
			pigui.PopFont()
	 end
	 -- ******************** Combat Target speed / distance ********************
	 local combatTarget = player:GetCombatTarget()
	 if combatTarget then
			-- target name

			pigui.PushFont("pionillium", 18)
			local leftTop = Vector(center.x, center.y + reticule_radius * 1.2)
			local xsize = pigui.CalcTextSize(combatTarget.label)
			pigui.AddText(leftTop - Vector(xsize.x/2, 0), colors.lightred, combatTarget.label)
			pigui.PopFont()

			local speed = combatTarget:GetVelocityRelTo(player)
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			xdrawWithUnit(Vector(center.x + 5, center.y + reticule_radius * 1.6), spd, unit .. "/s", colors.lightred)

			local distance = player:DistanceTo(combatTarget)
			local dist,unit = MyFormat.Distance(distance)
			xdrawWithUnit(Vector(center.x - 5, center.y + reticule_radius * 1.6), dist, unit, colors.lightred, true)
			--			local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
			--			pigui.PushFont("pionillium", 18)
			--			pigui.AddText(Vector(center.x + reticule_radius / 2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgreen, "~" .. Format.Distance(brakeDist))
			--			pigui.PopFont()
	 end

	 -- ******************** Frame speed / distance ********************
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
			xdrawWithUnit(Vector(center.x - reticule_radius / 2 * 1.7, center.y + reticule_radius / 2 * 1.3), dist, unit, colors.lightgrey, true)

			local brakeDist = player:GetDistanceToZeroV("frame", "retrograde")
			pigui.PushFont("pionillium", 18)
			pigui.AddText(Vector(center.x - reticule_radius /2 * 1.7 - 50, center.y + reticule_radius / 2 * 1.3 + 20), colors.darkgrey, "~" .. Format.Distance(brakeDist))
			pigui.PopFont()

			local speed = pigui.GetVelocity("frame_prograde")
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			xdrawWithUnit(Vector(center.x - reticule_radius - 10, center.y), spd, unit .. "/s", colors.lightgrey, true)

			-- ******************** Frame markers ********************
			show_marker("frame_prograde", icons.diamond, colors.orbital_marker, true)
			show_marker("frame_retrograde", icons.cross, colors.orbital_marker, show_retrograde_indicators)
			show_marker("normal", icons.normal, colors.orbital_marker, false)
			show_marker("anti_normal", icons.anti_normal, colors.orbital_marker, false)
			show_marker("radial_out", icons.radial_out, colors.orbital_marker, false)
			show_marker("radial_in", icons.radial_in, colors.orbital_marker, false)
			show_marker("away_from_frame", icons.circle, colors.orbital_marker, true)
			local pos,dir = markerPos("frame", reticule_radius + 5)
			if pos then
				 local size = 6
				 local left = dir:left() * 4 + pos
				 local right = dir:right() * 4 + pos
				 local top = dir * 8 + pos
				 pigui.AddTriangleFilled(left, right, top, colors.darkgrey)
			end
			-- ******************** Combat target ********************
			show_marker("combat_target", icons.circle, colors.combat_target, true)
			show_marker("combat_target_lead", icons.emptyBullseye, colors.combat_target, false)
	 end
	 -- ******************** NavTarget markers ********************
	 show_marker("nav_prograde", icons.diamond, colors.lightgreen, true)
	 show_marker("nav_retrograde", icons.cross, colors.lightgreen, show_retrograde_indicators)
	 show_marker("nav", icons.square, colors.lightgreen, false)
	 local pos,dir,point,side = markerPos("nav", reticule_radius + 5)
	 if pos then
			local size = 9
			local left = dir:left() * size + pos
			local right = dir:right() * size + pos
			local top = dir * size * 2 + pos
			pigui.AddTriangleFilled(left, right, top, colors.lightgrey)
	 end
	 -- ******************** Maneuver Node ********************
	 show_marker("maneuver", icons.bullseye, colors.maneuver, true)

	 show_ships_on_screen()
	 show_bodies_on_screen()
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

	 show_navball()
	 show_thrust()

	 pigui.End()
	 pigui.PopStyleColor(1);

	 pigui.PushStyleColor("WindowBg", colors.windowbg)
	 show_nav_window()
	 show_contacts()
	 -- show_settings()
	 -- show_debug_orbit()
	 --	show_debug_thrust()
	 -- show_debug_temp()
	 -- show_debug_gravity()

	 -- Missions, these should *not* be part of the regular HUD
	 --	show_missions()
	 pigui.PopStyleColor(1)
end
pigui.handlers.HUD = function(delta)
	 player = Game.player
	 system = Game.system
	 if should_show_hud then
			show_hud()
	 end
   handle_global_keys()
end
