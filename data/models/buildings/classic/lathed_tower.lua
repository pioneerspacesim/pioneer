-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--[[
define_model('lathed_tower', {
	info = {
		lod_pixels = {5, 20, 100},
		bounding_radius = 120,
		materials = {'mat'},
		scale = 2,
		tags = {'city_building'},
	},
	static = function(lod)
		set_material("mat", .5,.7,.7,1)
		use_material("mat")
		if lod == 1 then
			cylinder(5, v(0,0,0), v(0,110,0), v(0,0,1), 8)
		end
		if lod == 2 then
			cylinder(8, v(0,0,0), v(0,110,0), v(0,0,1), 8)
		end
		if lod == 3 then
			for i = 0,10 do
				tapered_cylinder(8, v(0,i*10,0), v(0,(i+1)*10,0),
					v(0,0,1), math.abs(noise(5*i,0,0))*3+5,
					math.abs(noise(5*(i+1),0,0))*3+5)
			end
		end
		billboard('smoke.png', 8, v(1,1,1), { v(0, 111, 0) })
	end,
})
--]]
