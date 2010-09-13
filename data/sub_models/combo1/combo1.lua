define_model('combo1_wins', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'win0', 'win1', 'win2', 'win3'},
			
		},
	static = function(lod)
        
		
	    if lod > 2 then
			texture('alu.png')
		else
		    texture('alu_s.png')
 		end
		use_material('win0')
		load_obj('combo1_win0.obj')
		use_material('win1')
		load_obj('combo1_win1.obj')
		use_material('win2')
		load_obj('combo1_win2.obj')
		use_material('win3')
		load_obj('combo1_win3.obj')
		
	end,	
	dynamic = function(lod)	
		local phase = math.fmod((get_arg(3)/6),1)

		if phase < .251 then
   			set_material('win0', .2,.33,.35,1,1.5,1.8,2,100)
            set_material('win1', .2,.33,.35,1,1.5,1.8,2,100)
			set_material('win2', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
            set_material('win3', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
        else
		    if phase < .501 then
		        set_material('win0', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
                set_material('win1', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
    			set_material('win2', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
                set_material('win3', .2,.33,.35,1,1.5,1.8,2,100)
			else
				if phase < .751 then
				    set_material('win0', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
                	set_material('win1', .2,.33,.35,1,1.5,1.8,2,100)
    				set_material('win2', .2,.33,.35,1,1.5,1.8,2,100)
                	set_material('win3', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
				else
				    if phase > .750 then
				        set_material('win0', .2,.33,.35,1,1.5,1.8,2,100)
	                	set_material('win1', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
	    				set_material('win2', .2,.33,.35,1,1.5,1.8,2,100,1.2,1.4,1.6)
	                	set_material('win3', .2,.33,.35,1,1.5,1.8,2,100)
					end
				end
			end
		end
			
		
		
	end			
})

define_model('combo1', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			materials={'cutout', 'trees'},
			
		},
	static = function(lod)
        set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
				
		if lod > 1 then
			texture('alu.png')

			load_obj('combo1_1.obj')
		
			call_model('combo1_wins', v(0,0,0),v(1,0,0),v(0,1,0),1)
			
			if lod > 2 then
				texture('door.png')
			else
			    texture('door_s.png')
			end
			use_material('cutout')
			zbias(1,v(0,5.909,3),v(0,0,1))
            load_obj('combo1_door2.obj')
		    zbias(0)
			
            if lod > 2 then
   				texture('door.png',v(.5,.29,.5),v(.54,0,0),v(0,.63,0))
   			else
       			texture('door_s.png',v(.5,.29,.5),v(.54,0,0),v(0,.63,0))
			end
			zbias(1,v(0,1.25,7),v(0,0,1))
			circle(6,v(0,1.25,7),v(0,0,1),v(1,0,0),.8)
			zbias(0)

			if lod > 2 then
			    texture('tree.png')
			else
       			texture('tree_s.png')
			end
			    use_material('trees')
			    load_obj('bush_0.obj')
   		end
 end
})

define_model('combo0', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			materials={'win', 'cutout', 'trees', 'concrete'},
			
			
		},
	static = function(lod)
        set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
		set_material('win', .2,.33,.35,.4,1.5,1.8,2,100)
		
		if lod > 1 then
		    if lod > 2 then
				texture('alu.png')
			else
			    texture('alu_s.png')
			end
		end
		load_obj('combo1_1.obj')
		
		use_material('concrete')
		call_model('bld_base_1', v(0,0,0),v(1,0,0),v(0,1,0),1)

		if lod > 1 then



   			if lod > 2 then
				texture('door.png')
			else
			    texture('door_s.png')
			end
			use_material('cutout')
			zbias(1,v(0,5.909,3),v(0,0,1))
			load_obj('combo1_door2.obj')
		    zbias(0)

			if lod > 2 then
   				texture('door.png',v(.5,.29,.5),v(.54,0,0),v(0,.63,0))
   			else
       			texture('door_s.png',v(.5,.29,.5),v(.54,0,0),v(0,.63,0))
			end
			zbias(1,v(0,1.25,7),v(0,0,1))
			circle(6,v(0,1.25,7),v(0,0,1),v(1,0,0),.8)
			zbias(0)
			
			if lod > 2 then
		    	texture('tree.png')
			else
			    texture('tree_s.png')
			end
				use_material('trees')
		    	load_obj('bush_1.obj')


            if lod > 2 then
				texture('alu.png')
			else
			    texture('alu_s.png')
			end

   			call_model('combo1_wins', v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('win')
			load_obj('combo1_plus_win.obj')
		end

	end
})	
--[[
define_model('combo0_0', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			materials={'concrete'},
			
			
		},
	static = function(lod)
        set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
		
		if lod > 1 then
			if lod > 2 then
				texture('concrete.png')
			else
                texture('concrete_s.png')
			end
			use_material('concrete')
		
			load_obj('combo1_1_plus.obj')
		    	
		end		
	end
})	
--]]


		
		

		
		
