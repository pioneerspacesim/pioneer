-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('head_m1', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m1.png')
		load_obj('head.obj')
	end
})

define_model('head_m2', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m2.png')
		load_obj('head.obj')
	end
})

define_model('head_m3', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m3.png')
		load_obj('head.obj')
	end
})

define_model('head_m4', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m4.png')
		load_obj('head.obj')
	end
})

define_model('head_m5', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m5.png')
		load_obj('head.obj')
	end
})

define_model('head_m6', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_m6.png')
		load_obj('head.obj')
	end
})

define_model('head_f1', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f1.png')
		load_obj('head.obj')
	end
})

define_model('head_f2', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f2.png')
		load_obj('head.obj')
	end
})

define_model('head_f3', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f3.png')
		load_obj('head.obj')
	end
})

define_model('head_f4', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f4.png')
		load_obj('head.obj')
	end
})

define_model('head_f5', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f5.png')
		load_obj('head.obj')
	end
})

define_model('head_f6', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
     	use_material('head')
		texture('head_f6.png')
		load_obj('head.obj')
	end
})

define_model('head_m1_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m1.png')
		load_obj('head.obj')
	end
})

define_model('head_m2_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m2.png')
		load_obj('head.obj')
	end
})

define_model('head_m3_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m3.png')
		load_obj('head.obj')
	end
})

define_model('head_m4_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m4.png')
		load_obj('head.obj')
	end
})

define_model('head_m5_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m5.png')
		load_obj('head.obj')
	end
})

define_model('head_m6_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_m6.png')
		load_obj('head.obj')
	end
})

define_model('head_f1_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f1.png')
		load_obj('head.obj')
	end
})

define_model('head_f2_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f2.png')
		load_obj('head.obj')
	end
})

define_model('head_f3_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f3.png')
		load_obj('head.obj')
	end
})

define_model('head_f4_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f4.png')
		load_obj('head.obj')
	end
})

define_model('head_f5_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f5.png')
		load_obj('head.obj')
	end
})

define_model('head_f6_lit', {
	info = {
			bounding_radius = 2,
			materials = {'head'},
			},

	static = function(lod)
        set_material('head', .45,.43,.4,1,.4,.4,.4,5)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
     	use_material('head')
		texture('head_f6.png')
		load_obj('head.obj')
	end
})

define_model('helmet_old_0', {
	info = {
 			lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'glass', 'radio'},
			},

	static = function(lod)
		set_material('glass', 0,0,.05,.4,1,1,1.5,100)
     	set_material('radio', .3,.32,.35,1,.5,.55,.6,5)

		use_material('radio')
		xref_cylinder(3*lod,v(1,0,0),v(1.05,0,0),v(0,1,0),.2)
		xref_tapered_cylinder(3*lod,v(1.025,.2,0),v(1.025,.7,0),v(1,0,0),.02,.01)
		tube(3*lod, v(0,-1.1,0),v(0,-.7,0),v(1,0,0),.58,.6)

		use_material('glass')
		sphere(2)
	end
})

define_model('helmet_old', {
	info = {
 		lod_pixels = {.1, 1, 5, 0},
		bounding_radius = 2,
		},

	static = function(lod)

            		call_model('helmet_old_0',v(0,.846,.185),v(1,0,0),v(0,1,.2),.18)


	end

})

define_model('helmet_mod_0', {
	info = {
 			lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'glass', 'radio', 'squad', 'black'},
			},

	static = function(lod)
     	set_material('glass', 0,0,.05,.4,1,1,1.5,100)
     	set_material('radio', .3,.32,.35,1,.5,.55,.6,5)
		set_material('black', .3,.3,.3,1,.2,.2,.2,5)

		texture('pipe.png')
        use_material('radio')
		load_obj('pipe_mod.obj')

		texture('padd.png')
		use_material('black')
		load_obj('padd.obj')

        texture('helmet.png')
		use_material('squad')
		load_obj('helmet_mod.obj')

  		texture(nil)
		use_material('glass')
		load_obj('visor_mod.obj') 		    
	end,
	dynamic = function(lod)
	   	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,1,.6,.6,.6,30)
	    elseif select1 < 401 then
           	set_material('squad', .45,.35,.01,1,.6,.6,.6,30)
		elseif select1 < 601 then
			set_material('squad', 0,.15,.7,1,.6,.6,.6,30)
   		elseif select1 < 801 then
			set_material('squad', .06,.35,0,1,.6,.6,.6,30)
		elseif select1 > 800 then
			set_material('squad', .2,0,.35,1,.6,.6,.6,30)
		end
	end
})

