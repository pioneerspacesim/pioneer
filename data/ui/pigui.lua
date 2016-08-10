local Engine = import('Engine')
local pigui = Engine.pigui

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
			return Vector(self / math.abs(self.magnitude()))
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

local radius = 80
local function markerPos(name, distance)
	 local center = Vector(pigui.screen_width/2, pigui.screen_height/2)
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

local mission_selected

function show_missions()
	 local windowbg = {r=0,g=0,b=50,a=200}
	 local SpaceStation = import("SpaceStation")
	 local Game = import('Game')
	 local Space = import('Space')
	 local Format = import('Format')
	 local station = Game.player:GetDockedWith()
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
						pigui.Text(Format.Distance(Space.GetBody(v.body.index):DistanceTo(Game.player)))
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
			local m = mission_selected
			pigui.Text(m.description)
			pigui.Text("Payout: " .. (m.payout and Format.Money(m.payout) or "-"))

			pigui.Text("System: " .. (m.system and m.system.name or "-"))
			pigui.Text("Body: " .. (m.body and m.body.name or "-"))

			if m.system then
				 if m.system.index == Game.system.index
						and m.system.sector.x == Game.system.sector.x
						and m.system.sector.y == Game.system.sector.y
						and m.system.sector.z == Game.system.sector.z
				 then
						pigui.Text("Distance: " .. Format.Distance(Space.GetBody(m.body.index):DistanceTo(Game.player)))
				 else
						pigui.Text("Jump Distance: " .. m.system:DistanceTo(Game.system) .. "ly")
				 end
			end
	 end
	 pigui.End()
	 pigui.PopStyleColor(1)
end

local selected

pigui.handlers.HUD = function(delta)
	 local center = Vector(pigui.screen_width/2, pigui.screen_height/2)
	 local windowbg = {r=0,g=0,b=50,a=200}
	 -- transparent full-size window, no inputs
	 pigui.SetNextWindowPos(Vector(0, 0), "Always")
	 pigui.SetNextWindowSize(Vector(pigui.screen_width, pigui.screen_height), "Always")
	 pigui.PushStyleColor("WindowBg", {r=0,g=0,b=0,a=0})
	 pigui.Begin("HUD", {"NoTitleBar","NoInputs","NoMove","NoResize","NoSavedSettings","NoFocusOnAppearing","NoBringToFrontOnFocus"})
	 -- reticule
	 pigui.AddCircle(center, radius, {r=200, g=200, b=200}, 128, 2.0)
	 pigui.AddLine(center - Vector(5,0), center + Vector(5,0), {r=200,g=200,b=200}, 4.0)
	 pigui.AddLine(center - Vector(0,5), center + Vector(0,5), {r=200,g=200,b=200}, 4.0)
	 -- various markers
	 local pos,dir,point,side = markerPos("prograde", radius - 10)
	 if pos then
			local size = 4
			local left = pos + Vector(-1,0) * size
			local right = pos + Vector(1,0) * size
			local top = pos + Vector(0,1) * size
			local bottom = pos + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, {r=200,g=200,b=200}, 1.0)
	 end
         if side == "onscreen" and point then
			local size = 12
			local left = point + Vector(-1,0) * size
			local right = point + Vector(1,0) * size
			local top = point + Vector(0,1) * size
			local bottom = point + Vector(0,-1) * size
			pigui.AddQuad(left, top, right, bottom, {r=200,g=200,b=200}, 3.0)
         end
         	 local pos,dir,point,side = markerPos("retrograde", radius - 10)
	 if pos then
			local size = 3
			local leftTop = pos + Vector(-1,1) * size
			local rightTop = pos + Vector(1,1) * size
			local leftBottom = pos + Vector(-1,-1) * size
			local rightBottom = pos + Vector(1,-1) * size
			pigui.AddLine(leftTop, rightBottom, {r=200,g=200,b=200}, 1.0)
                        pigui.AddLine(leftBottom, rightTop, {r=200,g=200,b=200}, 1.0)
	 end
         if side == "onscreen" and point then
			local size = 12
			local leftTop = point + Vector(-1,1) * size
			local rightTop = point + Vector(1,1) * size
			local leftBottom = point + Vector(-1,-1) * size
			local rightBottom = point + Vector(1,-1) * size
			pigui.AddLine(leftTop, rightBottom, {r=200,g=200,b=200}, 3.0)
                        pigui.AddLine(leftBottom, rightTop, {r=200,g=200,b=200}, 3.0)
         end

	 local pos,dir = markerPos("frame", radius + 5)
	 if pos then
			local left = dir:left() * 4 + pos
			local right = dir:right() * 4 + pos
			local top = dir * 8 + pos
			pigui.AddTriangle(left, right, top, {r=200,g=200,b=200}, 2.0)
	 end
	 local pos,dir = markerPos("nav_target", radius + 5)
	 if pos then
			local left = dir:left() * 7 + pos
			local right = dir:right() * 7 + pos
			local top = dir * 14 + pos
			pigui.AddTriangleFilled(left, right, top, {r=200,g=200,b=200})
	 end
	 pigui.End()
	 pigui.PopStyleColor(1);


	 -- nav window
	 pigui.SetNextWindowPos(Vector(0,0), "FirstUseEver")
	 pigui.SetNextWindowSize(Vector(200,800), "FirstUseEver")
	 pigui.PushStyleColor("WindowBg", windowbg)
	 pigui.Begin("Navigation", {})
	 pigui.Columns(2, "navcolumns", false)
	 local Game = import('Game')
	 local Format = import('Format')
	 local Space = import('Space')
	 local system = Game.system
	 local player = Game.player
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

	 -- Missions, these should *not* be part of the regular HUD
	 show_missions()
end
