-- TODO:
-- time accel
-- multi-function-display / scanner?
-- lua console?

-- DONE?
-- pause
-- rotation dampening button
-- wheels up/down button
-- blastoff / undock
-- hyperspace
-- 		const SystemPath dest = Pi::player->GetHyperspaceDest();
--  	RefCountedPtr<StarSystem> s = m_game->GetGalaxy()->GetStarSystem(dest);
--    name: dest.IsBodyPath() ? s->GetBodyByPath(dest)->GetName() : s->GetName()
--    m_game->GetHyperspaceArrivalProbability()*100.0
-- map sub buttons
-- alerts
-- comms log
-- hyperspace button
-- set speed / autopilot / manual
-- heading/pitch indicator
-- target hull/shield, general info
-- combat target / lead indicators + line

local Format = import('Format')
local Game = import('Game')
local Space = import('Space')
local Engine = import('Engine')
local Event = import("Event")
local ShipDef = import("ShipDef")
local Vector = import("Vector")
local Color = import("Color")
local Lang = import("Lang")

local lui = Lang.GetResource("ui-core");
local lc = Lang.GetResource("core");
local lec = Lang.GetResource("equipment-core");

local utils = import("utils")


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
local reticule_text_radius = reticule_radius * 1.1

local pi = 3.14159264
local pi_2 = pi / 2
local pi_4 = pi / 4
local two_pi = pi * 2
local standard_gravity = 9.80665

local icons = {
   -- first row
   prograde = 0,
   retrograde = 1,
   radial_out = 2,
   radial_in = 3,
   antinormal = 4,
   normal = 5,
   frame = 6,
   maneuver = 7,
   forward = 8,
   backward = 9,
   down = 10,
   right = 11,
   up = 12,
   left = 13,
   bullseye = 14,
   square = 15,
   -- second row
   prograde_thin = 16,
   retrograde_thin = 17,
   radial_out_thin = 18,
   radial_in_thin = 19,
   antinormal_thin = 20,
   normal_thin = 21,
   frame_away = 22,
   direction = 24,
   direction_hollow = 25,
   direction_frame = 26,
   direction_frame_hollow = 27,
   direction_forward = 28,
   apoapsis = 29,
   periapsis = 30,
   semi_major_axis = 31,
   -- third row
   heavy_fighter = 32,
   medium_fighter = 33,
   light_fighter = 34,
   sun = 35,
   asteroid_hollow = 36,
   current_height = 37,
   current_periapsis = 38,
   current_line = 39,
   current_apoapsis = 40,
   eta = 41,
   altitude = 42,
   gravity = 43,
   eccentricity = 44,
   inclination = 45,
   longitude = 46,
   latitude = 47,
   -- fourth row
   heavy_courier = 48,
   medium_courier = 49,
   light_courier = 50,
   rocky_planet = 51,
   ship = 52, -- useless?
   landing_gear_up = 53,
   landing_gear_down = 54,
   ecm = 55,
   rotation_damping_on = 56,
   rotation_damping_off = 57,
   hyperspace = 58,
   hyperspace_off = 59,
   scanner = 60,
   message_bubble = 61,
   fuel = 63,
   -- fifth row
   heavy_passenger_shuttle = 64,
   medium_passenger_shuttle = 65,
   light_passenger_shuttle = 66,
   moon = 67,
   autopilot_set_speed = 68,
   autopilot_manual = 69,
   autopilot_fly_to = 70,
   autopilot_dock = 71,
   autopilot_hold = 72,
   autopilot_undock = 73,
   autopilot_undock_illegal = 74,
   autopilot_blastoff = 75,
   autopilot_blastoff_illegal = 76,
   autopilot_low_orbit = 77,
   autopilot_medium_orbit = 78,
   autopilot_high_orbit = 79,
   -- sixth row
   heavy_passenger_transport = 80,
   medium_passenger_transport = 81,
   light_passenger_transport = 82,
   gas_giant = 83,
   time_accel_stop = 84,
   time_accel_paused = 85,
   time_accel_1x = 86,
   time_accel_10x = 87,
   time_accel_100x = 88,
   time_accel_1000x = 89,
   time_accel_10000x = 90,
   pressure = 91,
   shield = 92,
   hull = 93,
   temperature = 94,
   -- seventh row
   heavy_cargo_shuttle = 96,
   medium_cargo_shuttle = 97,
   light_cargo_shuttle = 98,
   spacestation = 99,
   time_backward_100x = 100,
   time_backward_10x = 101,
   time_backward_1x = 102,
   time_center = 103,
   time_forward_1x = 104,
   time_forward_10x = 105,
   time_forward_100x = 106,
   filter_bodies = 107,
   filter_stations = 108,
   filter_ships = 109,
   -- eighth row
   heavy_freighter = 112,
   medium_freighter = 113,
   light_freighter = 114,
   starport = 115,
   -- ninth row
   view_internal = 128,
   view_external = 129,
   view_sidereal = 130,
   comms = 131,
   market = 132,
   bbs = 133,
   equipment = 134,
   repairs = 135,
   info = 136,
   personal_info = 137,
   personal = 138,
   rooster = 139,
   map = 140,
   sector_map = 141,
   system_map = 142,
   system_overview = 143,
   -- tenth row
   galaxy_map = 144,
   settings = 145,
   language = 146,
   controls = 147,
   sound = 148,
   new = 149,
   skull = 150,
   mute = 151,
   unmute = 152,
   music = 153,
   zoom_in = 154,
   zoom_out = 155,
   search_lens = 156,
   message = 157,
   message_open = 158,
   search_binoculars = 159,
   -- eleventh row
   planet_grid = 160,
   bookmarks = 161,
   unlocked = 162,
   locked = 163,
   label = 165,
   broadcast = 166,
   shield_other = 167,
   hud = 168,
   factory = 169,
   star = 170,
}

local colors = {
   nav_filter_active = Color(0, 100, 0);
   lightgreen = Color(0, 255, 0),
   darkgreen = Color(0, 150, 0),
   deltav_total = Color(100, 100, 100, 200),
   deltav_remaining = Color(250, 250, 250),
   deltav_current = Color(150, 150, 150),
   deltav_maneuver = Color(168, 168, 255),
   darkgrey = Color(150, 150, 150),
   darkergrey = Color(50, 50, 50),
   orbital_marker = Color(150, 150, 150),
   lightgrey = Color(200, 200, 200),
   time_accel = Color( 100, 100, 150 ),
   time_accel_current = Color( 150, 150, 100 ),
   time_accel_requested = Color( 150, 100, 100 ),
   windowbg = Color(0, 0, 50, 200),
   transparent = Color(0, 0, 0, 0),
   lightred = Color(255, 150, 150),
   red = Color(255, 0, 0 ),
   green = Color(0, 255, 0 ),
   combat_target = Color(200, 100, 100 ),
   maneuver = Color(163, 163, 255 ),
   orbit_gauge_ground = Color(95, 95, 0 ),
   orbit_gauge_atmosphere = Color(97, 97, 241 ),
   orbit_gauge_space = Color(84, 84, 84 ),
   noz_darkblue = Color(6, 7, 38 , 180 ),
   noz_mediumblue = Color(3, 63, 113 ),
   noz_lightblue = Color(49, 102, 144 ),
   shield_gauge = Color(0, 255, 255 ),
   hull_gauge = Color(200, 100, 0 ),
   tmp_gauge = Color(255, 0, 0 ),
   gun_tmp_gauge = Color(200, 100, 0 ),
   gauge_darkergrey = Color(20, 20, 20),
   gauge_darkgrey = Color(35, 35, 35),
   paused_background = Color(0, 0, 0, 150),
   paused_text = Color(200, 150, 50, 150),
   white = Color(255, 255, 255 ),
   black = Color(0, 0, 0 ),
   mode_button = Color(49, 102, 144),
   mode_button_selected = Color(97, 102, 144),
}

-- dummy font, actually renders icons
local pionicons = {
   small = { name = "icons", size = 16, offset = 14 },
   large = { name = "icons", size = 22, offset = 28 }
}

-- these need to be declared in c++ too!
local pionillium = {
   large = { name = "pionillium", size = 30, offset = 24 },
   medium = { name = "pionillium", size = 18, offset = 14 },
   medsmall = { name = "pionillium", size = 15, offset = 12 },
   small = { name = "pionillium", size = 12, offset = 10 }
}

local MyFormat = {
   Duration = function(duration, elements)
	  -- shown elements items (2 -> wd or dh, 3 -> dhm or hms)
	  local seconds = math.floor(duration % 60)
	  local minutes = math.floor(duration / 60 % 60)
	  local hours = math.floor(duration / 60 / 60 % 24)
	  local days = math.floor(duration / 60 / 60 / 24 % 7)
	  local weeks = math.floor(duration / 60 / 60 / 24 / 7)
	  local i = elements or 5
	  local count = false
	  local result = ""
	  if i > 0 then
		 if weeks ~= 0 then
			result = result .. weeks .. "w"
			count = true
		 end
		 if count then
			i = i - 1
		 end
	  end
	  if i > 0 then
		 if days ~= 0 then
			result = result .. days .. "d"
			count = true
		 end
		 if count then
			i = i - 1
		 end
	  end
	  if i > 0 then
		 if hours ~= 0 then
			result = result .. hours .. "h"
			count = true
		 end
		 if count then
			i = i - 1
		 end
	  end
	  if i > 0 then
		 if minutes ~= 0 then
			result = result .. minutes .. "m"
			count = true
		 end
		 if count then
			i = i - 1
		 end
	  end
	  if i > 0 then
		 if seconds ~= 0 then
			result = result .. seconds .. "s"
			count = true
		 end
		 if count then
			i = i - 1
		 end
	  end
	  return result
   end,
   Distance = function(distance)
	  local d = math.abs(distance)
	  if d < 1000 then
		 return math.floor(distance), lc.UNIT_METERS
	  end
	  if d < 1000*1000 then
		 return string.format("%0.1f", distance / 1000), lc.UNIT_KILOMETERS
	  end
	  if d < 1000*1000*1000 then
		 return string.format("%0.1f", distance / 1000 / 1000), lc.UNIT_MILLION_METERS
	  end
	  return string.format("%0.1f", distance / 1.4960e11), lc.UNIT_AU
   end,
   Speed = function(distance)
	  local d = math.abs(distance)
	  if d < 1000 then
		 return math.floor(distance), lc.UNIT_METERS_PER_SECOND
	  end
	  if d < 1000*1000 then
		 return string.format("%0.1f", distance / 1000), lc.UNIT_KILOMETERS_PER_SECOND
	  end
	  return string.format("%0.1f", distance / 1000 / 1000), lc.UNIT_MILLION_METERS_PER_SECOND
	  -- no need for au/s
   end

}

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

local function count(tab)
   local i = 0
   for _,v in pairs(tab) do
	  i = i + 1
   end
   return i
end

local function window(name, params, fun)
   pigui.Begin(name, params)
   fun()
   pigui.End()
end

local function group(fun)
   pigui.BeginGroup()
   fun()
   pigui.EndGroup()
end

local function withFont(name, size, fun)
   pigui.PushFont(name, size)
   fun()
   pigui.PopFont()
end