define_model('helmet_mod', {
	info = {
 		lod_pixels = {.1, 1, 5, 0},
		bounding_radius = 2,
		},

	static = function(lod)
	end,
	dynamic = function(lod)         
     	selector4()
	   
		if select4 < 41 then
            		call_model('helmet_mod_0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif select4 < 81 then	     
			call_model('helmet_mod_0',v(0,-.02,.034),v(1,0,0),v(0,1,0),1)
		end

	end

})

define_model('decal_f', {
	info = {
			bounding_radius = 2,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad.png')		
		zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('sign_f.obj')
		zbias(0)		                           		
	end,
	dynamic = function(lod)
	   	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.6,.6,.6,10)
	    elseif select1 < 401 then
           	set_material('squad', .45,.35,.01,.99,.6,.6,.6,10)
		elseif select1 < 601 then
			set_material('squad', 0,.15,.7,.99,.6,.6,.6,10)
   		elseif select1 < 801 then
			set_material('squad', .06,.35,0,.99,.6,.6,.6,10)
		elseif select1 > 800 then
			set_material('squad', .2,0,.35,.99,.6,.6,.6,10)
		end
	end
})

define_model('decal_m', {
	info = {
			bounding_radius = 2,
			materials = {'squad'},
			},

	static = function(lod)
		use_material('squad')
		texture('squad.png')				
		zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('sign_m.obj')
		zbias(0)		                           		
	end,
	dynamic = function(lod)
	   	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.6,.6,.6,10)
	    elseif select1 < 401 then
           	set_material('squad', .45,.35,.01,.99,.6,.6,.6,10)
		elseif select1 < 601 then
			set_material('squad', 0,.15,.7,.99,.6,.6,.6,10)
   		elseif select1 < 801 then
			set_material('squad', .06,.35,0,.99,.6,.6,.6,10)
		elseif select1 > 800 then
			set_material('squad', .2,0,.35,.99,.6,.6,.6,10)
		end
	end
})

define_model('decal_f_lit', {
	info = {
			bounding_radius = 2,
			materials = {'squad'},
			},

	static = function(lod)
		set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		use_material('squad')
		texture('squad.png')				
		zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('sign_f.obj')
		zbias(0)		                           		
	end,
	dynamic = function(lod)
	   	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.6,.6,.6,10)
	    elseif select1 < 401 then
           	set_material('squad', .45,.35,.01,.99,.6,.6,.6,10)
		elseif select1 < 601 then
			set_material('squad', 0,.15,.7,.99,.6,.6,.6,10)
   		elseif select1 < 801 then
			set_material('squad', .06,.35,0,.99,.6,.6,.6,10)
		elseif select1 > 800 then
			set_material('squad', .2,0,.35,.99,.6,.6,.6,10)
		end
	end
})

define_model('decal_m_lit', {
	info = {
			bounding_radius = 2,
			materials = {'squad'},
			},

	static = function(lod)
		set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		use_material('squad')
		texture('squad.png')				
		zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('sign_m.obj')
		zbias(0)		                           		
	end,
	dynamic = function(lod)
	   	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.6,.6,.6,10)
	    elseif select1 < 401 then
           	set_material('squad', .45,.35,.01,.99,.6,.6,.6,10)
		elseif select1 < 601 then
			set_material('squad', 0,.15,.7,.99,.6,.6,.6,10)
   		elseif select1 < 801 then
			set_material('squad', .06,.35,0,.99,.6,.6,.6,10)
		elseif select1 > 800 then
			set_material('squad', .2,0,.35,.99,.6,.6,.6,10)
		end
	end
})


