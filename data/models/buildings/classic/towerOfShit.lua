--[[
define_model('towerOfShit', {
	info = {
		lod_pixels = {10, 50},
		bounding_radius=100,
		materials={'mat1'},
		tags = {'city_building'},
	},
	static = function(lod)
		set_material("mat1", 1,1,1,1)
		use_material("mat1")
		if lod == 1 then
			xref_cylinder(5, v(20,0,0), v(20,210,0), v(0,0,1), 20)
		else
			for i = 0,20 do
				local _xoff = 10*noise(v(0,i*10,0))
				local _zoff = 10*noise(v(0,0,i*10))
				local _start = v(30+_xoff,i*10,_zoff)
				local _end = v(30+_xoff,10+i*10,_zoff)
				xref_cylinder(16, _start, _end, v(1,0,0), 10.0+math.abs(10*noise(0,0.1*i,0)))
			end
		end
	end
})
--]]
