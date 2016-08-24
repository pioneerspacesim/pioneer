-- TODO:
-- pause
-- time accel
-- map sub buttons
-- rotation dampening button
-- alerts
-- comms log
-- multi-function-display / scanner?
-- speed lines
-- wheels up/down button
-- hyperspace button
-- blastoff / undock
-- set speed / autopilot / manual
-- heading/pitch indicator
-- target hull/shield, general info
-- combat target / lead indicators + line
-- hyperspace
-- lua console?

local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')

local player
local system
local pigui = Engine.pigui

local show_retrograde_indicators = true

local center
local mission_selected
local navball_center
local navball_radius = 80
local navball_text_radius = navball_radius * 1.4
local reticule_radius = 80
local reticule_text_radius = reticule_radius * 1.2

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
	 green = { r=0, g=255, b=0 },
	 combat_target = { r=200, g=100, b=100 },
	 maneuver = { r=163, g=163, b=255 },
	 orbit_gauge_ground = { r=95, g=95, b=0 },
	 orbit_gauge_atmosphere = { r=97, g=97, b=241 },
	 orbit_gauge_space = { r=84, g=84, b=84 },
	 noz_darkblue = { r=6, g=7, b=38 , a=180 },
	 noz_mediumblue = { r=3, g=63, b=113 },
	 noz_lightblue = { r=49, g=102, b=144 },
	 shield_gauge = { r=0, g=255, b=255 },
	 hull_gauge = { r=200, g=200, b=200 },
	 tmp_gauge = { r=255, g=0, b=0 },
	 gun_tmp_gauge = { r=200, g=200, b=200 },
	 gauge_darkergrey = {r=20,g=20,b=20},
	 gauge_darkgrey = {r=35,g=35,b=35},
}