define_model('pilot_seat', {
	info = 	{
			bounding_radius = 2,
			materials = {'black'},
			},
	static = function(lod)
		set_material('black',.12,.12,.1,1,.3,.3,.2,10)

		texture('chair.png')
		use_material('black')
		load_obj('chair_1.obj') -- when create new seats, center of model must be the same as pilots center. a dummy pilot is in the pilots folder, size 1:1 (dummy.obj)
	end
})

define_model('pilot_seat_lit', {
	info = 	{
			bounding_radius = 2,
			materials = {'black'},
			},
	static = function(lod)
		set_material('black',.12,.12,.1,1,.3,.3,.2,10)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		texture('chair.png')
		use_material('black')
		load_obj('chair_1.obj') 
  	end
})

define_model('pilot_cseat', {
	info = 	{
			bounding_radius = 2,
			materials = {'black','chrome'},
			},
	static = function(lod)
		set_material('black',.12,.12,.1,1,.3,.3,.2,10)
        set_material('chrome', .3,.33,.33,1,1,1.5,1.5,30)
        
        texture('pipe.png')
        use_material('chrome')
        load_obj('chair_c0.obj')
        
		texture('chair.png')
		use_material('black')
		load_obj('chair_c1.obj')
	end
})

define_model('pilot_cseat_lit', {
	info = 	{
			bounding_radius = 2,
			materials = {'black','chrome'},
			},
	static = function(lod)
		set_material('black',.12,.12,.1,1,.3,.3,.2,10)
        set_material('chrome', .3,.33,.33,1,1,1.5,1.5,30)
		set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)

		texture('pipe.png')
        use_material('chrome')
        load_obj('chair_c0.obj')

		texture('chair.png')
		use_material('black')
		load_obj('chair_c1.obj')
  	end
})

define_model('pilot_f_lit', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		texture('pilot_1.png')
		load_obj('pilot_f.obj')
	end
})

define_model('pilot_m_lit', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		texture('pilot_1.png')
		load_obj('pilot_m.obj')
	end
})

define_model('pilot_f', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
		texture('pilot_1.png')
		load_obj('pilot_f.obj')
	end
})

define_model('pilot_m', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
		texture('pilot_1.png')
		load_obj('pilot_m.obj')
	end
})

define_model('hands_f_lit', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		texture('pilot_1.png')
		load_obj('hands_f.obj')
	end
})

define_model('hands_m_lit', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	    set_light(2, 0.05, v(0,3,3), v(2,2,2))
	    set_local_lighting(true)
	    use_light(1)
	    use_light(2)
		texture('pilot_1.png')
		load_obj('hands_m.obj')
	end
})

define_model('hands_f', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
		texture('pilot_1.png')
		load_obj('hands_f.obj')
	end
})

define_model('hands_m', {
	info = {
			bounding_radius = 2,
			}, 
	static = function(lod)
		texture('pilot_1.png')
		load_obj('hands_m.obj')
	end
})

define_model('pilot_1_lit', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

	        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	        set_light(2, 0.05, v(0,3,3), v(2,2,2))

	        set_local_lighting(true)
	        use_light(1)
	        use_light(2)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,
	
	dynamic = function(lod)

        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
	        elseif select3 < 21 then
	            set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 31 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 41 then
			    set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 51 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 61 then
	        	set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
	        elseif select3 < 71 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 81 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 91 then
				set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 > 90 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m1_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
				    call_model('head_m2_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m3_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m4_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m5_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f1_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
				    call_model('head_f2_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f3_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f4_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f5_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
			set_local_lighting(false)
		end
	end
})

define_model('pilot_2_lit', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

	        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	        set_light(2, 0.05, v(0,3,3), v(2,2,2))

	        set_local_lighting(true)
	        use_light(1)
	        use_light(2)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,
	
	dynamic = function(lod)

     	
        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
	        elseif select3 < 21 then
	            set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 31 then
			    set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 41 then
			    set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 51 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 61 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
	        elseif select3 < 71 then
	        	set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 81 then
				set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 91 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 > 90 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f4_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_f5_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f3_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_f2_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f1_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m4_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_m5_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m3_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_m2_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m1_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
			set_local_lighting(false)
		end
	end
})

