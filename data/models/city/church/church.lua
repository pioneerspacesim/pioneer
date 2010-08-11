define_model('church', {
	info = {
	        scale = 1,
			lod_pixels = { 50, 100, 200, 0 },
			bounding_radius=60,
			materials={'wall', 'gold', 'wins', 'text', 'lamp'},
			tags = {'city_building'},
		},
	static = function(lod)
		set_material('wall', 1,1,1,1,.3,.3,.3,5)
		set_material('gold', 1, .9, .6, 1, 1, .9, .8, 50)

		texture('church.png')
		use_material('wall')
		load_obj('church_wall.obj')
        use_material('gold')
		load_obj('church_gold.obj')
        use_material('wins')
		load_obj('church_wins.obj')
		texture(nil)
		use_material('lamp')
		load_obj('church_light.obj')
		
		call_model('old_clock', v(-2,22,4), v(0,0,1), v(0,1,0), .30)
		call_model('old_clock', v(2,22,4), v(0,0,-1), v(0,1,0), .30)
   		call_model('woods_1', v(0,0,0), v(1,0,0), v(0,1,0), 1)
	end,
	dynamic = function(lod)
        set_material('wins', 1,1,1,1,.9,.9,1.5,100)
		set_material('lamp', .7,.7,.7,1,.9,.9,1.5,100)
		local switch = math.fmod((get_arg(3)*0.1),1)
  		if switch < .51  then
			set_material('wins', 1,1,1,1,1,1,2,100,1.5,1.4,.5)
   			set_material('lamp', .7,.7,.7,1,1,1,2,100,1.5,1.5,1.4)
  	    end
	    	  
	  
	end
})