local function withStyleColors(styles, fun)
   for k,v in pairs(styles) do
	  pigui.PushStyleColor(k, v)
   end
   fun()
   pigui.PopStyleColor(count(styles))
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

local function show_text(pos, text, color, font, anchor_horizontal, anchor_vertical, tooltip)
   local position = Vector(pos.x, pos.y)
   -- AddText aligns to upper left
   local size
   withFont(font.name, font.size, function()
			   size = pigui.CalcTextSize(text)
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
   end)
   if tooltip and not pigui.IsMouseHoveringAnyWindow() and tooltip ~= "" then
	  if pigui.IsMouseHoveringRect(position, position + size, true) then
		 pigui.SetTooltip(tooltip)
	  end
   end
   return Vector(size.x, size.y)
end

local function get_icon_tex(icon)
   assert(icon, "no icon given")
   local count = 16.0 -- icons per row/column
   local rem = math.floor(icon % count)
   local quot = math.floor(icon / count)
   return Vector(rem / count, quot/count), Vector((rem+1) / count, (quot+1)/count)
end

local function show_icon(position, icon, color, size, anchor_horizontal, anchor_vertical, tooltip, angle_rad)
   local pos = calc_alignment(position, Vector(size, size), anchor_horizontal, anchor_vertical)
   local uv0, uv1 = get_icon_tex(icon)
   if angle_rad then
	  local center = (pos + pos + Vector(size,size)) / 2
	  local up_left = Vector(-size/2, size/2):rotate2d(angle_rad)
	  local up_right = up_left:right()
	  local down_left = up_left:left()
	  local down_right = -up_left
	  pigui.AddImageQuad(pigui.icons_id, center + up_left, center + up_right, center + down_right, center + down_left, uv0, Vector(uv1.x, uv0.y), uv1, Vector(uv0.x, uv1.y), color)
   else
	  pigui.AddImage(pigui.icons_id, pos, pos + Vector(size, size), uv0, uv1, color)
   end
   return Vector(size, size)
end

local function icon(icon, size, color)
   local uv0, uv1 = get_icon_tex(icon)
   pigui.Image(pigui.icons_id, Vector(size,size), uv0, uv1, color)
end

