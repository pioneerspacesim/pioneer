define_model('building1', {
	info = {
			scale = 10,
			bounding_radius = 30,
			materials = {'wall', 'wins1', 'wins2', 'wins3', 'wins4', 'balcony'},
			tags = {'city_building', 'city_power', 'city_starport_building'},
		},
	static = function(lod)
		set_material('wall', 1,1,1,1,.3,.3,.3,5)
		set_material('balcony', 0,0,.3,.9,.7,.8,1,50)
  		texture('house_01.png')
		use_material('wall')
		load_obj('house_01.obj')
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
	end,
	dynamic = function(lod)
        set_material('wins1', 1,1,1,1,.9,.9,1.5,100,0,0,0)
		set_material('wins2', 1,1,1,1,.9,.9,1.5,100,0,0,0)
		set_material('wins3', 1,1,1,1,.9,.9,1.5,100,0,0,0)
		set_material('wins4', 1,1,1,1,.9,.9,1.5,100,0,0,0)
        local switch = math.fmod((get_arg(3)*0.2),1)
		if switch < .26 then
        	set_material('wins1', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
		else
			if switch < .51 then
			    set_material('wins2', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
			else
				if switch < .76 then
			        set_material('wins3', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
				else
				    if switch > .75 then
				        set_material('wins4', 1,1,1,1,.9,.9,1.5,100,.5,.7,.9)
					end
				end
			end
		end
	end
})

