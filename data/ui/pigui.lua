local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')

local player
local system
local pigui = Engine.pigui

local show_retrograde_indicators = true
local show_nav_distance_with_marker = false
local show_nav_distance_with_reticule = true
local show_nav_speed_with_marker = false
local show_nav_speed_with_reticule = true
local show_frame_speed_with_marker = false
local show_frame_speed_with_reticule = true


local center
local mission_selected
local navball_center
local navball_radius = 80
local reticule_radius = 80

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4

local colors = {
	 darkgreen = {r=0, g=150, b=0},
	 lightgreen = {r=0, g=255, b=0},
	 deltav_total = {r=100,g=100,b=100,a=200},
	 deltav_remaining = {r=250,g=250,b=250},
	 deltav_current = {r=150,g=150,b=150},
	 deltav_maneuver = {r=168,g=168,b=255},
	 darkgrey = {r=150,g=150,b=150},
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
			_DESCRIPTION = "Vectors in 2D"
	 }

	 meta.__index = meta

	 function meta:__add( v )
			if(type(v) == "number") then
				 return Vector(self.x + v, self.y + v)
			else
				 return Vector(self.x + v.x, self.y + v.y)
			end
	 end

	 function meta:__sub( v )
			if(type(v) == "number") then
				 return Vector(self.x - v, self.y - v)
			else
				 return Vector(self.x - v.x, self.y - v.y)
			end
	 end

	 function meta:__mul( v )
			if(type(v) == "number") then
				 return Vector(self.x * v, self.y * v)
			else
				 return Vector(self.x * v.x, self.y * v.y)
			end
	 end

	 function meta:__div( v )
			if(type(v) == "number") then
				 return Vector(self.x / v, self.y / v)
			else
				 return Vector(self.x / v.x, self.y / v.y)
			end
	 end

	 function meta:__tostring()
			return ("<%g, %g>"):format(self.x, self.y)
	 end

	 function meta:magnitude()
			return math.sqrt( self.x * self.x + self.y * self.y )
	 end

	 function meta:normalized()
			local len = math.abs(self:magnitude())
			return Vector(self.x / len, self.y / len)
	 end

	 function meta:left()
			return Vector(-self.y, self.x)
	 end

	 function meta:right()
			return Vector(self.y, -self.x)
	 end

	 setmetatable( Vector, {
										__call = function( V, x ,y ) return setmetatable( {x = x, y = y}, meta ) end
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
	 show_nav_distance_with_marker = pigui.Checkbox("Show nav distance with nav target marker", show_nav_distance_with_marker);
	 show_nav_distance_with_reticule = pigui.Checkbox("Show nav distance with reticule", show_nav_distance_with_reticule);
	 show_nav_speed_with_marker = pigui.Checkbox("Show nav speed with nav target marker", show_nav_speed_with_marker);
	 show_nav_speed_with_reticule = pigui.Checkbox("Show nav speed with reticule", show_nav_speed_with_reticule);
	 show_frame_speed_with_marker = pigui.Checkbox("Show frame speed with frame prograde marker", show_frame_speed_with_marker);
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
	 local dir_fwd = Vector(dir.x, dir.y)
	 if side == "onscreen" then
			pigui.AddLine(pos - Vector(size,0), pos + Vector(size,0), colors.lightgrey, 3.0)
			pigui.AddLine(pos - Vector(0,size), pos + Vector(0,size), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("backward")
	 if side == "onscreen" then
			pigui.AddLine(pos - Vector(size,size), pos + Vector(size,size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(size,-size), pos + Vector(-size,size), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("left")
	 if side == "onscreen" then
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("right")
	 if side == "onscreen" then
			pigui.AddLine(pos + Vector(0,size), pos + Vector(0,-size), colors.lightgrey, 3.0)
			pigui.AddLine(pos + Vector(0, 0), pos + Vector(-size,0), colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("up")
	 if side == "onscreen" then
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("down")
	 if side == "onscreen" then
			pigui.AddLine(pos + Vector(0,0), pos + dir_fwd * size, colors.lightgrey, 3.0)
			pigui.AddLine(pos + dir_fwd:left() * size, pos + dir_fwd:right() * size, colors.lightgrey, 3.0)
	 end

	 -- ******************** Reticule ********************
	 pigui.AddCircleFilled(center, 2, colors.lightgrey, 8)
	 pigui.AddCircle(center, reticule_radius, colors.lightgrey, 128, 2.0)
	 pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:left()):normalized() * size / 1.7, colors.lightgrey, 1.5)
	 pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:right()):normalized() * size / 1.7, colors.lightgrey, 1.5)
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

			if show_nav_speed_with_reticule then
				 local speed = pigui.GetVelocity("nav_prograde")
				 local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
				 drawWithUnit(Vector(center.x + reticule_radius + 10, center.y), spd, unit .. "/s", colors.lightgreen)
			end

			if show_nav_distance_with_reticule then
				 local distance = player:DistanceTo(navTarget)
				 local dist,unit = MyFormat.Distance(distance)
				 drawWithUnit(Vector(center.x + reticule_radius / 2 * 1.7, center.y + reticule_radius / 2 * 1.7), dist, unit, colors.lightgreen)
				 local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
				 pigui.PushFont("pionillium", 18)
				 pigui.AddText(Vector(center.x + reticule_radius / 2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgreen, "~" .. Format.Distance(brakeDist))
				 pigui.PopFont()

			end

	 end

	 local frame = player:GetFrame()
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

	 if show_frame_speed_with_reticule then
			local speed = pigui.GetVelocity("frame_prograde")
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			drawWithUnit(Vector(center.x - reticule_radius - 10, center.y), spd, unit .. "/s", colors.lightgrey, true)
	 end

	 -- ******************** Frame Prograde marker ********************
	 local pos,dir,point,side = markerPos("frame_prograde", reticule_radius - 10)
	 local color = colors.lightgrey
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
			if show_frame_speed_with_marker then
				 local speed = pigui.GetVelocity("frame_prograde")
				 local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
				 drawWithUnit(Vector(point.x + size + 5, point.y), spd, unit .. "/s", colors.lightgrey)
				 local brakeDist = player:GetDistanceToZeroV("frame", "retrograde")
				 pigui.PushFont("pionillium", 18)
				 pigui.AddText(Vector(point.x + size + 5, point.y + 15), colors.darkgrey, "~" .. Format.Distance(brakeDist))
				 pigui.PopFont()
			end
	 end
	 -- ******************** Frame Retrograde marker ********************
	 local pos,dir,point,side = markerPos("frame_retrograde", reticule_radius - 10)
	 local color = colors.lightgrey
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
			if show_frame_speed_with_marker then
				 local speed = pigui.GetVelocity("frame_prograde")
				 local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
				 drawWithUnit(Vector(point.x + size + 5, point.y), spd, unit .. "/s", colors.lightgrey)
				 local brakeDist = player:GetDistanceToZeroV("frame", "prograde")
				 pigui.PushFont("pionillium", 18)
				 pigui.AddText(Vector(point.x + size + 5, point.y + 15), colors.darkgrey, "~" .. Format.Distance(brakeDist))
				 pigui.PopFont()
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
			if show_nav_speed_with_marker then
				 local speed = pigui.GetVelocity("nav_prograde")
				 local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
				 drawWithUnit(Vector(point.x + size + 5, point.y), spd, unit .. "/s", colors.lightgreen)
				 local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
				 pigui.PushFont("pionillium", 18)
				 pigui.AddText(Vector(point.x + size + 5, point.y + 15), colors.darkgrey, "~" .. Format.Distance(brakeDist))
				 pigui.PopFont()
			end
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
			if show_nav_speed_with_marker then
				 local speed = pigui.GetVelocity("nav_prograde")
				 local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
				 drawWithUnit(Vector(point.x + size + 5, point.y), spd, unit .. "/s", colors.lightgreen)
				 local brakeDist = player:GetDistanceToZeroV("nav", "prograde")
				 pigui.PushFont("pionillium", 18)
				 pigui.AddText(Vector(point.x + size + 5, point.y + 15), colors.darkgrey, "~" .. Format.Distance(brakeDist))
				 pigui.PopFont()
			end
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
			if show_nav_distance_with_marker then
				 local distance = player:DistanceTo(player:GetNavTarget())
				 local dist,unit = MyFormat.Distance(distance)
				 drawWithUnit(Vector(point.x + size + 5, point.y), dist, unit, colors.lightgreen)
				 local mp = pigui.GetMousePos()
				 if (Vector(mp.x,mp.y) - point):magnitude() < 15 then
						pigui.SetTooltip("Nav target\nThis shows the current navigational target")
				 end
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
	 for key,data in pairs(system:GetBodyPaths()) do
			local system_body = data:GetSystemBody()
			local body = Space.GetBody(system_body.index)
			local pos = body:GetProjectedScreenPosition()
			if pos then
				 local supertype = body.superType
				 if pos.x > 0 and pos.y > 0 then
						pigui.AddCircleFilled(Vector(pos.x, pos.y), 2, colors.lightgrey, 8)
						local mp = pigui.GetMousePos()
						if (Vector(mp.x,mp.y) - Vector(pos.x, pos.y)):magnitude() < 15 then
							 pigui.SetTooltip(body.label)
						end
				 end
			end
	 end
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


	 -- ******************** Navigation Window ********************
	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.PushStyleColor("WindowBg", windowbg)
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

	 show_settings()
	 -- Missions, these should *not* be part of the regular HUD
	 show_missions()
end