local function show_text_fancy(position, texts, colors, fonts, anchor_horizontal, anchor_vertical, tooltips)
   -- always align texts at baseline
   local spacing = 2
   local size = Vector(0, 0)
   local max_offset = 0
   assert(#texts == #colors and #texts == #fonts, "not the same number of texts, colors and fonts")
   for i=1,#texts do
	  local is_icon = fonts[i].name ~= "icons"
	  local s
	  if is_icon then
		 pigui.PushFont(fonts[i].name, fonts[i].size)
		 s = pigui.CalcTextSize(texts[i])
	  else
		 s = Vector(fonts[i].size, fonts[i].size)
	  end
	  size.x = size.x + s.x
	  size.x = size.x + spacing -- spacing
	  size.y = math.max(size.y, s.y)
	  max_offset = math.max(max_offset, fonts[i].offset)
	  if is_icon then
		 pigui.PopFont()
	  end
   end
   size.x = size.x - spacing -- remove last spacing
   position = calc_alignment(position, size, anchor_horizontal, nil)
   if anchor_vertical == anchor.top then
	  position.y = position.y + max_offset
   elseif anchor_vertical == anchor.bottom then
	  position.y = position.y - (size.y - max_offset)
   end
   for i=1,#texts do
	  local is_icon = fonts[i].name ~= "icons"
	  if is_icon then
		 withFont(fonts[i].name, fonts[i].size, function()
					 local s = show_text(position, texts[i], colors[i], fonts[i], anchor.left, anchor.baseline, tooltips and tooltips[i] or nil)
					 position.x = position.x + s.x + spacing
		 end)
	  else
		 local s = show_icon(position, texts[i], colors[i], fonts[i].size, anchor.left, anchor.bottom, tooltips[i])
		 position.x = position.x + s.x + spacing
	  end
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

-- function show_missions()
-- 	 local windowbg = colors.windowbg
-- 	 local SpaceStation = import("SpaceStation")
-- 	 local Game = import('Game')
-- 	 local Space = import('Space')
-- 	 local Format = import('Format')
-- 	 local station = player:GetDockedWith()
-- 	 if not station then
-- 			return
-- 	 end
-- 	 pigui.PushStyleColor("WindowBg", windowbg)
-- 	 pigui.SetNextWindowSize(Vector(pigui.screen_width / 2, pigui.screen_height / 1.5), "FirstUseEver")
-- 	 pigui.Begin("Missions", {})
-- 	 pigui.Columns(2, "missionscolumns", true)
-- 	 pigui.BeginChild("foo");
-- 	 --	 pigui.BeginGroup()
-- 	 for k,v in pairs(station.adverts[station]) do
-- 			pigui.BeginGroup()
-- 			pigui.Text(v.description)
-- 			pigui.Text(v.payout and Format.Money(v.payout) or "-")
-- 			pigui.SameLine()
-- 			if v.system
-- 			then
-- 				 if v.system.index == Game.system.index
-- 						and v.system.sector.x == Game.system.sector.x
-- 						and v.system.sector.y == Game.system.sector.y
-- 						and v.system.sector.z == Game.system.sector.z
-- 				 then
-- 						pigui.Text(Format.Distance(Space.GetBody(v.body.index):DistanceTo(player)))
-- 				 else
-- 						pigui.Text(v.system:DistanceTo(Game.system) .. "ly")
-- 				 end
-- 			else
-- 				 pigui.Text("-")
-- 			end
-- 			pigui.SameLine()
-- 			pigui.Text(v.deadline and Format.Duration(v.deadline - Game.time) or "-")
-- 			--			pigui.Separator()
-- 			pigui.EndGroup()
-- 			if pigui.IsItemClicked(0) then
-- 				 mission_selected = v
-- 			end
-- 			pigui.Dummy(Vector(0,20))
-- 	 end
-- 	 --	 pigui.EndGroup()
-- 	 pigui.EndChild()
-- 	 pigui.NextColumn()
-- 	 if mission_selected then
-- 			pigui.BeginChild("bar")
-- 			pigui.Columns(2, "desccolumns", false)
-- 			local m = mission_selected
-- 			pigui.PushFont("pionillium", 18)
-- 			pigui.Text(m.description)
-- 			pigui.PopFont()
-- 			pigui.Spacing()
-- 			if m.details then
-- 				 pigui.TextWrapped(m.details)
-- 			end
-- 			pigui.Spacing()
-- 			local distance
-- 			if m.system then
-- 				 if m.system.index == Game.system.index
-- 						and m.system.sector.x == Game.system.sector.x
-- 						and m.system.sector.y == Game.system.sector.y
-- 						and m.system.sector.z == Game.system.sector.z
-- 				 then
-- 						distance = Format.Distance(Space.GetBody(m.body.index):DistanceTo(player))
-- 				 else
-- 						distance = math.floor(m.system:DistanceTo(Game.system) * 10) / 10 .. "ly"
-- 				 end
-- 			end

-- 			pigui.Text("Destination: " .. (m.system and m.system.name or "-") .. ", " .. (m.body and m.body.name or "-") .. " (" .. distance .. ")")
-- 			pigui.Text("Deadline: " .. Format.Date(m.deadline) .. " (" .. (m.deadline and Format.Duration(m.deadline - Game.time) or "-") .. ")")
-- 			pigui.Text("Wage: " .. (m.payout and Format.Money(m.payout) or "-"))
-- 			pigui.Spacing()

-- 			pigui.NextColumn()
-- 			pigui.Dummy(Vector(100,100))
-- 			pigui.Text("Image")
-- 			pigui.PushFont("pionillium", 18)
-- 			pigui.Text(m.client or "Anonymous")
-- 			pigui.PopFont()
-- 			pigui.PushItemWidth(-1)
-- 			if pigui.Button("OK, I'll do it.") then
-- 				 print("cool")
-- 			end
-- 			if pigui.Button("Hang up") then
-- 				 mission_selected = nil
-- 				 print("then not")
-- 			end
-- 		  pigui.PopItemWidth()
-- 			pigui.EndChild()
-- 	 end
-- 	 pigui.End()
-- 	 pigui.PopStyleColor(1)
-- end

local function show_settings()
   window("Settings",
		  {},
		  function ()
			 local _,sri = pigui.Checkbox("Show retrograde indicators", show_retrograde_indicators);
			 show_retrograde_indicators = sri
			 for k,v in pairs(colors) do

				local changed, r, g, b, a = pigui.DragInt4(k, v.r or 0, v.g or 0, v.b or 0, v.a or 255, 1.0, 0, 255)
				if changed then
				   v.r = r
				   v.g = g
				   v.b = b
				   v.a = a
				end
			 end
   end)
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
   local ratio_end = arc_start + range * utils.clamp(ratio, 0, 1)

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
   show_text(center, math.ceil(ratio * 100), colors.lightgrey, center_text_font, anchor.center, anchor.center)
   show_text(center + Vector(4, radius), main_text, colors.lightgrey, main_text_font, anchor.left, anchor.center)
   if small_text then
	  show_text(center + Vector(radius, radius):normalized()*radius, small_text, colors.lightgrey, small_text_font, anchor.left, anchor.bottom)
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
   local spd,unit = MyFormat.Speed(deltav_remaining)
   local position = point_on_circle_radius(navball_center, navball_text_radius, -3)
   show_text_fancy(position, { lui.HUD_DELTA_V, spd, unit }, { colors.darkgrey, colors.lightgrey, colors.darkgrey }, { pionillium.medium, pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)
   local time_until_empty = deltav_remaining / player:GetAccel("forward")
   show_text_fancy(position, { math.floor(dvr*100) .. "%     " .. Format.Duration(time_until_empty) }, { colors.lightgrey }, { pionillium.small }, anchor.right, anchor.top)
   if deltav_maneuver > 0 then
	  local spd,unit = MyFormat.Speed(deltav_maneuver)
	  position = point_on_circle_radius(navball_center, navball_text_radius, -4)
	  show_text_fancy(position, { lui.MANEUVER_DELTA_V, spd, unit }, { colors.maneuver, colors.maneuver, colors.maneuver }, { pionillium.medium, pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)
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
		 local textsize = show_text_fancy(position,
										  { icons.apoapsis, dist_apo, unit_apo },
										  { colors.lightgrey, colors.lightgrey, colors.darkgrey },
										  { pionicons.small, pionillium.medium, pionillium.small },
										  anchor.left,
										  anchor.baseline,
										  { lui.HUD_TOOLTIP_APOAPSIS_DISTANCE, lui.HUD_TOOLTIP_APOAPSIS_DISTANCE, lui.HUD_TOOLTIP_APOAPSIS_DISTANCE })
		 show_text(position + Vector(textsize.x * 1.2), lui.HUD_T_MINUS .. Format.Duration(o_time_at_apoapsis), (o_time_at_apoapsis < 30 and colors.lightgreen or colors.lightgrey), pionillium.small, anchor.left, anchor.baseline, lui.HUD_TOOLTIP_APOAPSIS_TIME)
	  end
   end
   -- altitude
   local alt, vspd, lat, lon = player:GetGPS()
   if alt then
	  local altitude,unit = MyFormat.Distance(alt)
	  local position = point_on_circle_radius(navball_center, navball_text_radius, 2.6)
	  local textsize = show_text_fancy(position,
									   { icons.altitude, altitude, unit },
									   {colors.lightgrey, colors.lightgrey, colors.darkgrey },
									   { pionicons.large, pionillium.large, pionillium.medium },
									   anchor.left,
									   anchor.baseline,
									   { lui.HUD_TOOLTIP_ALTITUDE, lui.HUD_TOOLTIP_ALTITUDE, lui.HUD_TOOLTIP_ALTITUDE } )
	  local vspeed, unit = MyFormat.Speed(vspd)
	  show_text_fancy(position + Vector(textsize.x * 1.1),
					  { (vspd > 0 and "+" or "") .. vspeed, unit },
					  { (vspd < 0 and colors.lightred or colors.lightgreen), colors.darkgrey },
					  {pionillium.medium, pionillium.small },
					  anchor.left,
					  anchor.baseline,
					  { lui.HUD_TOOLTIP_CHANGE_IN_ALTITUDE, lui.HUD_TOOLTIP_CHANGE_IN_ALTITUDE })
   end
   -- periapsis
   if not player:IsDocked() then
	  local position = point_on_circle_radius(navball_center, navball_text_radius, 3)
	  local pa_d = pa - frame_radius
	  local dist_per, unit_per = MyFormat.Distance(pa_d)
	  if pa and pa_d ~= 0 then
		 local textsize = show_text_fancy(position,
										  { icons.periapsis, dist_per, unit_per, "     " .. lui.HUD_T_MINUS .. Format.Duration(o_time_at_periapsis) },
										  { colors.lightgrey, (pa - frame_radius < 0 and colors.lightred or colors.lightgrey), colors.darkgrey, (o_time_at_periapsis < 30 and colors.lightgreen or colors.lightgrey) },
										  { pionicons.small, pionillium.medium, pionillium.small, pionillium.small },
										  anchor.left,
										  anchor.baseline,
										  { lui.HUD_TOOLTIP_PERIAPSIS_DISTANCE, lui.HUD_TOOLTIP_PERIAPSIS_DISTANCE, lui.HUD_TOOLTIP_PERIAPSIS_DISTANCE, lui.HUD_TOOLTIP_PERIAPSIS_TIME })
	  end
   end
   -- inclination, eccentricity
   if not player:IsDocked() then
	  local position = point_on_circle_radius(navball_center, navball_text_radius, 3.4)
	  show_text_fancy(position,
					  { icons.inclination, math.floor(o_inclination / two_pi * 360) .. lc.UNIT_DEGREES, "    ", icons.eccentricity, string.format("%.02f", o_eccentricity)},
					  { colors.lightgrey, colors.lightgrey, colors.lightgrey, colors.lightgrey, colors.lightgrey },
					  { pionicons.small, pionillium.medium, pionillium.medium, pionicons.small, pionillium.medium },
					  anchor.left, anchor.baseline,
					  {lui.HUD_TOOLTIP_INCLINATION, lui.HUD_TOOLTIP_INCLINATION, lui.HUD_TOOLTIP_INCLINATION, lui.HUD_TOOLTIP_ECCENTRICITY, lui.HUD_TOOLTIP_ECCENTRICITY})
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
	  local tooltips = {}
	  if pressure and pressure > 0.001 then
		 table.insert(txts, icons.pressure)
		 table.insert(txts, string.format("%.02f", pressure))
		 table.insert(txts, lc.UNIT_ATM)
		 table.insert(cols, colors.lightgrey)
		 table.insert(cols, colors.lightgrey)
		 table.insert(cols, colors.darkgrey)
		 table.insert(fnts, pionicons.small)
		 table.insert(fnts, pionillium.medium)
		 table.insert(fnts, pionillium.small)
		 table.insert(tooltips, lui.HUD_TOOLTIP_PRESSURE)
		 table.insert(tooltips, lui.HUD_TOOLTIP_PRESSURE)
		 table.insert(tooltips, lui.HUD_TOOLTIP_PRESSURE)
	  end
	  if pressure and pressure > 0.001 and gravity then
		 table.insert(txts, "    ")
		 table.insert(cols, colors.darkgrey)
		 table.insert(fnts, pionillium.small)
		 table.insert(tooltips, "")
	  end
	  if gravity then
		 table.insert(txts, icons.gravity)
		 table.insert(txts, gravity)
		 table.insert(txts, lc.UNIT_GRAVITY)
		 table.insert(cols, colors.lightgrey)
		 table.insert(cols, colors.lightgrey)
		 table.insert(cols, colors.darkgrey)
		 table.insert(fnts, pionicons.small)
		 table.insert(fnts, pionillium.medium)
		 table.insert(fnts, pionillium.small)
		 table.insert(tooltips, lui.HUD_TOOLTIP_GRAVITY)
		 table.insert(tooltips, lui.HUD_TOOLTIP_GRAVITY)
		 table.insert(tooltips, lui.HUD_TOOLTIP_GRAVITY)
	  end
	  show_text_fancy(position, txts, cols, fnts, anchor.left, anchor.baseline, tooltips)
	  -- latitude, longitude
	  position = point_on_circle_radius(navball_center, navball_text_radius, 4.5)
	  if lat and lon then
		 local textsize = show_text_fancy(position,
										  { icons.latitude, lat },
										  { colors.lightgrey, colors.lightgrey },
										  { pionicons.small, pionillium.medium },
										  anchor.left,
										  anchor.baseline,
										  { lui.HUD_TOOLTIP_LATITUDE, lui.HUD_TOOLTIP_LATITUDE })
		 show_text_fancy(position + Vector(0, textsize.y * 1.2),
						 { icons.longitude, lon },
						 { colors.lightgrey, colors.lightgrey },
						 { pionicons.small, pionillium.medium },
						 anchor.left,
						 anchor.baseline,
						 { lui.HUD_TOOLTIP_LONGITUDE, lui.HUD_TOOLTIP_LONGITUDE })
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
	  local my_height = utils.clamp((my_position - min_height) / range, ends/2, 1 - ends/2)
	  local apoapsis = aa > 0 and utils.clamp((aa - min_height) / range, ends, 1 - ends/2) or nil
	  local periapsis = utils.clamp((pa - min_height) / range, ends, 1 - ends/2)
	  local atmosphere_ratio = frame_sb.hasAtmosphere and math.max(ends, (atmosphere_height - min_height) / range - 2 * ends) or 0
	  orbit_gauge(navball_center, navball_radius + 5, colors.orbit_gauge_space, thickness * 0.99, 0.0, 1.0)
	  orbit_gauge(navball_center, navball_radius + 5, colors.orbit_gauge_ground, thickness, 0, ends)
	  orbit_gauge(navball_center, navball_radius + 5, colors.orbit_gauge_atmosphere, thickness, ends, ends + atmosphere_ratio)

	  local size = 12
	  show_icon(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, my_height), icons.current_height, colors.lightgrey, size, anchor.center, anchor.center)
	  if apoapsis then
		 local self_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, apoapsis)
		 local close_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, apoapsis * 1.05)
		 local dir = (close_position - self_position):normalized()
		 local angle = dir:angle() - math.pi
		 show_icon(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, apoapsis), icons.current_apoapsis, colors.lightgrey, size, anchor.center, anchor.center, "", angle)
	  end
	  do
		 local self_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, periapsis)
		 local close_position = orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, periapsis * 0.95)
		 local dir = (close_position - self_position):normalized()
		 local angle = dir:angle()
		 show_icon(orbit_gauge_position(navball_center, navball_radius + 5 + thickness*0.5, periapsis), icons.current_periapsis, colors.lightgrey, size, anchor.center, anchor.center, "", angle)
	  end
   end
   -- compass
   do
	  local heading, pitch = player:GetHeadingPitch("planet")
	  do
		 local function pitchline(hrs, radius, color, thickness)
			local a = point_on_circle_radius(navball_center, navball_radius - 2 - radius, hrs)
			local b = point_on_circle_radius(navball_center, navball_radius + radius, hrs)
			pigui.AddLine(a, b, color, thickness)
		 end
		 pitchline(3, 1, colors.darkgrey, 2)
		 pitchline(2.25, 1, colors.darkgrey, 2)
		 pitchline(3.75, 1, colors.darkgrey, 2)
		 pitchline(1.5, 2, colors.darkgrey, 2)
		 pitchline(4.5, 2, colors.darkgrey, 2)
		 local xpitch = ((pitch / two_pi * 360) + 90) / 180
		 local xpitch_h = 4.5 - xpitch * 3
		 pitchline(xpitch_h, 3, colors.lightgrey, 2)
	  end
	  
	  heading = heading / two_pi * 360
	  local relevant = {}
	  local directions = { [0] = "N", [45] = "NE", [90] = "E", [135] = "SE", [180] = "S", [225] = "SW", [270] = "W", [315] = "NW" }
	  local function cl(x)
		 if x < 0 then
			return cl(x + 360)
		 elseif x >= 360 then
			return cl(x - 360)
		 else
			return x
		 end
	  end
	  local left = math.floor(heading - 45)
	  local right = left + 90
	  local d = left

	  pigui.PathArcTo(navball_center, navball_radius + 5, - pi_2 - pi_4 + 0.05, -pi_2 + pi_4 - 0.05, 64)
	  pigui.PathStroke(colors.lightgrey, false, 3)
	  local function stroke(d, p, n, height)
		 if d % n == 0 then
			local a = point_on_circle_radius(navball_center, navball_radius + 5, 2.8 * p - 1.4)
			local b = point_on_circle_radius(navball_center, navball_radius + 5 + height, 2.8 * p - 1.4)
			pigui.AddLine(a, b, colors.lightgrey, 2)
		 end
	  end
	  while true do
		 if d > right then
			break
		 end
		 local p = (d - left) / 90
		 stroke(d, p, 7.5, 5)
		 stroke(d, p, 45, 8)
		 stroke(d, p, 90, 10)
		 for k,v in pairs(directions) do
			if cl(k) == cl(d) then
			   local a = point_on_circle_radius(navball_center, navball_radius + 5 + 8, 3 * p - 1.5)
			   show_text(a, v, colors.lightgrey, pionillium.small, anchor.center, anchor.bottom, "")
			end
		 end
		 d = d + 1
	  end
   end
   -- circular stats, lower left
   local position = Vector(navball_center.x - 180,pigui.screen_height - 40)
   show_circular_gauge(position, player:GetHullTemperature(), colors.tmp_gauge, lui.HUD_TEMPERATURE, lui.HUD_HULL)
   show_circular_gauge(position + Vector(-90, 0), 1 - player:GetHullPercent() / 100, colors.hull_gauge, lui.HUD_DAMAGE, lui.HUD_HULL)
   if player:GetShieldsPercent() then
	  show_circular_gauge(position + Vector(-180, 0), player:GetShieldsPercent() / 100, colors.shield_gauge, lui.HUD_SHIELD)
   end
end

local radial_nav_target = nil

Event.Register("onHyperspace", function (target)
				  radial_nav_target = nil
end)

