--[[
define_model('building1', {
	info = {
			scale = 10,
			bounding_radius = 15,
			lod_pixels = {20, 120, 130},
			materials = {'wall', 'wins1', 'wins2', 'wins3', 'wins4', 'balcony'},
			tags = {'city_building'},
		},
	static = function(lod)
		set_material('wall', 1,1,1,1,.3,.3,.3,5)
		set_material('balcony', 0,0,.3,.9,.7,.8,1,50)
		if lod == 1 then
			local w = 1
			local h = 1.2
			texture('house_01.png')
			use_material('wall')
			extrusion(v(0,0,w), v(0,0,-w), v(0,1,0), 1.0,
				v(-w,0,0), v(w,0,0), v(w,h,0), v(-w,h,0))
		else
			texture('house_01.png')
			use_material('wall')
			load_obj('house_01.obj')
		end
		if lod > 2 then
			use_material('wins1')
			load_obj('house_01_wins1.obj')
			use_material('wins2')
			load_obj('house_01_wins2.obj')
			use_material('wins3')
			load_obj('house_01_wins3.obj')
			use_material('wins4')
			load_obj('house_01_wins4.obj')
			use_material('balcony')
			load_obj('house_01_balcony.obj')
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('wins1', 1,1,1,1,.9,.9,1.5,100,0,0,0)
			set_material('wins2', 1,1,1,1,.9,.9,1.5,100,0,0,0)
			set_material('wins3', 1,1,1,1,.9,.9,1.5,100,0,0,0)
			set_material('wins4', 1,1,1,1,.9,.9,1.5,100,0,0,0)
			local switch = math.fmod((get_time('HOURS')*0.2),1)
			if switch < .26 then
				set_material('wins1', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
			elseif switch < .51 then
				set_material('wins2', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
			elseif switch < .76 then
				set_material('wins3', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
			else
				set_material('wins4', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
			end
		end
	end
})
--]]
