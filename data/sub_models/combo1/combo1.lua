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
		
			--call_model('combo1_wins', v(0,0,0),v(1,0,0),v(0,1,0),1)
			
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
