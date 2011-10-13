--[[
define_model('church_new_clockhd', {
    info = {
			lod_pixels = {1,20,50,0},
			bounding_radius = 30,
   			},
	static = function(lod)
        load_obj('church_new_pyr.obj',Matrix.rotate(.5*math.pi,v(-1,0,0)))
	end
})

define_model('church_new_clock', {
    info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 5,
			materials = {'hour', 'min', 'glow'},
   			},
	static = function(lod)
        set_material('hour', .7,.1,.1,1,.3,.3,.3,30)
		set_material('min', .2,.5,.6,1,.3,.3,.3,30)
		set_material('glow', .5,.5,.5,1,1,1.5,2,100,.6,1.2,1.2)
		
		use_material('glow')
  		if lod > 2 then
			texture('glow.png')
		elseif lod > 1 then
		    texture('glow_s.png')
		end
		load_obj('church_new_win.obj',Matrix.translate(v(0,0,-.5)))
	end,
	dynamic = function(lod)
        local minutePos = -2*math.pi * get_time('HOURS')
		zbias(1,v(0,34,0),v(0,1,0))
		use_material('min')
		call_model('church_new_clockhd', v(0,35,0),v(math.cos(minutePos),0,math.sin(minutePos)),v(math.cos(minutePos+math.pi*0.5),0, math.sin(minutePos+math.pi*0.5)), 1.5)
		local hourPos = minutePos / 12
		zbias(2,v(0,34,0),v(0,1,0))
		use_material('hour')
		call_model('church_new_clockhd', v(0,35,0),v(math.cos(hourPos),0,math.sin(hourPos)),v(math.cos(hourPos+math.pi*0.5),0, math.sin(hourPos+math.pi*0.5)), 1.5)
	    zbias(0)
	end
})

define_model('church_new_0', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 30,
			materials={'default', 'glow', 'cutout'},

		},
	static = function(lod)
        set_material('default', .5,.5,.45,1,.5,.5,.6,10)
        set_material('cutout', .65,.6,.55,.9,.55,.5,.5,10)
        set_material('glow', .5,.5,.5,1,1,1.5,2,100,.6,1.2,1.2)

        if lod > 2 then
			texture('concrete.png')
            use_material('default')
		elseif lod > 1 then
	    	texture('concrete_s.png')
	    	use_material('default')
		end
        load_obj('church_new_0.obj',Matrix.translate(v(0,0,-.5)))

		if lod > 2 then
   			texture('door.png',v(.5,.13,0),v(.445,0,0),v(0,.52,0))
   		else
       		texture('door_s.png',v(.5,.13,0),v(.445,0,0),v(0,.52,0))
		end
		    zbias(1,v(0,1.2,16.897),v(0,0,1))
			circle(6,v(0,1.2,16.897),v(0,0,1),v(1,0,0),1)
			zbias(0)

		if lod > 1 then
			call_model('church_new_clock',v(0,0,0),v(1,0,0),v(0,1,0),1)
			

	        
			if lod > 3 then
            	texture('church_new_clock.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			elseif lod > 2 then
			    texture('church_new_clock_m.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			else
			    texture('church_new_clock_s.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			end
			use_material('cutout')
			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(-8,-7,-8)))
	        use_material('cutout')
			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(8.01,7.01,8.01)))
			
			
			
			
			
			
			
        end
	end,
	dynamic = function(lod)

	end
})

define_model('church_new_1', {
	info = {
			lod_pixels = {.1,20,100,0},
			bounding_radius = 30,
			materials={'default', 'glow', 'cutout'},
            --tags = {'city_building'},
		},
	static = function(lod)
        set_material('default', .5,.5,.45,1,.5,.5,.6,10)
        set_material('cutout', .65,.6,.55,.9,.55,.5,.5,10)
        set_material('glow', .5,.5,.5,1,1,1.5,2,100,.6,1.2,1.2)

        if lod > 2 then
			texture('concrete.png')
            use_material('default')
		elseif lod > 1 then
	    	texture('concrete_s.png')
	    	use_material('default')
		end
        load_obj('church_new_1.obj',Matrix.translate(v(0,0,-.5)))
        
        if lod > 2 then
   			texture('door.png',v(.5,.13,0),v(.445,0,0),v(0,.52,0))
   		else
       		texture('door_s.png',v(.5,.13,0),v(.445,0,0),v(0,.52,0))
		end
		zbias(1,v(0,1.2,16.897),v(0,0,1))
			circle(6,v(0,1.2,16.897),v(0,0,1),v(1,0,0),1)
		zbias(0)

		if lod > 1 then
			call_model('church_new_clock',v(0,0,0),v(1,0,0),v(0,1,0),1)

			if lod > 2 then
				texture('glow.png')
			else
			    texture('glow_s.png')
			end
	    	use_material('glow')
	        load_obj('church_new_win.obj',Matrix.translate(v(0,0,-.5)))



			if lod > 3 then
            	texture('church_new_clock.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			elseif lod > 2 then
			    texture('church_new_clock_m.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			else
			    texture('church_new_clock_s.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			end
   			use_material('cutout')
			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(-8,-7,-8)))
	        use_material('cutout')
			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(8.01,7.01,8.01)))

        end
	end
})
--]]