local function show_comm_log()
   window("CommLog", {}, function ()
			 for k,v in pairs(Game.comms_log_lines) do
				pigui.Text(Format.Duration(Game.time - v.time) .. " " .. (v.sender and (v.sender .. ": ") or "") .. v.text)
			 end
   end)
end

local radial_actions = {
   dock = function(body) player:AIDockWith(body); player:SetNavTarget(body) end, -- TODO check for autopilot
   fly_to = function(body) player:AIFlyTo(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
   low_orbit = function(body) player:AIEnterLowOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
   medium_orbit = function(body) player:AIEnterMediumOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
   high_orbit = function(body) player:AIEnterHighOrbit(body); player:SetNavTarget(body)  end, -- TODO check for autopilot
   clearance = function(body) local msg = player:RequestDockingClearance(body); Game.AddCommsLogLine(msg, body.label); player:SetNavTarget(body)  end, -- TODO check for room for docking, or make it so you can be denied
   radial_in = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-radial-in") end,
   radial_out = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-radial-out") end,
   normal = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-normal") end,
   anti_normal = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-antinormal") end,
   prograde = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-prograde") end,
   retrograde = function(body) player:SetNavTarget(body); player:SetFlightControlState("fix-heading-retrograde") end,
   -- hypercloud_analyzer: 	Pi::game->GetSectorView()->SetHyperspaceTarget(cloud->GetShip()->GetHyperspaceDest());
}

local should_show_radial_menu = false

local function show_radial_menu()
   pigui.DisableMouseFacing(true)
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
	  local uv0, uv1 = get_icon_tex(icon)
	  table.insert(items, { id = pigui.icons_id, uv0 = uv0, uv1 = uv1 })
	  table.insert(actions, action)
	  table.insert(tooltips, item)
   end

   if radial_nav_target then
	  local typ = radial_nav_target.superType
	  if typ == "STARPORT" then
		 addItem(icons.comms, lui.HUD_RADIAL_TOOLTIP_COMMS, "clearance")
		 addItem(icons.autopilot_dock, lui.HUD_RADIAL_TOOLTIP_AUTO_DOCK, "dock")
	  end
	  addItem(icons.autopilot_fly_to, lui.HUD_RADIAL_TOOLTIP_FLY_TO, "fly_to")
	  if typ == "STAR" or typ == "ROCKY_PLANET" or typ == "GAS_GIANT" then
		 addItem(icons.autopilot_low_orbit, lui.HUD_RADIAL_TOOLTIP_LOW_ORBIT, "low_orbit")
		 addItem(icons.autopilot_medium_orbit, lui.HUD_RADIAL_TOOLTIP_MEDIUM_ORBIT, "medium_orbit")
		 addItem(icons.autopilot_high_orbit, lui.HUD_RADIAL_TOOLTIP_HIGH_ORBIT, "high_orbit")
	  end
	  local frame = player:GetFrame()
	  if frame and frame:GetSystemBody() and radial_nav_target:GetSystemBody() and frame:GetSystemBody().index == radial_nav_target:GetSystemBody().index then
		 addItem(icons.radial_in, "Hold Radial in", "radial_in")
		 addItem(icons.radial_out, "Hold radial out", "radial_out")
		 addItem(icons.normal, "Hold normal", "normal")
		 addItem(icons.antinormal, "Hold anti-normal", "anti_normal")
		 addItem(icons.prograde, "Hold prograde", "prograde")
		 addItem(icons.retrograde, "Hold retrograde", "retrograde")
	  end
   end
   local n = pigui.RadialMenu(radial_menu_center, "##piepopup", items, "pionicons", 30, tooltips) -- pionicons.large.name, pionicons.large.size
   if n == -2 then
	  should_show_radial_menu = false
	  pigui.DisableMouseFacing(false)
	  pigui.SetMouseButtonState(3, false);  -- hack, imgui lets the press go through, but eats the release, so Pi still thinks rmb is held
   elseif n >= 0 then
	  local action = actions[n + 1]
	  radial_actions[action](radial_nav_target)
	  should_show_radial_menu = false
	  pigui.DisableMouseFacing(false)
	  pigui.SetMouseButtonState(3, false);  -- hack, imgui lets the press go through, but eats the release, so Pi still thinks rmb is held
   end
end

local function get_body_icon(body)
   local typ = body.type
   local superType = body.superType
   if superType == "STAR" then
	  return icons.sun
   elseif superType == "GAS_GIANT" then
	  return icons.gas_giant
   elseif superType == "ROCKY_PLANET" then
	  sb = body:GetSystemBody()
	  if sb.parent.superType == "STAR" then
		 return icons.rocky_planet
	  else
		 return icons.moon
	  end
   else -- if superType == "STARPORT" then
	  if typ == "STARPORT_ORBITAL" then
		 return icons.spacestation
	  elseif typ == "STARPORT_SURFACE" then
		 return icons.starport
	  else -- ship
		 local shipdef = ShipDef[body.shipId]
		 local class = shipdef.shipClass
		 assert(class)
		 return icons[class]
	  end
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
	  elseif typ == "STARPORT_SURFACE" then
		 return "p"
	  else -- ship
		 return "h"
	  end
   end
end

local function get_hierarchical_bodies(filter)
   local count = 0
   local body_paths = system:GetBodyPaths()
   -- create intermediate structure
   local data = {} -- list of items systemBody, body, distance, name, parent, children
   local sb_children = {} -- map sb.index to list of children (sb.indices)
   local sb_lookup = {} -- map sb.index to item
   --	 local body_path_count = 0
   -- go through all bodies in system
   for _,system_path in pairs(body_paths) do
	  --			body_path_count = body_path_count + 1
	  local system_body = system_path:GetSystemBody()
	  if system_body then
		 local parent = system_body.parent
		 if parent then
			local parent_children = sb_children[parent.index] or {}
			table.insert(parent_children, system_body)
			sb_children[parent.index] = parent_children
		 end
		 local body = Space.GetBody(system_body.index)
		 if body then
			local distance = player:DistanceTo(body)
			local item = { systemBody = system_body, body = body, distance = distance, name = system_body.name, parent = parent, children = children, hidden = false }
			count = count + 1
			table.insert(data, item)
			sb_lookup[system_body.index] = item
		 end
	  end
   end
   -- go through all bodies, set parent and children
   for _,body in pairs(data) do
	  local ch = sb_children[body.systemBody.index]
	  local children = {}
	  if ch then
		 for _,sb in pairs(ch) do
			table.insert(children, sb_lookup[sb.index])
		 end
		 body.children = children
	  end
   end
   -- space ships
   local bodies = Space.GetBodies(function (body) return body:IsShip() and player:DistanceTo(body) < 100000000 end)
   for _,body in pairs(bodies) do
	  local parent = body:GetDockedWith()
	  if not parent then
		 parent = body.frameBody
	  end
	  local parent_sb = parent:GetSystemBody()
	  if parent_sb then
		 local ch = sb_lookup[parent_sb.index].children
		 if ch == nil then
			ch = {}
			sb_lookup[parent_sb.index].children = ch
		 end
		 local this = { systemBody = nil, body = body, distance = player:DistanceTo(body), name = body.label, parent = parent_sb, children = nil, hidden = false }
		 table.insert(ch, this)
		 table.insert(data, this)
		 count = count + 1
	  else
		 error("systemBody is nil for " .. (parent.label or parent.name))
	  end
   end
   -- filter

   if filter then
	  for _,body in pairs(data) do
		 body.hidden = true
	  end
	  for _,body in pairs(data) do
		 local visible = filter(body.body)
		 if visible then
			body.hidden = false
		 end
		 if not body.hidden and body.parent then
			local parent = sb_lookup[body.parent.index]
			if parent then
			   repeat
				  parent.hidden = false
				  local pp = parent.parent
				  if pp then
					 parent = sb_lookup[pp.index]
				  else
					 parent = nil
				  end
			   until parent == nil
			end
		 end
	  end
   end
   -- only return stars
   local suns = {}
   for _,body in pairs(data) do
	  if body.systemBody and body.systemBody.superType == "STAR" then
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

local filter_ship = true
local filter_station = true
local filter_planet = true

local function image_button(icon, size, bg_color, fg_color, tooltip)
   if not tooltip then
	  error("must supply tooltip for image buttons")
   end
   pigui.PushID(tooltip)
   local uv0, uv1 = get_icon_tex(icon)
   local res = pigui.ImageButton(pigui.icons_id, Vector(size, size), uv0, uv1, -1, bg_color, fg_color)
   if pigui.IsItemHovered() then
	  pigui.SetTooltip(tooltip)
   end
   pigui.PopID(tooltip)
   return res
end

local function show_nav_window()
   -- ******************** Navigation Window ********************
   --	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
   --	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
   window(lui.HUD_WINDOW_NAVIGATION, {}, function ()
			 do
				local fs = filter_ship
				if fs then
				   pigui.PushStyleColor("Button", colors.nav_filter_active)
				   pigui.PushStyleColor("ButtonHovered", colors.nav_filter_active:tint(0.3))
				end
				if image_button(icons.filter_ships, 24, colors.transparent, colors.lightgrey, "Filter Ships") then --  pigui.Button("ships")
				   filter_ship = not filter_ship
				end
				if fs then
				   pigui.PopStyleColor(2)
				end
			 end
			 pigui.SameLine()
			 do
				local fs = filter_station
				if fs then
				   pigui.PushStyleColor("Button", colors.nav_filter_active)
				   pigui.PushStyleColor("ButtonHovered", colors.nav_filter_active:tint(0.3))
				end
				if image_button(icons.filter_stations, 24, colors.transparent, colors.lightgrey, "Filter Stations") then
				   filter_station = not filter_station
				end
				if fs then
				   pigui.PopStyleColor(2)
				end
			 end
			 pigui.SameLine()
			 do
				local fs = filter_planet
				if fs then
				   pigui.PushStyleColor("Button", colors.nav_filter_active)
				   pigui.PushStyleColor("ButtonHovered", colors.nav_filter_active:tint(0.3))
				end
				if image_button(icons.filter_bodies, 24, colors.transparent, colors.lightgrey, "Filter Bodies") then
				   filter_planet = not filter_planet
				end
				if fs then
				   pigui.PopStyleColor(2)
				end
			 end
			 -- pigui.PopFont()
			 pigui.Columns(2, "navcolumns", false)
			 local filter = nil
			 if filter_ship or filter_station or filter_planet or filter_moon then
				filter = function(body)
				   if filter_ship and body:IsShip() then
					  return true
				   end
				   if filter_station and (body:IsSpaceStation() or body:IsStarPort()) then
					  return true
				   end
				   if filter_planet and (body:IsGasGiant() or body:IsRockyPlanet()) and not body:IsMoon() then
					  return true
				   end
				   if filter_moon and body:IsMoon() then
					  return true
				   end
				   return false
				end
			 end
			 local data,count = get_hierarchical_bodies(filter)
			 local nav_target = player:GetNavTarget()
			 local combat_target = player:GetCombatTarget()
			 local lines = 0
			 local function AddLine(data, indent)
				--			print("AddLine " .. spaces(indent) .. data.body.label)
				lines = lines + 1
				if not data.hidden then
				   group(function()
						 pigui.Dummy(Vector(indent * 10, 0))
						 pigui.SameLine()
						 icon(get_body_icon(data.body), 14, colors.lightgrey)
						 -- pigui.PushFont("pionicons", 12)
						 -- pigui.Text(spaces(indent) .. get_body_icon_letter(data.body))
						 -- --			if(pigui.Selectable(spaces(indent) .. get_body_icon_letter(data.body), nav_target == data.body, {"SpanAllColumns"})) then
						 -- --				 player:SetNavTarget(data.body)
						 -- --			end
						 -- pigui.PopFont()
						 pigui.SameLine()
						 if(pigui.Selectable(data.name, nav_target == data.body, {"SpanAllColumns"})) then
							if data.body:IsShip() then
							   if combat_target == data.body then
								  player:SetCombatTarget(nil)
							   else
								  player:SetCombatTarget(data.body)
							   end
							else
							   if nav_target == data.body then
								  player:SetNavTarget(nil)
							   else
								  player:SetNavTarget(data.body)
							   end
							end
						 end
						 pigui.NextColumn()
						 pigui.Text(Format.Distance(data.distance))
						 pigui.NextColumn()
				   end)
				   if pigui.IsItemClicked(1) then
					  pigui.OpenPopup("##piepopup")
					  radial_nav_target = data.body
					  should_show_radial_menu = true
				   end
				   for _,data in pairs(data.children or {}) do
					  AddLine(data, indent + 1)
				   end
				end
			 end
			 -- sort by distance
			 table.sort(data, function(a,b) return a.distance < b.distance end)
			 -- display
			 for key,data in pairs(data) do
				AddLine(data, 0)
			 end
			 -- how do we deal with filtering here :-/
			 -- if lines ~= count then
			 -- 		print_r(data)
			 -- 		error("nok: " .. count .. " count vs. lines " .. lines)
			 -- end
			 if should_show_radial_menu then
				show_radial_menu()
			 end
   end)
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
		 if pigui.Selectable(string.interp(lui.HUD_SET_LOW_THRUST_TO, { percent = setting }), false, {}) then
			player:SetLowThrustPower(setting / 100)
		 end
	  end
	  pigui.EndPopup()
   end
   local size = pigui.CalcTextSize(math.floor(low_thrust_power * 100))
   pigui.AddText(position - Vector(size.x/2, size.y/2), colors.lightgrey, math.floor(low_thrust_power * 100))
   local time_position = Vector(30, pigui.screen_height - 90)
   local year, month, day, hour, minute, second = Game.GetDateTime()
   withFont("pionillium", 18, function()
			   local months = { "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec" }
			   local ymd = string.format("%04d %s. %02d ", year, months[month], day)
			   local hms = string.format("%02d:%02d:%02d", hour, minute, second)
			   local size = pigui.CalcTextSize(ymd)
			   pigui.AddText(time_position, colors.darkgrey, ymd)
			   pigui.AddText(time_position + Vector(size.x, 0), colors.lightgrey, hms)
   end)
   local position = Vector(100, pigui.screen_height - 150)
   local step = 12
   local distance = 2

   local thrust_fb_center = position + Vector(step * 5, 0)
   draw_thrust_fwd(thrust_fb_center + Vector(0, distance), step, thrust.z < 0 and math.abs(thrust.z) or 0)
   draw_thrust_bwd(thrust_fb_center + Vector(0,-distance), step, thrust.z > 0 and math.abs(thrust.z) or 0)

   draw_thrust_up(position + Vector(0, distance), step, thrust.y > 0 and math.abs(thrust.y) or 0)
   draw_thrust_down(position + Vector(0, -distance), step, thrust.y < 0 and math.abs(thrust.y) or 0)
   draw_thrust_left(position + Vector(-distance, 0), step, thrust.x > 0 and math.abs(thrust.x) or 0)
   draw_thrust_right(position + Vector(distance, 0), step, thrust.x < 0 and math.abs(thrust.x) or 0)

   -- ******************** directional spaceship markers ********************
   do
	  local vel = player:GetOrientedVelocity()
	  local max = math.max(math.abs(vel.x),(math.max(math.abs(vel.y), math.abs(vel.z))))
	  local size = 2*step
	  local thickness = 5
	  local velocity_center = Vector(50,50)

	  if max < size then
		 max = size
	  end

	  local vx,vy,vz = vel.x/max, -vel.y/max, vel.z/max

	  -- pigui.AddText(velocity_center + Vector(-13,-size*1.5-8), colors.darkgrey, "L")
	  -- pigui.AddText(velocity_center + Vector(-13,size*1.5), colors.darkgrey, "R")
	  pigui.AddLine(position + Vector(-size, step*3), position + Vector(size, step*3), colors.darkergrey, thickness)
	  pigui.AddLine(position + Vector(0,step * 3), position + Vector(vx * size, step * 3), colors.lightgrey, thickness)
	  -- pigui.AddText(velocity_center + Vector(-3,-size*1.5-8), colors.darkgrey, "U")
	  -- pigui.AddText(velocity_center + Vector(-3,size*1.5), colors.darkgrey, "D")
	  pigui.AddLine(position + Vector(-step*3, -size), position + Vector(-step*3, size), colors.darkergrey, thickness)
	  pigui.AddLine(position + Vector(-step*3,0), position + Vector(-step*3, vy * size), colors.lightgrey, thickness)
	  -- pigui.AddText(velocity_center + Vector(7,-size*1.5-8), colors.darkgrey, "F")
	  -- pigui.AddText(velocity_center + Vector(7,size*1.5), colors.darkgrey, "B")
	  pigui.AddLine(thrust_fb_center + Vector(-20, - size), thrust_fb_center + Vector(-20, size), colors.darkergrey, thickness)
	  pigui.AddLine(thrust_fb_center + Vector(-20,0), thrust_fb_center + Vector(-20, vz * size), colors.lightgrey, thickness)
   end

end

local function show_debug_orbit()
   window("Orbit", {}, function ()
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
   end)
end

local function show_debug_thrust()
   -- thrusters
   window("Thrusters", {}, function ()
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
   end)
end

local function show_debug_temp()
   window("Temperatures", {}, function ()
			 pigui.Text("Hull Temperature: " .. player:GetHullTemperature())
			 pigui.Text("Gun 0 Temperature: " .. player:GetGunTemperature(0))
			 pigui.Text("Gun 1 Temperature: " .. player:GetGunTemperature(1))
			 pigui.Text("Hull percent: " .. player:GetHullPercent())
			 pigui.Text("Shields percent: " .. player:GetShieldsPercent())
   end)
end

local function show_debug_gravity()
   window("Gravity", {}, function ()
			 local g = player:GetGravity()
			 local gr = Vector(g.x, g.y, g.z)
			 pigui.Text("Gravity: " .. string.format("%0.2f", gr:magnitude() / standard_gravity) .. ", " .. g.x .. "/" .. g.y .. "/" .. g.z)
   end)
end

local next_cam_type = { ["internal"] = "external", ["external"] = "sidereal", ["sidereal"] = "internal" }

local function select_world_view()
   local current_cam_type = Game.GetWorldCamType()
   if Game.GetView() == "world" then
	  Game.SetWorldCamType(next_cam_type[current_cam_type])
   else
	  Game.SetView("world")
	  Game.SetWorldCamType("internal")
   end
end

local function handle_global_keys()
   -- if pigui.IsKeyReleased(pigui.keys.f12) then
   -- 		if Game.GetTimeAcceleration() == "paused" then
   -- 			 Game.SetTimeAcceleration("1x", true)
   -- 		else
   -- 			 Game.SetTimeAcceleration("paused", false)
   -- 			 return
   -- 		end
   -- end
   if pigui.IsKeyReleased(pigui.keys.f1) then -- ShipCpanel.cpp:317
	  select_world_view()
   end
   if pigui.IsKeyReleased(pigui.keys.f2) then
	  if Game.GetView() == "sector" then
		 Game.SetView("system")
	  elseif Game.GetView() == "system" then
		 Game.SetView("system_info")
	  elseif Game.GetView() == "system_info" then
		 Game.SetView("galaxy")
	  elseif Game.GetView() == "galaxy" then
		 Game.SetView("sector")
	  else
		 Game.SetView("sector")
	  end
   end
   if pigui.IsKeyReleased(pigui.keys.f3) then
	  Game.SetView("info")
   end
   if pigui.IsKeyReleased(pigui.keys.f4) and player:IsDocked() then
	  Game.SetView("space_station")
   end
   local view = Game.GetView()
   local is_map_view = view == "system" or view == "sector" or view == "system_info" or view == "galaxy"
   if pigui.IsKeyReleased(pigui.keys.f5) then
	  if is_map_view then
		 Game.SetView("sector")
	  elseif (player:IsDocked() or player:IsLanded()) then
		 player:TakeOff()
	  end
   end
   if pigui.IsKeyReleased(pigui.keys.f6) then
	  if is_map_view then
		 Game.SetView("system")
	  else
		 player:ToggleWheelState()
	  end
   end
   if pigui.IsKeyReleased(pigui.keys.f7) then
	  if is_map_view then
		 Game.SetView("system_info")
	  else

		 if player:IsHyperspaceActive() then
			player:AbortHyperjump()
			Game.AddCommsLogLine("Hyperspace jump aborted", nil)
		 else
			player:HyperjumpTo(player:GetHyperspaceTarget())
			Game.AddCommsLogLine("Hyperjump started" , nil)
		 end
	  end
   end
   if pigui.IsKeyReleased(pigui.keys.f8) then
	  if is_map_view then
		 Game.SetView("galaxy")
	  end
   end
   if pigui.IsKeyReleased(pigui.keys.right) then
	  Game.ChangeInternalCameraDirection("right")
   end
   if pigui.IsKeyReleased(pigui.keys.up) then
	  Game.ChangeInternalCameraDirection("front")
   end
   if pigui.IsKeyReleased(pigui.keys.left) then
	  Game.ChangeInternalCameraDirection("left")
   end
   if pigui.IsKeyReleased(pigui.keys.down) then
	  Game.ChangeInternalCameraDirection("rear")
   end

   if pigui.IsKeyReleased(pigui.keys.escape) then
	  if Game.GetTimeAcceleration() ~= "paused" then
		 Game.SetTimeAcceleration("paused", true)
	  else
		 if Game.GetView() == "settings" then
			Game.SetTimeAcceleration("1x", true)
			Game.SetView(player:IsDead() and "death" or "world")
		 else
			Game.SetView("settings")
		 end
	  end
   end
end

local function show_ships_on_screen()
   local bodies = Space.GetBodies(function (body) return body:IsShip() and player:DistanceTo(body) < 100000000 end)
   for _,body in pairs(bodies) do
	  local pos = body:GetProjectedScreenPosition()
	  local size = 8
	  if pos then
		 local iconsize = show_icon(pos, get_body_icon(body), colors.lightgreen, 48, anchor.center, anchor.center)
		 show_text(pos + Vector(iconsize.x/2), body.label, colors.lightgreen, pionillium.small, anchor.left, anchor.center)
		 local mp = pigui.GetMousePos()
		 if (Vector(mp.x,mp.y) - Vector(pos)):magnitude() < size and pigui.IsMouseReleased(0) then
			player:SetCombatTarget(body)
		 end
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
   local nav_target_in_this_group = false
   local navtarget = player:GetNavTarget() and player:GetNavTarget():GetSystemBody() or nil
   for pos,group in pairs(body_groups) do
	  nav_target_in_this_group = false
	  local mp = pigui.GetMousePos()
	  labels = {}
	  for p,body in pairs(group) do
		 table.insert(labels, body)
		 local sb = body:GetSystemBody()
		 if navtarget and navtarget.index == sb.index then
			nav_target_in_this_group = true
		 end
	  end
	  table.sort(labels, function (a,b) return a.label < b.label end)
	  do
		 local best_body, count = get_body_group_max_body(group)
		 if nav_target_in_this_group then
			best_body = player:GetNavTarget()
		 end
         local textsize = show_icon(pos, get_body_icon(best_body), colors.lightgrey, 24, anchor.center, anchor.center)
		 show_text(pos + Vector(textsize.x * 1.1), best_body.label .. (count > 1 and " +" or ""), colors.lightgrey, pionillium.small, anchor.left, anchor.center)
	  end
	  local label = table.concat(map(function (a) return a.label end, labels), "\n")
	  local show_tooltip = false
	  if (Vector(mp.x - pos.x,mp.y - pos.y)):magnitude() < cutoff then
		 if pigui.IsMouseClicked(1) and (#labels == 1 or nav_target_in_this_group) then
			pigui.OpenPopup("##piepopup")
			if #labels == 1 then
			   radial_nav_target = labels[1]
			else
			   radial_nav_target = player:GetNavTarget()
			end
			should_show_radial_menu = true
		 end
		 if pigui.IsMouseReleased(0) then
			if #labels == 1 then
			   if player:GetNavTarget() == labels[1] then
				  player:SetNavTarget(nil)
			   else
				  player:SetNavTarget(labels[1])
			   end
			else
			   pigui.OpenPopup("navtarget" .. label)
			end
		 else
			show_tooltip = true
		 end
	  end
	  if should_show_radial_menu then
		 show_radial_menu()
	  end

	  if pigui.BeginPopup("navtarget" .. label) then
		 for _,body in pairs(labels) do
			icon(get_body_icon(body), 16, colors.lightgrey)
			pigui.SameLine()
			if pigui.Selectable(body.label, navtarget and body:GetSystemBody().index == navtarget.index or false, {}) then
			   if player:GetNavTarget() == body then
				  player:SetNavTarget(nil)
			   else
				  player:SetNavTarget(body)
			   end
			end
		 end
		 pigui.EndPopup()
	  else
		 if show_tooltip and not should_show_radial_menu then
			pigui.SetTooltip(label)
		 end
	  end
   end
end


local function show_marker_old(name, painter, color, show_in_reticule, direction, size, tooltip)
   local siz = size and size or 12
   local pos,dir,point,side = markerPos(name, reticule_radius - 10)
   local mp = pigui.GetMousePos()
   if pos and show_in_reticule then
	  local thesize = siz / 3
	  painter(pos, thesize, color, 1.0, direction)
	  if tooltip and tooltip ~= "" and (Vector(mp.x,mp.y) - pos):magnitude() <= thesize and not pigui.IsMouseHoveringAnyWindow() then
		 pigui.SetTooltip(tooltip)
	  end
   end
   if side == "onscreen" and point then
	  local thesize = siz
	  painter(point, thesize, color, 3.0, direction)
	  if tooltip and tooltip ~= "" and (Vector(mp.x,mp.y) - point):magnitude() <= thesize and not pigui.IsMouseHoveringAnyWindow() then
		 pigui.SetTooltip(tooltip)
	  end
   end
end

local function show_marker(name, icon, color, show_in_reticule, direction, size, tooltip)
   local siz = size or 24
   local pos,dir,point,side = markerPos(name, reticule_radius - 10)
   local mp = pigui.GetMousePos()
   if pos and show_in_reticule then
	  local thesize = siz / 2
	  show_icon(pos, icon, color, thesize, anchor.center, anchor.center)
	  if tooltip and tooltip ~= "" and (Vector(mp.x,mp.y) - pos):magnitude() <= thesize and not pigui.IsMouseHoveringAnyWindow() then
		 pigui.SetTooltip(tooltip)
	  end
   end
   if side == "onscreen" and point then
	  local thesize = siz
	  show_icon(point, icon, color, thesize, anchor.center, anchor.center)
	  if tooltip and tooltip ~= "" and (Vector(mp.x,mp.y) - point):magnitude() <= thesize and not pigui.IsMouseHoveringAnyWindow() then
		 pigui.SetTooltip(tooltip)
	  end
   end
end

local planeType = "system-wide"


local function show_ship_functions()
   window("Internal ship functions", { "NoTitleBar" }, function ()
			 do -- rotation damping
				local rd = player:GetRotationDamping()
				if image_button(rd and icons.rotation_damping_on or icons.rotation_damping_off, 24, colors.darkgrey, colors.lightgrey, "Toggle rotation damping") then
				   player:ToggleRotationDamping()
				end
			 end
			 do -- wheelstate
				if not player:IsDocked() and not player:IsLanded() then
				   local wheelstate = player:GetWheelState() -- 0.0 is up, 1.0 is down
				   local icon = wheelstate == 0.0 and icons.landing_gear_down or (wheelstate == 1.0 and icons.landing_gear_up or nil)
				   if icon then
					  if image_button(icon, 24, colors.darkgrey, colors.lightgrey, "Toggle landing gear") then
						 player:ToggleWheelState()
					  end
				   end
				   pigui.Text("Wheelstate: " .. wheelstate)
				end
			 end
   end)
end

local function show_stuff()
   window("Stuff",{}, function()
			 do -- take off
				pigui.Text(player:GetFlightState())
				local takeoff=false
				if player:IsLanded() then
				   takeoff=true
				   if pigui.Button("Blastoff") then
					  player:Blastoff()
				   end
				elseif player:IsDocked() then
				   takeoff=true
				   if pigui.Button("Undock") then
					  player:Undock()
				   end
				end
				if takeoff then
				   if pigui.Button("Take Off") then
					  player:TakeOff()
				   end
				end
			 end
			 do -- pitch/heading
				if pigui.Button("Plane type: " .. planeType) then
				   if planeType == "system-wide" then
					  planeType = "planet"
					  Game.AddCommsLogLine("Switched to planet", nil)
				   else
					  planeType = "system-wide"
					  Game.AddCommsLogLine("Switched to system-wide", nil)
				   end
				end
				-- local heading, pitch = player:GetHeadingPitch(planeType)
				-- pigui.Text("Heading: " .. math.ceil(heading / two_pi * 360) .. "°, Pitch: " .. math.ceil(pitch / two_pi * 360) .. "°")
			 end
			 do -- flight control state
				local state = player:GetFlightControlState()
				pigui.Text(state)
				local states = { "manual", "fix-speed", "fix-heading-prograde", "fix-heading-retrograde", "fix-heading-normal", "fix-heading-antinormal", "fix-heading-radial-in", "fix-heading-radial-out", "fix-heading-kill-rot" }
				if state == "fix-speed" then
				   pigui.Text(" speed: " .. player:GetFlightControlSpeed())
				end
				for k,v in pairs(states) do
				   if state ~= v then
					  if pigui.Button(v) then
						 player:SetFlightControlState(v)
					  end
				   end
				end
			 end
   end)
end

local function show_hyperspace_countdown()
   show_text(Vector(pigui.screen_width/2, pigui.screen_height/3), string.interp(lc.HYPERSPACING_IN_N_SECONDS, { seconds = math.ceil(player:GetHyperspaceCountdown()) }), colors.green, pionillium.large, anchor.center, anchor.center)
end

local function draw_missile(position, typ, amount, missile_object)
   local width = 60
   local height = 16
   local lower_right = position + Vector(width, height)
   local name = lec[string.upper(typ)]
   local short = name:sub(0, name:find(" "))
   pigui.AddRectFilled(position, lower_right, colors.white, 0.0, 0)
   show_text(position + Vector(10, height / 2), short, colors.black, pionillium.small, anchor.left, anchor.center)
   show_text(position + Vector(1, height / 2), amount, colors.darkergrey, pionillium.small, anchor.left, anchor.center)
   local stripes = 15
   local function stripe(start, stop, count)
	  local w = width / count
	  pigui.AddRectFilled(position + Vector(start * w + 1, 1), position + Vector(stop * w - 1, height - 1), colors.black, 0.0, 0)
   end
   if typ == "missile_naval" then
	  stripe(12,14,stripes)
	  stripe(14,15,stripes)
   elseif typ == "missile_unguided" then
	  stripe(14,15,stripes)
   elseif typ == "missile_smart" then
	  stripe(13,15,stripes)
   elseif typ == "missile_guided" then
	  stripe(13,14,stripes)
	  stripe(14,15,stripes)
   else
	  error("unknown missile type " .. typ)
   end
   if pigui.IsMouseHoveringRect(position, lower_right, true) then
	  if pigui.IsMouseReleased(0) and Game.player:GetCombatTarget() then
		 Game.player:FireMissileAt(missile_object, Game.player:GetCombatTarget())
	  end
	  pigui.SetTooltip(string.interp(lui.HUD_TOOLTIP_FIRE_MISSILE, { missile = name}))
   end
end

local function show_weapons()
   local position = Vector(20, pigui.screen_height / 2)
   -- missiles
   local equipped = player:GetEquip("missile")
   local missiles = {}
   local idx = {} -- arbitrary index containing that missile type
   for k,v in pairs(equipped) do
	  assert(v.missile_type)
	  missiles[v.missile_type] = (missiles[v.missile_type] or 0) + 1
	  idx[v.missile_type] = v
   end
   local i = 0
   for k,v in pairs(missiles) do
	  local pos = position + Vector(0, i * 20)
	  draw_missile(pos, k, v, idx[k])
	  i = i+1
   end
   -- gun stats, left side
   local pos = position + Vector(25, i * 20 + 50)
   show_circular_gauge(pos, player:GetGunTemperature(0), colors.gun_tmp_gauge, lui.HUD_GUN, lui.HUD_FRONT)
   show_circular_gauge(pos + Vector(0, 80), player:GetGunTemperature(1), colors.gun_tmp_gauge, lui.HUD_GUN, lui.HUD_REAR)

end

local function show_radar_mapper()
   -- TODO: check whether the relevant equipment is actually on ship
   local t = player:GetCombatTarget()
   if t and t:IsShip() then
	  window("Radar mapper", {}, function()
				local shipdef = ShipDef[t.shipId]
				pigui.Text("Ship type: " .. shipdef.name)
				pigui.Text("Label: " .. t.label)
				pigui.Text("Hyperdrive class: " .. shipdef.hyperdriveClass)
				pigui.Text("Mass: " .. t:GetStats().static_mass)
				pigui.Text("Cargo: " .. t:GetStats().used_cargo)
				local position = Vector(pigui.GetWindowPos()) + Vector(50, 150)
				show_circular_gauge(position + Vector(0, 0), 1 - t:GetHullPercent() / 100, colors.hull_gauge, lui.HUD_DAMAGE, lui.HUD_HULL)
				if t:GetShieldsPercent() then
				   show_circular_gauge(position + Vector(100, 0), t:GetShieldsPercent() / 100, colors.shield_gauge, lui.HUD_SHIELDS)
				end
	  end)
   end
end

local function show_hyperspace_analyzer()
   -- TODO: check whether the relevant equipment is actually on ship
   local t = player:GetNavTarget()
   if t and t:IsHyperspaceCloud() then
	  -- departure, mass, destination, due date
	  window("Hyperspace analyzer", {}, function()
				local cloud = t:GetHyperspaceCloudInfo()
				pigui.Text("Arrival: " .. (cloud.is_arrival and "yes" or "no"));
				pigui.Text("Ship Mass: " .. cloud.ship:GetStats().static_mass)
				pigui.Text("Destination: " .. cloud.destination:GetStarSystem().name)
				pigui.Text("Due Date: " .. Format.Date(cloud.due_date))
	  end)
   end

end
local function ColoredSelectedButton(text, is_selected, color, selected_color, tooltip)
   if is_selected then
	  pigui.PushStyleColor("Button", selected_color)
	  pigui.PushStyleColor("ButtonHovered", selected_color:tint(0.3))
   else
	  pigui.PushStyleColor("Button", color)
	  pigui.PushStyleColor("ButtonHovered", color:tint(0.3))
   end
   local res = pigui.Button(text)
   pigui.PopStyleColor(2)
   if pigui.IsItemHovered() then
	  pigui.SetTooltip(tooltip)
   end
   return res
end

local function ColoredSelectedIconButton(icon, is_selected, color, selected_color, tooltip)
   if is_selected then
	  pigui.PushStyleColor("Button", selected_color)
	  pigui.PushStyleColor("ButtonHovered", selected_color:tint(0.3))
   else
	  pigui.PushStyleColor("Button", color)
	  pigui.PushStyleColor("ButtonHovered", color:tint(0.3))
   end
   local uv0,uv1 = get_icon_tex(icon)
   pigui.PushID(tooltip)
   local res = pigui.ImageButton(pigui.icons_id, Vector(32,32), uv0, uv1, -1, colors.transparent, colors.lightgrey)
   pigui.PopID()
   pigui.PopStyleColor(2)
   if pigui.IsItemHovered() then
	  pigui.SetTooltip(tooltip)
   end
   return res
end

local function show_modes()
   local mode = Game.GetView()
   window("Modes", {"NoTitleBar", "NoResize"}, function()
			 --   pigui.SetNextWindowSize(Vector(0.0, 0.0), "Always")
			 local view_icon = icons["view_" .. Game.GetWorldCamType()]

			 if ColoredSelectedIconButton(view_icon, mode == "world", colors.mode_button, colors.mode_button_selected, "World View") then
				select_world_view()
			 end
			 pigui.SameLine()
			 if ColoredSelectedIconButton(icons.sector_map, mode == "sector", colors.mode_button, colors.mode_button_selected, "Map View") then
				Game.SetView("sector")
			 end
			 pigui.SameLine()
			 if ColoredSelectedIconButton(icons.system_map, mode == "system", colors.mode_button, colors.mode_button_selected, "System View") then
				Game.SetView("system")
			 end
			 pigui.SameLine()
			 if ColoredSelectedIconButton(icons.system_overview, mode == "system_info", colors.mode_button, colors.mode_button_selected, "System Info View") then
				Game.SetView("system_info")
			 end
			 pigui.SameLine()
			 if ColoredSelectedIconButton(icons.galaxy_map, mode == "galaxy", colors.mode_button, colors.mode_button_selected, "Galaxy View") then
				Game.SetView("galaxy")
			 end
			 pigui.SameLine()
			 if ColoredSelectedIconButton(icons.personal_info, mode == "info", colors.mode_button, colors.mode_button_selected, "Ship Status View") then
				Game.SetView("info")
			 end
			 if player:IsDocked() then
				pigui.SameLine()
				if ColoredSelectedIconButton(icons.comms, mode == "space_station", colors.mode_button, colors.mode_button_selected, "Comms View") then
				   Game.SetView("space_station")
				end
			 end
   end)
end

local function show_hyperspace_button()
   local disabled = false
   local shown = true
   local illegal = player:IsHyperjumpAllowed()
   local abort = false
   local targetpath = player:GetHyperspaceTarget()
   local target = targetpath and targetpath:GetStarSystem()
   if player:CanHyperjumpTo(targetpath) then
	  if player:IsDocked() or player:IsLanded() then
		 disabled = true
	  elseif player:IsHyperspaceActive() then
		 abort = true
	  end
   else
	  shown = false
   end
   if shown then
	  window("Hyperspace Button", {"NoTitleBar","NoResize"}, function ()
				local position = Vector(pigui.GetWindowPos()) + Vector(0, 0)
				local size = 24
				if disabled then
				   local uv0, uv1 = get_icon_tex(icons.hyperspace)
				   pigui.Image(pigui.icons_id, Vector(size, size), uv0, uv1, colors.darkgrey, colors.lightgrey)
				else
				   if image_button(illegal and icons.hyperspace or icons.hyperspace_off, size, colors.transparent, colors.lightgrey, "Hyperjump") then
					  if player:IsHyperspaceActive() then
						 player:AbortHyperjump()
					  else
						 player:HyperjumpTo(player:GetHyperspaceTarget())
					  end
				   end
				end
				pigui.SameLine()
				group(function ()
					  -- local size = show_icon(position, illegal and icons.hyperspace or icons.hyperspace_off, colors.lightgrey, 24, anchor.left, anchor.top)
					  -- show_text(position + Vector(size,0), target.name or "unknown", colors.lightgrey, pionillium.small, anchor.left, anchor.top, "Hyperspace target")
					  pigui.Text(target.name or "unknown")
					  local engine = player:GetEquip("engine", 1)
					  if not engine then
						 return
					  end
					  local distance, fuel, duration = engine:CheckJump(player, Game.system.path, targetpath)
					  local text = "" .. string.format("(%d,%d,%d)", targetpath.sectorX, targetpath.sectorY, targetpath.sectorZ) .. " " .. string.format("%.2f", distance) .. "ly " .. fuel .. "t " .. MyFormat.Duration(duration, 2)
					  -- show_text(position + Vector(size,textsize.y), text, colors.lightgrey, pionillium.small, anchor.left, anchor.top, "Stuff")
					  pigui.Text(text)
					  -- "(1,1,1) 4.34ly 3t 32hrs"
				end)
	  end)
   end
end

local function show_frame_markers(frame)
   local position = point_on_circle_radius(center, reticule_text_radius, -1.35)
   show_text(position, "Frame", colors.darkgrey, pionillium.small, anchor.right, anchor.bottom)
   position = point_on_circle_radius(center, reticule_text_radius, -2)
   show_text(position, frame.label, colors.lightgrey, pionillium.medium, anchor.right, anchor.bottom)

   local speed = Vector(pigui.GetVelocity("frame_prograde"))
   local spd,unit = MyFormat.Speed(speed:magnitude())
   position = point_on_circle_radius(center, reticule_text_radius, -2.9)
   show_text_fancy(position, { spd, unit }, { colors.lightgrey, colors.darkgrey }, { pionillium.large, pionillium.medium }, anchor.right, anchor.bottom)

   local distance = player:DistanceTo(frame)
   local dist,unit = MyFormat.Distance(distance)
   position = point_on_circle_radius(center, reticule_text_radius, -3.1)
   local textsize = show_text_fancy(position, { dist, unit }, { colors.lightgrey, colors.darkgrey }, { pionillium.large, pionillium.medium }, anchor.right, anchor.top )

   local brakeDist = player:GetDistanceToZeroV("frame", "retrograde")
   position.y = position.y + textsize.y * 1.1
   show_text(position, "~" .. Format.Distance(brakeDist), colors.darkgrey, pionillium.medium, anchor.right, anchor.top)

   -- ******************** Frame markers ********************
   do
	  local size = 24
	  local smallsize = 24
	  show_marker("frame_prograde", icons.prograde, colors.orbital_marker, true, nil, size) -- , "Prograde direction relative to frame"
	  show_marker("frame_retrograde", icons.retrograde, colors.orbital_marker, show_retrograde_indicators, nil, size) -- , "Retrograde direction relative to frame"
	  show_marker("normal", icons.normal, colors.orbital_marker, false, nil, smallsize)
	  show_marker("anti_normal", icons.antinormal, colors.orbital_marker, false, nil, smallsize)
	  show_marker("radial_out", icons.radial_out, colors.orbital_marker, false, nil, smallsize)
	  show_marker("radial_in", icons.radial_in, colors.orbital_marker, false, nil, smallsize)
	  show_marker("away_from_frame", icons.frame_away, colors.orbital_marker, true, nil, size) -- , "Direction away from frame"
   end
   local pos,dir = markerPos("frame", reticule_radius + 10)
   if pos then
	  local size = 16
	  show_icon(pos, icons.direction_frame_hollow, colors.lightgrey, size, anchor.center, anchor.center, "", dir:angle())
   end
   -- ******************** Combat target ********************
   if player:GetCombatTarget() then
	  show_marker("combat_target", icons.current_height, colors.combat_target, true)
	  show_marker("combat_target_lead", icons.bullseye, colors.combat_target, false)
	  do -- line between lead and target TODO: dashed
		 local c_pos,c_dir,c_point,c_side = markerPos("combat_target", reticule_radius - 10)
		 local cl_pos,cl_dir,cl_point,cl_side = markerPos("combat_target_lead", reticule_radius - 10)
		 if c_point and cl_point and (c_side ~= "hidden" or cl_side ~= "hidden") then
			pigui.AddLine(c_point, cl_point, colors.combat_target, 1.0)
		 end
	  end
   end
end

local function show_hud(delta)
   center = Vector(pigui.screen_width/2, pigui.screen_height/2)
   navball_center = Vector(center.x, pigui.screen_height - 25 - navball_radius)
   local windowbg = colors.noz_darkblue
   -- transparent full-size window, no inputs
   pigui.SetNextWindowPos(Vector(0, 0), "Always")
   pigui.SetNextWindowSize(Vector(pigui.screen_width, pigui.screen_height), "Always")
   withStyleColors({ ["WindowBg"] = colors.transparent }, function()
		 window("HUD", {"NoTitleBar","NoInputs","NoMove","NoResize","NoSavedSettings","NoFocusOnAppearing","NoBringToFrontOnFocus"}, function()

				   --	 pigui.Image(pigui.icons_id, Vector(512,512), Vector(0.0,0.0), Vector(1.0,1.0), colors.red);

				   -- ******************** Ship Directional Markers ********************
				   local size=24
				   local side, dir, pos = pigui.GetHUDMarker("forward")
				   local dir_fwd = Vector(dir.x, dir.y)
				   local show_forward_direction_in_reticule = true
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  show_icon(pos, icons.forward, colors.lightgrey, size, anchor.center, anchor.center)
				   end
				   local side, dir, pos = pigui.GetHUDMarker("backward")
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  show_icon(pos, icons.backward, colors.lightgrey, size, anchor.center, anchor.center)
				   end
				   local side, dir, pos = pigui.GetHUDMarker("left")
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  show_icon(pos, icons.left, colors.lightgrey, size, anchor.center, anchor.center)
				   end
				   local side, dir, pos = pigui.GetHUDMarker("right")
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  show_icon(pos, icons.right, colors.lightgrey, size, anchor.center, anchor.center)
				   end
				   local side, dir, pos = pigui.GetHUDMarker("up")
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  local angle = dir_fwd:angle() - math.pi -- icon is pointing the wrong way
					  show_icon(pos, icons.up, colors.lightgrey, size, anchor.center, anchor.center, "", angle)
				   end
				   local side, dir, pos = pigui.GetHUDMarker("down")
				   if side == "onscreen" then
					  if Vector(pos.x - center.x, pos.y - center.y):magnitude() < reticule_radius then
						 show_forward_direction_in_reticule = false
					  end
					  local angle = dir_fwd:angle()
					  show_icon(pos, icons.down, colors.lightgrey, size, anchor.center, anchor.center, "", angle)
				   end
				   -- ******************** Reticule ********************
				   if show_forward_direction_in_reticule then
					  local size = 16
					  local angle = dir_fwd:angle()
					  show_icon(center, icons.direction_forward, colors.lightgrey, size, anchor.center, anchor.center, "", angle)
				   end
				   -- navigation circle
				   local segments = circle_segments(reticule_radius)
				   pigui.AddCircle(center, reticule_radius, colors.lightgrey, segments, 2.0)
				   -- ******************** Nav Target speed / distance ********************
				   local navTarget = player:GetNavTarget()
				   if navTarget then
					  -- target name
					  local position = point_on_circle_radius(center, reticule_text_radius, 1.35)
					  show_text(position, "Nav Target", colors.darkgreen, pionillium.small, anchor.left, anchor.bottom)
					  position = point_on_circle_radius(center, reticule_text_radius, 2)
					  show_text(position, navTarget.label, colors.lightgreen, pionillium.medium, anchor.left, anchor.bottom) -- , "The current navigational target"

					  local speed = Vector(pigui.GetVelocity("nav_prograde"))
					  local spd,unit = MyFormat.Speed(speed:magnitude())
					  position = point_on_circle_radius(center, reticule_text_radius, 2.9)
					  local textsize = show_text_fancy(position, { spd, unit }, { colors.lightgreen, colors.darkgreen }, { pionillium.large, pionillium.medium }, anchor.left, anchor.bottom)
					  do
						 local pos = Vector(player:GetPositionRelTo(navTarget))
						 local vel = Vector(player:GetVelocityRelTo(navTarget))
						 local proj = pos:dot(vel) / pos:magnitude()
						 local spd,unit = MyFormat.Speed(proj)
						 show_text_fancy(position + Vector(textsize.x * 1.1, 0), { spd, unit }, { colors.lightgreen, colors.darkgreen }, { pionillium.medium, pionillium.small }, anchor.left, anchor.bottom)
					  end


					  local distance = player:DistanceTo(navTarget)
					  local dist,unit = MyFormat.Distance(distance)
					  position = point_on_circle_radius(center, reticule_text_radius, 3.1)
					  local textsize = show_text_fancy(position, { dist, unit }, { colors.lightgreen, colors.darkgreen }, { pionillium.large, pionillium.medium }, anchor.left, anchor.top )
					  local brakeDist = player:GetDistanceToZeroV("nav", "retrograde")
					  position.y = position.y + textsize.y * 1.1
					  show_text(position, "~" .. Format.Distance(brakeDist), colors.darkgreen, pionillium.medium, anchor.left, anchor.top) -- , "Time to brake with main thrusters"
				   end

				   -- ******************** Maneuver speed ********************
				   local spd = player:GetManeuverSpeed()
				   if spd then
					  local position = point_on_circle_radius(center, reticule_text_radius, 0)
					  local speed,unit = MyFormat.Speed(spd)
					  show_text_fancy(position, { speed, unit }, { colors.maneuver, colors.maneuver }, { pionillium.large, pionillium.medium }, anchor.center, anchor.bottom)
				   end
				   -- ******************** Combat Target speed / distance ********************
				   local combatTarget = player:GetCombatTarget()
				   if combatTarget then
					  -- target name

					  local position = point_on_circle_radius(center, reticule_text_radius, 6)
					  local textsize = show_text(position, combatTarget.label, colors.lightred, pionillium.medium, anchor.center, anchor.top) -- , "Combat target"

					  position.y = position.y + textsize.y * 1.1
					  local speed = combatTarget:GetVelocityRelTo(player)
					  local spd,unit = MyFormat.Speed(math.sqrt(speed.x*speed.x+speed.y*speed.y+speed.z*speed.z))
					  show_text_fancy(position + Vector(-5.0), { spd, unit }, { colors.lightred, colors.lightred }, { pionillium.large, pionillium.medium }, anchor.right, anchor.top)
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
					  show_frame_markers(frame)
				   end
				   -- ******************** Nav Target markers ********************
				   show_marker("nav_prograde", icons.prograde, colors.lightgreen, true)
				   show_marker("nav_retrograde", icons.retrograde, colors.lightgreen, show_retrograde_indicators)
				   show_marker("nav", icons.square, colors.lightgreen, false)
				   local pos,dir,point,side = markerPos("nav", reticule_radius + 15)
				   if pos then
					  local size = 16
					  show_icon(pos, icons.direction, colors.lightgreen, size, anchor.center, anchor.center, "", dir:angle())
				   end
				   -- ******************** Maneuver Node ********************
				   show_marker("maneuver", icons.bullseye, colors.maneuver, true)

				   show_bodies_on_screen()

				   show_navball()
				   show_thrust()
				   show_weapons()
				   show_ships_on_screen()

				   if player:IsHyperspaceActive() then
					  show_hyperspace_countdown()
				   end
		 end)
   end)

   withStyleColors({ ["WindowBg"] = colors.noz_darkblue }, function()
		 show_nav_window()
		 show_stuff()
		 show_comm_log()
		 show_radar_mapper()
		 show_hyperspace_analyzer()
		 show_hyperspace_button()
		 show_ship_functions()
		 --	 show_settings()

		 -- show_debug_orbit()
		 --	show_debug_thrust()
		 -- show_debug_temp()
		 -- show_debug_gravity()

		 -- Missions, these should *not* be part of the regular HUD
		 --	show_missions()
   end)
end

local function show_pause_screen()
	 if Game.GetView() == "world" then
			pigui.SetNextWindowPos(Vector(0, 0), "Always")
			pigui.SetNextWindowSize(Vector(pigui.screen_width, pigui.screen_height), "Always")
			withStyleColors({ ["WindowBg"] = colors.paused_background }, function()
						window("Pause", {"NoTitleBar","NoMove","NoResize","NoSavedSettings"}, function()
											local center = Vector(pigui.screen_width / 2, pigui.screen_height / 4)
											show_text(center, lc.PAUSED, colors.paused_text, pionillium.large, anchor.center, anchor.center)
						end)
			end)
	 end
end

local function show_hyperspace()
	 withStyleColors({ ["WindowBg"] = colors.transparent }, function()
	 window("HUD", {"NoTitleBar","NoInputs","NoMove","NoResize","NoSavedSettings","NoFocusOnAppearing","NoBringToFrontOnFocus"}, function()
						 show_text(Vector(pigui.screen_width / 2, pigui.screen_height / 2), lc.HYPERSPACE_IN_HYPERSPACE, colors.red, pionillium.large, anchor.center, anchor.center)
						 local systempath_target = player:GetHyperspaceTarget()
						 local starsystem = systempath_target:GetStarSystem()
						 local sector = starsystem.sector
						 show_text(Vector(pigui.screen_width / 2, pigui.screen_height / 3 * 2), string.interp(lc.IN_TRANSIT_TO_N_X_Y_Z, { system = starsystem.name, x = sector.x, y = sector.y, z = sector.z }), colors.green, pionillium.large, anchor.center, anchor.center)
						 show_text(Vector(pigui.screen_width / 2, pigui.screen_height / 3 * 2.3), string.interp(lc.JUMP_COMPLETE, { percent = math.ceil(100 * Game.GetHyperspaceTravelledPercentage())}), colors.green, pionillium.large, anchor.center, anchor.center)
	 end)
	 end)
end

local function show_time_accel_buttons()
	 center = Vector(pigui.screen_width/2, pigui.screen_height/2)
	 navball_center = Vector(center.x, pigui.screen_height - 25 - navball_radius)

	 -- the following doesn't work, seems like a bug in imgui? just call SetWindowFocus after showing the window
	 -- if Game.GetTimeAcceleration == "paused" then
	 -- 		pigui.SetNextWindowFocus() -- bring time accel buttons above pause window
	 -- end
	 local requested = Game.GetRequestedTimeAcceleration()
	 local current = Game.GetTimeAcceleration()
	 pigui.SetNextWindowPos(Vector(0, pigui.screen_height - 50), "Always")
	 withStyleColors({ ["WindowBg"] = colors.noz_darkblue }, function()
				 window("TimeAccel", {"NoTitleBar", "NoMove","NoResize","NoSavedSettings"}, function()
									 local size = 32
									 for k,v in pairs({"paused", "1x", "10x", "100x", "1000x", "10000x"}) do
											local color = (current == v and colors.time_accel_current or (requested == v and colors.time_accel_requested or colors.time_accel))
											withStyleColors({ ["Button"] = color }, function()
														if image_button(icons["time_accel_" .. v], size, colors.transparent, colors.lightgrey, v) then
															 Game.SetTimeAcceleration(v, pigui.key_ctrl)
														end
											end)
											pigui.SameLine()
									 end
				 end)
				 if Game.GetTimeAcceleration() == "paused" then
						pigui.SetWindowFocus("TimeAccel")
				 end
	 end)
end

pigui.handlers.system_info = function(delta)
	 show_modes()
end

pigui.handlers.system = function(delta)
	 show_modes()
end

pigui.handlers.galaxy = function(delta)
	 show_modes()
end

pigui.handlers.space_station = function(delta)
	 show_modes()
end

pigui.handlers.info = function(delta)
	 show_modes()
end

pigui.handlers.world = function(delta)
	 if Game.InHyperspace() then
			show_hyperspace()
	 else
			show_hud(delta)
	 end

	 show_modes()
end

local sector_search_string = ""
local sector_search_result = nil
pigui.handlers.sector = function(delta)
	 window(lc.SEARCH, {}, function()
						 local search, return_pressed = pigui.InputText("", sector_search_string, { "EnterReturnsTrue" })
						 sector_search_string = search
						 pigui.SameLine()
						 if pigui.Button(lc.SEARCH) or return_pressed then
								sector_search_result = Game.SectorViewSearch(sector_search_string)
								sector_search_string = ""
						 end
						 if sector_search_result then
								pigui.Text(sector_search_result)
						 end
	 end)

	 show_modes()
end

pigui.handlers.HUD = function(delta)
	 player = Game.player
	 system = Game.system
	 pigui.PushStyleVar("WindowRounding", 0)
	 local view = Game.GetView()
	 local handler = pigui.handlers[view]
	 if handler then
			handler(delta)
	 end
	 handle_global_keys()

	 if Game.GetTimeAcceleration() == "paused" then
			show_pause_screen()
	 end
	 show_time_accel_buttons()
	 pigui.PopStyleVar(1)
end