define_model('pilot_3_lit', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

	        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
	        set_light(2, 0.05, v(0,3,3), v(2,2,2))

	        set_local_lighting(true)
	        use_light(1)
	        use_light(2)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,
	
	dynamic = function(lod)

     	
        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
	        elseif select3 < 21 then
	            set_material('body',.02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 31 then
			    set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 41 then
			    set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 51 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 61 then
	        	set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
	        elseif select3 < 71 then
	        	set_material('body',.02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 81 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 91 then
				set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 > 90 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f1_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_f3_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f4_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_f5_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f2_lit',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m1_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_m3_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m4_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_m5_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m2_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m_lit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
			set_local_lighting(false)
		end
	end
})

define_model('pilot_1', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,                            	
	dynamic = function(lod)         
     	
        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
	        elseif select3 < 21 then
	            set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 31 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 41 then
			    set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 51 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 61 then
	        	set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
	        elseif select3 < 71 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 81 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 91 then
				set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 > 90 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
				    call_model('head_m2',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m3',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m4',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m5',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m',v(0,0,0),v(1,0,0),v(0,1,0),1) 
          
			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f1',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
				    call_model('head_f2',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f3',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f4',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f5',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f',v(0,0,0),v(1,0,0),v(0,1,0),1)


			end
		end
	end
})

define_model('pilot_2', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,
	
	dynamic = function(lod)

     	
        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
	        elseif select3 < 21 then
	            set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 31 then
			    set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 41 then
			    set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 51 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 61 then
	        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
	        elseif select3 < 71 then
	        	set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 81 then
				set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
			elseif select3 < 91 then
				set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 > 90 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f4',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_f5',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f3',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_f2',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f1',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m4',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_m5',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m3',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_m2',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
			
			end
		end
	end
})

define_model('pilot_3', {
	info = 	{
            lod_pixels = {.1, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'head', 'hands', 'body', 'black', 'squad'},
			},        
	static = function(lod)
		if lod > 1 then
			set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
			set_material('black',.12,.12,.1,1,.3,.3,.2,10)

			texture('pilot_1.png')
			use_material('feet')
			load_obj('feet.obj')  
		end
  	end,
	
	dynamic = function(lod)

     	
        if lod > 1 then
		selector4()
     	selector2()
     	selector3()
	     	if select3 < 11 then
	        	set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
	        elseif select3 < 21 then
	            set_material('body',.02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 31 then
			    set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 41 then
			    set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 < 51 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			elseif select3 < 61 then
	        	set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
	        elseif select3 < 71 then
	        	set_material('body',.02, 0, .04, 1, .2, .2, .2, 5)
			elseif select3 < 81 then
				set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
			elseif select3 < 91 then
				set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			elseif select3 > 90 then
				set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			end

			if select4 < 51 then
		  		if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_f1',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_f3',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f4',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_f5',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f6',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_f2',v(0,-.014,.035),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_f',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_f',v(0,0,0),v(1,0,0),v(0,1,0),1)

			else
	        	if select2 < 17 then
	        		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
	        		call_model('head_m1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 34 then
				    set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
				    call_model('head_m3',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 51 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m4',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 68 then
					set_material('hands', .18,.15,.5,1,.3,.3,.3,5)
					call_model('head_m5',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 < 85 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m6',v(0,0,0),v(1,0,0),v(0,1,0),1)
				elseif select2 > 84 then
					set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
					call_model('head_m2',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
				use_material('body')
				call_model('pilot_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				use_material('hands')
				call_model('hands_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('decal_m',v(0,0,0),v(1,0,0),v(0,1,0),1)
			
			end
		end
	end
})



