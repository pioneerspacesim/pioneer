--[[
define_model('church', {
	info = {
			scale = 1,
			lod_pixels = { 5, 100, 200 },
			bounding_radius=60,
			materials={'wall', 'gold', 'wins', 'text', 'lamp'},
			tags = {'city_building'},
		},
	static = function(lod)
		set_material('wall', 1,1,1,1,.3,.3,.3,5)
		set_material('gold', 1, .9, .6, 1, 1, .9, .8, 50)
		if lod == 1 then
			local w = 10
			local h = 24
			local d = 15
			use_material('wall')
			texture('church.png')
			extrusion(v(0,0,d), v(0,0,-d), v(0,1,0), 1.0,
				v(-w,0,0), v(w,0,0), v(w,h,0), v(-w,h,0))
		end
		if lod > 1 then
			texture('church.png')
			use_material('wall')
			load_obj('church_wall.obj')
			call_model('woods_1', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		end
		if lod > 2 then
			use_material('gold')
			load_obj('church_gold.obj')
			use_material('wins')
			load_obj('church_wins.obj')
			texture(nil)
			use_material('lamp')
			load_obj('church_light.obj')

			call_model('old_clock', v(-2,22,4), v(0,0,1), v(0,1,0), .30)
			call_model('old_clock', v(2,22,4), v(0,0,-1), v(0,1,0), .30)
		end
	end,
	dynamic = function(lod)
		set_material('wins', 1,1,1,1,.9,.9,1.5,100)
		set_material('lamp', .7,.7,.7,1,.9,.9,1.5,100)
		local switch = math.fmod((get_time('HOURS')*0.1),1)
  		if switch < .51  then
			set_material('wins', 1,1,1,1,1,1,2,100,1.5,1.4,.5)
			set_material('lamp', .7,.7,.7,1,1,1,2,100,1.5,1.5,1.4)
		end
	end
})
--]]