local pionicons = {
	 small = { name = "pionicons", size = 12, offset = nil },
	 large = { name = "pionicons", size = 30, offset = nil }
}
local pionillium = {
	 large = { name = "pionillium", size = 30, offset = 24 },
	 medium = { name = "pionillium", size = 18, offset = 14 },
	 medsmall = { name = "pionillium", size = 15, offset = 12 },
	 small = { name = "pionillium", size = 12, offset = 10 }
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
										__call = function( V, x ,y ,z )
											 local result
											 if type(x) == "table" then
													result = { x = x.x or 0, y = x.y or 0, z = x.z or 0 }
											 else
													result = {x = x or 0, y = y or 0, z = z or 0}
											 end
											 return setmetatable( result, meta ) end
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

-- ******************** Utils ********************
local function clamp(value, min, max)
	 if value < min then
			return min
	 elseif value > max then
			return max
	 else
			return value
	 end
end

local function count(tab)
	 local i = 0
	 for _,v in pairs(tab) do
			i = i + 1
	 end
	 return i
end

local function point_on_circle_radius(center, radius, hours)
	 -- 0 hours is top, going rightwards, negative goes leftwards
	 local a = math.fmod(hours / 12 * two_pi, two_pi)
	 local p = Vector(0, -radius)
	 return Vector(center.x, center.y) + Vector(p.x * math.cos(a) - p.y * math.sin(a), p.y * math.cos(a) + p.x * math.sin(a))
end

local anchor = { left = 1, right = 2, center = 3, top = 4, bottom = 5, baseline = 6 }

local function calc_alignment(pos, size, anchor_horizontal, anchor_vertical)
	 local position = Vector(pos.x, pos.y)
	 if anchor_horizontal == anchor.left or anchor_horizontal == nil then
			position.x = position.x -- do nothing
	 elseif anchor_horizontal == anchor.right then
			position.x = position.x - size.x
	 elseif anchor_horizontal == anchor.center then
			position.x = position.x - size.x/2
	 else
			error("show_text: incorrect horizontal anchor " .. anchor_horizontal)
	 end
	 if anchor_vertical == anchor.top or anchor_vertical == nil then
			position.y = position.y -- do nothing
	 elseif anchor_vertical == anchor.center then
			position.y = position.y - size.y/2
	 elseif anchor_vertical == anchor.bottom then
			position.y = position.y - size.y
	 else
			error("show_text: incorrect vertical anchor " .. anchor_vertical)
	 end
	 return position
end

local function show_text(pos, text, color, anchor_horizontal, anchor_vertical, font)
	 local position = Vector(pos.x, pos.y)
	 -- AddText aligns to upper left
	 pigui.PushFont(font.name, font.size)
	 local size = pigui.CalcTextSize(text)
	 local foo
	 if anchor_vertical == anchor.baseline then
			foo = nil
	 else
			foo = anchor_vertical
	 end
	 position = calc_alignment(position, size, anchor_horizontal, foo) -- ignore vertical if baseline
	 if anchor_vertical == anchor.baseline then
			position.y = position.y - font.offset
	 end
	 pigui.AddText(position, color, text)
	 -- pigui.AddQuad(position, position + Vector(size.x, 0), position + Vector(size.x, size.y), position + Vector(0, size.y), colors.red, 1.0)
	 pigui.PopFont()
	 return Vector(size.x, size.y)
end

local function show_text_fancy(position, texts, colors, fonts, anchor_horizontal, anchor_vertical)
	 -- always align texts at baseline
	 local spacing = 2
	 local size = Vector(0, 0)
	 local max_offset = 0
	 assert(#texts == #colors and #texts == #fonts)
	 for i=1,#texts do
			pigui.PushFont(fonts[i].name, fonts[i].size)
			local s = pigui.CalcTextSize(texts[i])
			size.x = size.x + s.x
			size.x = size.x + spacing -- spacing
			size.y = math.max(size.y, s.y)
			max_offset = math.max(max_offset, fonts[i].offset)
			pigui.PopFont()
	 end
	 size.x = size.x - spacing -- remove last spacing
	 position = calc_alignment(position, size, anchor_horizontal, nil)
	 if anchor_vertical == anchor.top then
			position.y = position.y + max_offset
	 elseif anchor_vertical == anchor.bottom then
			position.y = position.y - (size.y - max_offset)
	 end
	 for i=1,#texts do
			pigui.PushFont(fonts[i].name, fonts[i].size)
			local s = show_text(position, texts[i], colors[i], anchor.left, anchor.baseline, fonts[i])
			position.x = position.x + s.x + spacing
			pigui.PopFont()
	 end
	 return size
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

local symbol = {
	 cross = function(pos, size, col, thickness, direction)
			pigui.AddLine(pos - Vector(size,size), pos + Vector(size,size), col, thickness)
			pigui.AddLine(pos + Vector(size,-size), pos + Vector(-size,size), col, thickness)
	 end,
	 plus = function(pos, size, col, thickness, direction)
			pigui.AddLine(pos - Vector(size,0), pos + Vector(size,0), col, thickness)
			pigui.AddLine(pos - Vector(0,size), pos + Vector(0,size), col, thickness)
	 end,
	 chevron = function(pos, size, col, thickness, direction)
			local dir = direction or Vector(0, -1)
			local factor = 2
			local triCenter = pos + dir * size
			local triLeft = pos + dir:left() * size
			local triRight = pos + dir:right() * size
			pigui.AddLine(triLeft, triCenter , col, thickness)
			pigui.AddLine(triRight, triCenter , col, thickness)
	 end,
	 normal = function(pos, size, col, thickness, direction)
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
	 anti_normal = function(pos, size, col, thickness, direction)
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
	 radial_out = function(pos, size, col, thickness, direction)
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
	 radial_in = function(pos, size, col, thickness, direction)
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
	 disk = function(pos, size, col, thickness, direction)
			local segments = circle_segments(size)
			pigui.AddCircleFilled(pos, size, col, segments)
	 end,
	 circle = function(pos, size, col, thickness, direction)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
	 end,
	 diamond = function(pos, size, col, thickness, direction)
			local left = pos + Vector(-1,0) * size
			local right = pos + Vector(1,0) * size
			local top = pos + Vector(0,1) * size
			local bottom = pos + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, col, thickness)
	 end,
	 square = function(pos, size, col, thickness, direction)
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			pigui.AddQuad(leftTop, leftBottom, rightBottom, rightTop, col, thickness)
	 end,
	 bullseye = function(pos, size, col, thickness, direction)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
			pigui.AddCircleFilled(pos, size / 2, col, segments)
	 end,
	 empty_bullseye = function(pos, size, col, thickness, direction)
			local segments = circle_segments(size)
			pigui.AddCircle(pos, size, col, segments, thickness)
			pigui.AddCircle(pos, size / 2, col, segments, thickness)
	 end,
	 bottom = function(pos, size, col, thickness, direction)
			pigui.AddLine(pos + Vector(0,0), pos + direction * size, col, thickness)
			pigui.AddLine(pos + direction:left() * size, pos + direction:right() * size, col, thickness)
	 end
}

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

local function show_settings()
	 pigui.Begin("Settings", {})
	 show_retrograde_indicators = pigui.Checkbox("Show retrograde indicators", show_retrograde_indicators);
	 for k,v in pairs(colors) do
			
			local changed, r, g, b, a = pigui.DragInt4(k, v.r or 0, v.g or 0, v.b or 0, v.a or 255, 1.0, 0, 255)
			if changed then
				 v.r = r
				 v.g = g
				 v.b = b
				 v.a = a
			end
	 end
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

-- ratio is 1.0 for full, 0.0 for empty
local function orbit_gauge(center, radius, color, thickness, start_ratio, end_ratio)
	 pigui.PathArcTo(center, radius + thickness / 2, -pi_4 + pi_2 * (1 - end_ratio), -pi_4 + pi_2 * (1 - start_ratio), 64)
	 pigui.PathStroke(color, false, thickness)
end

local function orbit_gauge_position(center, radius, ratio)
	 return point_on_circle_radius(center, radius, 4.5 - 3 * ratio)
end

local function show_circular_gauge(center, ratio, color, main_text, small_text)
	 local radius = 23
	 local thickness = 10
	 local segments = circle_segments(radius)
	 local center_text_font = pionillium.medium
	 local main_text_font = pionillium.medsmall
	 local small_text_font = pionillium.small
	 local arc_start = pi_2
	 local arc_end = two_pi
	 local range = arc_end - arc_start
	 local first_quarter = arc_start + range * 0.25
	 local second_quarter = arc_start + range * 0.50
	 local third_quarter = arc_start + range * 0.75
	 local ratio_end = arc_start + range * clamp(ratio, 0, 1)

	 -- quarters
	 pigui.PathArcTo(center, radius, arc_start, first_quarter, segments)
	 pigui.PathStroke(colors.gauge_darkergrey, false, thickness * 0.99)
	 pigui.PathArcTo(center, radius, first_quarter, second_quarter, segments)
	 pigui.PathStroke(colors.gauge_darkgrey, false, thickness * 0.99)
	 pigui.PathArcTo(center, radius, second_quarter, third_quarter, segments)
	 pigui.PathStroke(colors.gauge_darkergrey, false, thickness * 0.99)
	 pigui.PathArcTo(center, radius, third_quarter, arc_end, segments)
	 pigui.PathStroke(colors.gauge_darkgrey, false, thickness * 0.99)

	 pigui.PathArcTo(center, radius, arc_start, ratio_end, segments)
	 pigui.PathStroke(color, false, thickness)
	 show_text(center, math.ceil(ratio * 100), colors.lightgrey, anchor.center, anchor.center, center_text_font)
	 show_text(center + Vector(4, radius), main_text, colors.lightgrey, anchor.left, anchor.center, main_text_font)
	 if small_text then
			show_text(center + Vector(radius, radius):normalized()*radius, small_text, colors.lightgrey, anchor.left, anchor.bottom, small_text_font)
	 end
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

	 -- delta-v remaining
	 local spd,unit = MyFormat.Distance(deltav_remaining)
	 local position = point_on_circle_radius(navball_center, navball_text_radius, -3)
	 show_text_fancy(position, { "Δv", spd, unit .. "/s" }, { colors.darkgrey, colors.lightgrey, colors.darkgrey }, { pionillium.medium, pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)
	 local time_until_empty = deltav_remaining / player:GetAccel("forward")
	 show_text_fancy(position, { math.floor(dvr*100) .. "%     " .. Format.Duration(time_until_empty) }, { colors.lightgrey }, { pionillium.small }, anchor.right, anchor.top)
	 if deltav_maneuver > 0 then
	 		local spd,unit = MyFormat.Distance(deltav_maneuver)
			position = point_on_circle_radius(navball_center, navball_text_radius, -4)
			show_text_fancy(position, { "mΔv", spd, unit .. "/s" }, { colors.maneuver, colors.maneuver, colors.maneuver }, { pionillium.medium, pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)
			local maneuver_time = deltav_maneuver / player:GetAccel("forward")
			show_text_fancy(position, { math.floor(dvm/dvr*100) .. "%     " .. Format.Duration(maneuver_time) }, { colors.maneuver }, { pionillium.small }, anchor.right, anchor.top)
	 end

	 local frame = player:GetFrame()
	 local frame_radius = frame and frame:GetSystemBody().radius or 0

	 -- ******************** Orbital stats ********************
	 local o_eccentricity, o_semimajoraxis, o_inclination, o_period, o_time_at_apoapsis, o_apoapsis, o_time_at_periapsis, o_periapsis = player:GetOrbit()
	 local aa = Vector(o_apoapsis.x, o_apoapsis.y, o_apoapsis.z):magnitude()
	 local pa = Vector(o_periapsis.x, o_periapsis.y, o_periapsis.z):magnitude()
	 -- apoapsis
	 if not player:IsDocked() then
			local position = point_on_circle_radius(navball_center, navball_text_radius, 2)
			local aa_d = aa - frame_radius
			local dist_apo, unit_apo = MyFormat.Distance(aa_d)
			if aa_d > 0 then
				 local textsize = show_text_fancy(position, { "a", dist_apo, unit_apo }, { colors.darkgrey, colors.lightgrey, colors.darkgrey }, {pionillium.small, pionillium.medium, pionillium.small }, anchor.left, anchor.baseline)
				 show_text(position + Vector(textsize.x * 1.2), "t-" .. Format.Duration(o_time_at_apoapsis), (o_time_at_apoapsis < 30 and colors.lightgreen or colors.lightgrey), anchor.left, anchor.baseline, pionillium.small)
			end
	 end
	 -- altitude
	 local alt, vspd, lat, lon = player:GetGPS()
	 if alt then
			local altitude,unit = MyFormat.Distance(alt)
			local position = point_on_circle_radius(navball_center, navball_text_radius, 2.6)
			local textsize = show_text_fancy(position, { "alt", altitude, unit }, {colors.darkgrey, colors.lightgrey, colors.darkgrey }, {pionillium.medium, pionillium.large, pionillium.medium }, anchor.left, anchor.baseline)
			local vspeed, unit = MyFormat.Distance(vspd)
			show_text_fancy(position + Vector(textsize.x * 1.1), { (vspd > 0 and "+" or "") .. vspeed, unit .. "/s" }, { (vspd < 0 and colors.lightred or colors.lightgreen), colors.darkgrey }, {pionillium.medium, pionillium.small }, anchor.left, anchor.baseline)
	 end
	 -- periapsis
	 if not player:IsDocked() then
			local position = point_on_circle_radius(navball_center, navball_text_radius, 3)
			local pa_d = pa - frame_radius
			local dist_per, unit_per = MyFormat.Distance(pa_d)
			if pa and pa_d ~= 0 then
				 local textsize = show_text_fancy(position,
																					{ "p", dist_per, unit_per, "     t-" .. Format.Duration(o_time_at_periapsis) },
																					{ colors.darkgrey, (pa - frame_radius < 0 and colors.lightred or colors.lightgrey), colors.darkgrey, (o_time_at_periapsis < 30 and colors.lightgreen or colors.lightgrey) },
																					{pionillium.small, pionillium.medium, pionillium.small, pionillium.small },
																					anchor.left,
																					anchor.baseline)
			end
	 end
	 -- inclination, eccentricity
	 if not player:IsDocked() then
			local position = point_on_circle_radius(navball_center, navball_text_radius, 3.4)
			show_text_fancy(position,
											{ "inc", math.floor(o_inclination / two_pi * 360) .. "°", "    ecc", math.floor(o_eccentricity * 100) / 100},
											{ colors.darkgrey, colors.lightgrey, colors.darkgrey, colors.lightgrey },
											{ pionillium.small, pionillium.medium, pionillium.small, pionillium.medium },
											anchor.left, anchor.baseline)
	 end
	 -- pressure, gravity
	 if frame then
			local position = point_on_circle_radius(navball_center, navball_text_radius, 3.8)
			local pressure, density = frame:GetAtmosphericState()
			local g = player:GetGravity()
			local grav = Vector(g.x, g.y, g.z):magnitude() / standard_gravity
			local gravity
			if grav > 0.01 then
				 gravity = string.format("%0.2f", grav)
			end
			local txts = {}
			local cols = {}
			local fnts = {}
			if pressure and pressure > 0.001 then
				 table.insert(txts, "pr")
				 table.insert(txts, math.floor(pressure*100)/100)
				 table.insert(txts, "atm")
				 table.insert(cols, colors.darkgrey)
				 table.insert(cols, colors.lightgrey)
				 table.insert(cols, colors.darkgrey)
				 table.insert(fnts, pionillium.small)
				 table.insert(fnts, pionillium.medium)
				 table.insert(fnts, pionillium.small)
			end
			if pressure and pressure > 0.001 and gravity then
				 table.insert(txts, "    ")
				 table.insert(cols, colors.darkgrey)
				 table.insert(fnts, pionillium.small)
			end
			if gravity then
				 table.insert(txts, "grav")
				 table.insert(txts, gravity)
				 table.insert(txts, "g")
				 table.insert(cols, colors.darkgrey)
				 table.insert(cols, colors.lightgrey)
				 table.insert(cols, colors.darkgrey)
				 table.insert(fnts, pionillium.small)
				 table.insert(fnts, pionillium.medium)
				 table.insert(fnts, pionillium.small)
			end
			show_text_fancy(position, txts, cols, fnts, anchor.left, anchor,baseline)
			-- latitude, longitude
			position = point_on_circle_radius(navball_center, navball_text_radius, 4.5)
			if lat and lon then
				 local textsize = show_text_fancy(position, { "Lat:", lat }, { colors.darkgrey, colors.lightgrey }, { pionillium.small, pionillium.medium }, anchor.left, anchor.baseline)
				 show_text_fancy(position + Vector(0, textsize.y * 1.2), { "Lon:", lon }, { colors.darkgrey, colors.lightgrey }, { pionillium.small, pionillium.medium }, anchor.left, anchor.baseline)
			end
	 end
	 -- ******************** orbit display ********************
	 if frame then
			local frame_sb = frame:GetSystemBody()
			local atmosphere_height = frame_sb.hasAtmosphere and frame_radius * frame_sb.atmosphereRadius or frame_radius
			local my_position = Vector(player:GetPosition()):magnitude()
			local min_height = frame_radius
			local max_height = math.max(math.max(atmosphere_height, aa), my_position)
			local range = max_height - min_height
			local thickness = 10
			local ends = 0.05
			local my_height = clamp((my_position - min_height) / range, ends/2, 1 - ends/2)
			local apoapsis = aa > 0 and clamp((aa - min_height) / range, ends, 1 - ends/2) or nil
			local periapsis = clamp((pa - min_height) / range, ends, 1 - ends/2)
			local atmosphere_ratio = frame_sb.hasAtmosphere and math.max(ends, (atmosphere_height - min_height) / range - 2 * ends) or 0
			orbit_gauge(navball_center, navball_radius + 5 + thickness, colors.orbit_gauge_space, thickness * 0.99, 0.0, 1.0)
			orbit_gauge(navball_center, navball_radius + 5 + thickness, colors.orbit_gauge_ground, thickness, 0, ends)
			orbit_gauge(navball_center, navball_radius + 5 + thickness, colors.orbit_gauge_atmosphere, thickness, ends, ends + atmosphere_ratio)

			symbol.circle(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, my_height), thickness / 2.3, colors.lightgrey, 2)
			if apoapsis then
				 local self_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, apoapsis)
				 local close_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, apoapsis * 1.05)
				 local dir = (close_position - self_position):normalized()
				 symbol.chevron(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, apoapsis), thickness / 2.3, colors.lightgrey, 2, dir)
			end
			do
				 local self_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, periapsis)
				 local close_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, periapsis * 0.95)
				 local dir = (close_position - self_position):normalized()
				 symbol.chevron(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*1.5, periapsis), thickness / 2.3, colors.lightgrey, 2, dir)
			end
	 end
	 -- circular stats, lower left
	 local position = Vector(navball_center.x - 180,pigui.screen_height - 40)
	 show_circular_gauge(position, player:GetHullTemperature(), colors.tmp_gauge, "Temp", "Hull")
	 show_circular_gauge(position + Vector(-90, 0), player:GetHullPercent() / 100, colors.hull_gauge, "Hull", "Integrity")
	 if player:GetShieldsPercent() then
			show_circular_gauge(position + Vector(-180, 0), player:GetShieldsPercent() / 100, colors.shield_gauge, "Shield")
	 end
	 -- gun stats, left side
	 local position = Vector(45, pigui.screen_height - 290)
	 show_circular_gauge(position, player:GetGunTemperature(0), colors.gun_tmp_gauge, "Blaster", "Front")
	 show_circular_gauge(position + Vector(0, 90), player:GetGunTemperature(1), colors.gun_tmp_gauge, "Blaster", "Rear")
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
	 dock = function(body) player:AIDockWith(body); player:SetNavTarget(body) end, -- TODO check for autopilot
	 fly_to = function(body) player:AIFlyTo(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
	 low_orbit = function(body) player:AIEnterLowOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
	 medium_orbit = function(body) player:AIEnterMediumOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
	 high_orbit = function(body) player:AIEnterHighOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
	 clearance = function(body) player:RequestDockingClearance(body); player:SetNavTarget(body)  end, -- TODO check for room for docking, or make it so you can be denied
	 radial_in = function(body) print("implement radial_in") end, -- 	Pi::player->GetPlayerController()->SetFlightControlState(s); CONTROL_FIXHEADING_FORWARD
	 radial_out = function(body) print("implement radial_out") end,
	 normal = function(body) print("implement normal") end,
	 anti_normal = function(body) print("implement anti_normal") end,
	 prograde = function(body) print("implement prograde") end,
	 retrograde = function(body) print("implement retrograde") end,
	 -- hypercloud_analyzer: 	Pi::game->GetSectorView()->SetHyperspaceTarget(cloud->GetShip()->GetHyperspaceDest());
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
	 local tooltips = {}
	 local function addItem(icon, item, action)
			table.insert(items, icon)
			table.insert(actions, action)
			table.insert(tooltips, item)
	 end

	 if radial_nav_target then
			local typ = radial_nav_target.superType
			if typ == "STARPORT" then
				 addItem("6", "Docking Clearance", "clearance")
				 addItem("4", "Auto-Dock", "dock")
			end
			addItem("5", "Fly to", "fly_to")
			if typ == "STAR" or typ == "ROCKY_PLANET" or typ == "GAS_GIANT" then
				 addItem("1", "Low Orbit", "low_orbit")
				 addItem("2", "Medium Orbit", "medium_orbit")
				 addItem("3", "High Orbit", "high_orbit")
			end
			-- addItem("Hold Radial in", "radial_in")
			-- addItem("Hold radial out", "radial_out")
			-- addItem("Hold normal", "normal")
			-- addItem("Hold anti-normal", "anti_normal")
			-- addItem("Hold prograde", "prograde")
			-- addItem("Hold retrograde", "retrograde")
	 end
	 local n = pigui.RadialMenu(radial_menu_center, "##piepopup", items, pionicons.large.name, pionicons.large.size, tooltips)
	 if n >= 0 then
			local action = actions[n + 1]
			radial_actions[action](radial_nav_target)
	 end
end
local function get_body_icon_letter(body)
	 local typ = body.type
	 local superType = body.superType
	 if superType == "STAR" then
			return "S"
	 elseif superType == "GAS_GIANT" then
			return "G"
	 elseif superType == "ROCKY_PLANET" then
			sb = body:GetSystemBody()
			if sb.parent.superType == "STAR" then
				 return "P"
			else
				 return "M"
			end
	 else -- if superType == "STARPORT" then
			if typ == "STARPORT_ORBITAL" then
				 return "s"
			else -- if typ == "STARPORT_SURFACE"
				 return "p"
			end
	 end
end

local function get_hierarchical_bodies()
	 local count = 0
	 local body_paths = system:GetBodyPaths()
	 -- create intermediate structure
	 local data = {}
	 local index = {}
	 local lookup = {}
	 --	 local body_path_count = 0
	 for _,system_path in pairs(body_paths) do
			--			body_path_count = body_path_count + 1
			local system_body = system_path:GetSystemBody()
			if system_body then
				 local parent = system_body.parent
				 if parent then
						local parent_children = index[parent.index] or {}
						table.insert(parent_children, system_body)
						index[parent.index] = parent_children
				 end
				 local body = Space.GetBody(system_body.index)
				 if body then
						local distance = player:DistanceTo(body)
						local item = { systemBody = system_body, body = body, distance = distance, name = system_body.name, parent = parent, children = children }
						count = count + 1
						table.insert(data, item)
						lookup[system_body.index] = item
				 end
			end
	 end
	 --	 print("body path count: " .. body_path_count)
	 for _,body in pairs(data) do
			local ch = index[body.systemBody.index]
			local children = {}
			if ch then
				 for _,sb in pairs(ch) do
						table.insert(children, lookup[sb.index])
				 end
				 body.children = children
			end
	 end
	 local suns = {}
	 for _,body in pairs(data) do
			if body.parent == nil then
				 table.insert(suns, body)
			end
	 end
	 return suns, count
end

local function spaces(n)
	 local res = ""
	 for i=1,n+1 do
			res = res .. " "
	 end
	 return res
end
local function show_nav_window()
	 -- ******************** Navigation Window ********************
	 --	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 --	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.Begin("Navigation", {})
	 pigui.Columns(2, "navcolumns", false)
	 local data,count = get_hierarchical_bodies()
	 local nav_target = player:GetNavTarget()
	 local lines = 0
	 local function AddLine(data, indent)
			--			print("AddLine " .. spaces(indent) .. data.body.label)
			lines = lines + 1
			pigui.BeginGroup()
			pigui.PushFont("pionicons", 12)
			pigui.Text(spaces(indent) .. get_body_icon_letter(data.body))
			--			if(pigui.Selectable(spaces(indent) .. get_body_icon_letter(data.body), nav_target == data.body, {"SpanAllColumns"})) then
			--				 player:SetNavTarget(data.body)
			--			end
			pigui.PopFont()
			pigui.SameLine()
			if(pigui.Selectable(data.name, nav_target == data.body, {"SpanAllColumns"})) then
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
			for _,data in pairs(data.children or {}) do
				 AddLine(data, indent + 1)
			end
	 end
	 -- sort by distance
	 table.sort(data, function(a,b) return a.distance < b.distance end)
	 -- display
	 for key,data in pairs(data) do
			AddLine(data, 0)
	 end
	 if lines ~= count then
			error("nok: " .. count .. " count vs. lines " .. lines)
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
	 local textsize = show_text_fancy(position, { total_g, "G" }, { colors.lightgrey, colors.lightgrey }, { pionillium.large, pionillium.medium }, anchor.center, anchor.center)
	 local low_thrust_power = player:GetLowThrustPower()
	 position = position + Vector(0,-50)
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

-- return the "larger" body
local function get_max_body(body_a, body_b)
	 -- feel free to change
	 local order = { "S", "G", "P", "M", "s", "p" }
	 local icon_a = get_body_icon_letter(body_a)
	 local icon_b = get_body_icon_letter(body_b)
	 for _,letter in pairs(order) do
			if icon_a == letter then
				 return body_a
			elseif icon_b == letter then
				 return body_b
			end
	 end
	 return body_a
end

local function get_body_group_max_body(group)
	 local best_body
	 local i = 0
	 for _,body in pairs(group) do
			if not best_body then
				 best_body = body
			else
				 best_body = get_max_body(best_body, body)
			end
			i = i + 1
	 end
	 return best_body, i
end

local function show_bodies_on_screen()
	 local body_groups = {}
	 local cutoff = 15
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
			do
				 local best_body, count = get_body_group_max_body(group)
				 local textsize = show_text(pos, get_body_icon_letter(best_body), colors.lightgrey, anchor.center, anchor.center, pionicons.small)
				 show_text(pos + Vector(textsize.x * 1.1), best_body.label .. (count > 1 and " +" or ""), colors.lightgrey, anchor.left, anchor.center, pionillium.small)
			end
			local mp = pigui.GetMousePos()
			labels = {}
			for p,body in pairs(group) do
				 table.insert(labels, body)
			end
			table.sort(labels, function (a,b) return a.label < b.label end)
			local label = table.concat(map(function (a) return a.label end, labels), "\n")
			local show_tooltip = false
			if (Vector(mp.x - pos.x,mp.y - pos.y)):magnitude() < cutoff then
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


local function show_marker(name, painter, color, show_in_reticule, direction, size)
	 local siz = size and size or 12
	 local pos,dir,point,side = markerPos(name, reticule_radius - 10)
	 if pos and show_in_reticule then
			local thesize = siz / 3
			painter(pos, thesize, color, 1.0, direction)
	 end
	 if side == "onscreen" and point then
			local thesize = siz
			painter(point, thesize, color, 3.0, direction)
	 end
end

local function show_hud()
	 center = Vector(pigui.screen_width/2, pigui.screen_height/2)
	 navball_center = Vector(center.x, pigui.screen_height - 25 - navball_radius)
	 local windowbg = colors.noz_darkblue
	 -- transparent full-size window, no inputs
	 pigui.SetNextWindowPos(Vector(0, 0), "Always")
	 pigui.SetNextWindowSize(Vector(pigui.screen_width, pigui.screen_height), "Always")
	 pigui.PushStyleColor("WindowBg", colors.transparent)
	 pigui.Begin("HUD", {"NoTitleBar","NoInputs","NoMove","NoResize","NoSavedSettings","NoFocusOnAppearing","NoBringToFrontOnFocus"})

	 -- symbol.disk(Vector(100,100), 2, colors.red, 1.0)
	 -- show_text_fancy(Vector(100,100), { "bot", "100.5", "atm" }, { colors.lightgrey, colors.red, colors.green }, { pionillium.large, pionillium.small, pionillium.medium }, anchor.right, anchor.bottom)
	 -- show_text_fancy(Vector(100,100), { "top", "100.5", "atm" }, { colors.lightgrey, colors.red, colors.green }, { pionillium.large, pionillium.small, pionillium.medium }, anchor.left, anchor.top)
	 -- show_text(Vector(100,100), "foo", colors.green, anchor.right, anchor.baseline, pionillium.large)
	 -- show_text(Vector(100,100), "bar", colors.green, anchor.left, anchor.baseline, pionillium.small)
	 -- ******************** Ship Directional Markers ********************
	 local size=8
	 local side, dir, pos = pigui.GetHUDMarker("forward")
	 local dir_fwd = Vector(dir.x, dir.y)
	 local show_forward_direction_in_reticule = true
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.plus(pos, size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("backward")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.cross(pos, size, colors.lightgrey, 3.0)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("left")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.bottom(pos, size, colors.lightgrey, 3.0, dir_fwd)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("right")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.bottom(pos, size, colors.lightgrey, 3.0, dir_fwd)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("up")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.bottom(pos, size, colors.lightgrey, 3.0, dir_fwd)
	 end
	 local side, dir, pos = pigui.GetHUDMarker("down")
	 if side == "onscreen" then
			if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
				 show_forward_direction_in_reticule = false
			end
			symbol.bottom(pos, size, colors.lightgrey, 3.0, dir_fwd)
	 end
	 -- ******************** Reticule ********************
	 if show_forward_direction_in_reticule then
			-- center of screen marker, small circle
			symbol.disk(center, 2, colors.lightgrey)
			-- pointer to forward, small triangle
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:left()):normalized() * size / 1.7, colors.lightgrey, 1.5)
			pigui.AddLine(center + dir_fwd * size, center + (dir_fwd + dir_fwd:right()):normalized() * size / 1.7, colors.lightgrey, 1.5)
	 end
	 -- navigation circle
	 symbol.circle(center, reticule_radius, colors.lightgrey, 2.0)

	 -- ******************** Nav Target speed / distance ********************
	 local navTarget = player:GetNavTarget()
	 if navTarget then
			-- target name
			local position = point_on_circle_radius(center, reticule_text_radius, 1.35)
			show_text(position, "Nav Target", colors.darkgreen, anchor.left, anchor.bottom, pionillium.small)
			position = point_on_circle_radius(center, reticule_text_radius, 2)
			show_text(position, navTarget.label, colors.lightgreen, anchor.left, anchor.bottom, pionillium.medium)

			local speed = Vector(pigui.GetVelocity("nav_prograde"))
			local spd,unit = MyFormat.Distance(speed:magnitude())
			position = point_on_circle_radius(center, reticule_text_radius, 2.9)
			local textsize = show_text_fancy(position, { spd, unit .. "/s" }, { colors.lightgreen, colors.darkgreen }, { pionillium.large, pionillium.medium }, anchor.left, anchor.bottom)
			do
				 local pos = Vector(player:GetPositionRelTo(navTarget))
				 local vel = Vector(player:GetVelocityRelTo(navTarget))
				 local proj = pos:dot(vel) / pos:magnitude()
				 local spd,unit = MyFormat.Distance(proj)
				 show_text_fancy(position + Vector(textsize.x * 1.1, 0), { spd, unit .. "/s" }, { colors.lightgreen, colors.darkgreen }, { pionillium.medium, pionillium.small }, anchor.left, anchor.bottom)
			end


			local distance = player:DistanceTo(navTarget)
			local dist,unit = MyFormat.Distance(distance)
			position = point_on_circle_radius(center, reticule_text_radius, 3.1)
			local textsize = show_text_fancy(position, { dist, unit }, { colors.lightgreen, colors.darkgreen }, { pionillium.large, pionillium.medium }, anchor.left, anchor.top )
			local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
			position.y = position.y + textsize.y * 1.1
			show_text(position, "~" .. Format.Distance(brakeDist), colors.darkgreen, anchor.left, anchor.top, pionillium.medium)
	 end

	 -- ******************** Maneuver speed ********************
	 local spd = player:GetManeuverSpeed()
	 if spd then
			local position = point_on_circle_radius(center, reticule_text_radius, 0)
			local speed,unit = MyFormat.Distance(spd)
			show_text_fancy(position, { speed, unit .. "/s" }, { colors.maneuver, colors.maneuver }, { pionillium.large, pionillium.medium }, anchor.center, anchor.bottom)
	 end
	 -- ******************** Combat Target speed / distance ********************
	 local combatTarget = player:GetCombatTarget()
	 if combatTarget then
			-- target name

			local position = point_on_circle_radius(center, reticule_text_radius, 6)
			local textsize = show_text(position, combatTarget.label, colors.lightred, anchor.center, anchor.top, pionillium.medium)

			position.y = position.y + textsize.y * 1.1
			local speed = combatTarget:GetVelocityRelTo(player)
			local spd,unit = MyFormat.Distance(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
			show_text_fancy(position + Vector(-5.0), { spd, unit .. "/s" }, { colors.lightred, colors.lightred }, { pionillium.large, pionillium.medium }, anchor.right, anchor.top)
			local distance = player:DistanceTo(combatTarget)
			local dist,unit = MyFormat.Distance(distance)
			show_text_fancy(position + Vector(5,0), { dist, unit }, { colors.lightred, colors.lightred }, { pionillium.large, pionillium.medium }, anchor.left, anchor.top)
			--			local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
			--			pigui.PushFont("pionillium", 18)
			--			pigui.AddText(Vector(center.x + reticule_radius / 2 * 1.7 - 20, center.y + reticule_radius / 2 * 1.7 + 20), colors.darkgreen, "~" .. Format.Distance(brakeDist))
			--			pigui.PopFont()
	 end

	 -- ******************** Frame speed / distance ********************
	 local frame = player:GetFrame()
	 if frame then
			local position = point_on_circle_radius(center, reticule_text_radius, -1.35)
			show_text(position, "Frame", colors.darkgrey, anchor.right, anchor.bottom, pionillium.small)
			position = point_on_circle_radius(center, reticule_text_radius, -2)
			show_text(position, frame.label, colors.lightgrey, anchor.right, anchor.bottom, pionillium.medium)

			local speed = Vector(pigui.GetVelocity("frame_prograde"))
			local spd,unit = MyFormat.Distance(speed:magnitude())
			position = point_on_circle_radius(center, reticule_text_radius, -2.9)
			show_text_fancy(position, { spd, unit .. "/s" }, { colors.lightgrey, colors.darkgrey }, { pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)

			local distance = player:DistanceTo(frame)
			local dist,unit = MyFormat.Distance(distance)
			position = point_on_circle_radius(center, reticule_text_radius, -3.1)
			local textsize = show_text_fancy(position, { dist, unit }, { colors.lightgrey, colors.darkgrey }, { pionillium.large, pionillium.medium }, anchor.right, anchor.top )

			local brakeDist = player:GetDistanceToZeroV("frame", "retrograde")
			position.y = position.y + textsize.y * 1.1
			show_text(position, "~" .. Format.Distance(brakeDist), colors.darkgrey, anchor.right, anchor.top, pionillium.medium)

			-- ******************** Frame markers ********************
			show_marker("frame_prograde", symbol.diamond, colors.orbital_marker, true)
			show_marker("frame_retrograde", symbol.cross, colors.orbital_marker, show_retrograde_indicators)
			show_marker("normal", symbol.normal, colors.orbital_marker, false, nil, 8)
			show_marker("anti_normal", symbol.anti_normal, colors.orbital_marker, false, nil, 8)
			show_marker("radial_out", symbol.radial_out, colors.orbital_marker, false, nil, 8)
			show_marker("radial_in", symbol.radial_in, colors.orbital_marker, false, nil, 8)
			show_marker("away_from_frame", symbol.circle, colors.orbital_marker, true)
			local pos,dir = markerPos("frame", reticule_radius + 5)
			if pos then
				 local size = 6
				 local left = dir:left() * 4 + pos
				 local right = dir:right() * 4 + pos
				 local top = dir * 8 + pos
				 pigui.AddTriangleFilled(left, right, top, colors.darkgrey)
			end
			-- ******************** Combat target ********************
			show_marker("combat_target", symbol.circle, colors.combat_target, true)
			show_marker("combat_target_lead", symbol.empty_bullseye, colors.combat_target, false)
	 end
	 -- ******************** NavTarget markers ********************
	 show_marker("nav_prograde", symbol.diamond, colors.lightgreen, true)
	 show_marker("nav_retrograde", symbol.cross, colors.lightgreen, show_retrograde_indicators)
	 show_marker("nav", symbol.square, colors.lightgreen, false)
	 local pos,dir,point,side = markerPos("nav", reticule_radius + 5)
	 if pos then
			local size = 9
			local left = dir:left() * size + pos
			local right = dir:right() * size + pos
			local top = dir * size * 2 + pos
			pigui.AddTriangleFilled(left, right, top, colors.lightgrey)
	 end
	 -- ******************** Maneuver Node ********************
	 show_marker("maneuver", symbol.bullseye, colors.maneuver, true)

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

	 pigui.PushStyleColor("WindowBg", colors.noz_darkblue)

	 show_nav_window()
	 -- show_contacts()

	 show_settings()
	 
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
	 pigui.PushStyleVar("WindowRounding", 0)
	 if should_show_hud then
			show_hud()
	 end

   handle_global_keys()
	 pigui.PopStyleVar(1)
end
