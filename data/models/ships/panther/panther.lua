--[[

define_model('pantengl', {     
   	info = {
			lod_pixels = { .1, 10 ,100, 0 },
			bounding_radius = 20,
        	},

   	static = function(lod)
		if lod > 1 then
			texture('pantherleg.png')
        end
		load_obj('panthereng.obj', Matrix.rotate(math.pi,v(0,0,1))) 
	end
})
  
define_model('pantengr', {     
	info = {
         	lod_pixels = { .1, 10 ,100, 0 },
			bounding_radius = 20,
         	},

	static = function(lod)
    	if lod > 1 then
			texture('pantherleg.png')
        end
		load_obj('panthereng.obj')
     end
})    


--	you had once used the selector for skin variation, why not?
--	if you still like to have two models
--	all what is dynamic here will be static in the main model

define_model('pant_sub', {     
   	info = {
			lod_pixels = { .1, 50 ,400, 0 },
			bounding_radius = 70,
        	materials={'top', 'bot', 'steel', 'glass', 'inside'},
			},

   	static = function(lod)
   			if lod == 1 then
				load_obj('pant_coll.obj')
			end   

--			i added this because lod 1 represents the collision mesh
--			if the collision mesh is large (>1000 polys) the program can get slow
--			further textures and materials aren't needed for a collision mesh (textures slow down to)
--			the "pant_coll.obj" is rebuild using ~50 polys
--			you can script or rebuild a very low poly mesh based on the main vectors of your ship to make it as low as possible
--			but leave lod 1 or add a collision mesh for the UC, else the docking can't be released proper and the ship is levelled wrong.
--			this could be a very simple one, maybe a box that appears only "if get_arg(0) ~= 0" to represent the UC.
--			even a single quad where the lowest part of the UC is would be enough
--			if the lod is set "if lod == 1 then" and lod_pixels is ".1" for lod 1, the mesh won't appear, it's used for collision detection only.
--			you can use the low poly mesh also for a real body at next lod if you map the texture to it
--			set lod 2 to 50 instead 100 if you don't like the lowpoly mesh appear so early

	end,
   	dynamic = function(lod)
   		if lod > 1 then
			selector2()
			--select2 = 10
			if select2 < 65 then
				if lod > 3 then
					texture('models/ships/panther/panthernew.png')
				elseif lod > 2 then
					texture('models/ships/panther/panthernew_m.png')
				else
					texture('models/ships/panther/panthernew_s.png')
				end
				set_material('top', get_arg_material(0))
				set_material('bot', .6,.65,.7,1,.7,.8,.9,30)
				set_material('glass', .6,.6,.6,.7,.7,.8,.9,250)
				set_material('inside', .6,.65,.7,1,.7,.8,.9,10)
			else
		    	if lod > 3 then
					texture('models/ships/panther/pantherold.png')
    			elseif lod > 2 then
					texture('models/ships/panther/pantherold_m.png')
				else
					texture('models/ships/panther/pantherold_s.png')	
				end	
		    	set_material('top', .5,.5,.5,1,.35,.38,.4,30)
				set_material('bot', .5,.5,.5,1,.35,.38,.4,30)
				set_material('glass', .6,.6,.6,.7,.7,.8,.9,100)
				set_material('inside', .6,.65,.7,1,.7,.8,.9,10)
    		end
		
			if lod > 2 then			
		        use_material('bot')
				load_obj('models/ships/panther/panther_bot.obj')
				use_material('top')
				load_obj('models/ships/panther/panther_top.obj')
				
				texture(nil)
				use_material('inside')
				set_light(1, 0.9, v(0,1.4,-13.1), v(1,1,1))
				set_light(2, 0.9, v(-.8,1.4,-12.9), v(1,.2,.2))
				set_light(3, 0.9, v(.8,1.4,-12.9), v(.2,1,.2))
				set_local_lighting(true)
				use_light(1) 
				use_light(2)
				use_light(3)
				load_obj('models/ships/panther/panther_inside.obj')
				
				if lod > 3 then
					call_model('pilot1', v(-.8,1.57,-12.5), v(1,0,0), v(0,1,0), .06)
					call_model('pilot1', v(.8,1.57,-12.5), v(1,0,0), v(0,1,0), .06)
					call_model('pilot1', v(0,1.56,-12.7), v(1,0,0), v(0,1,0), .06)
					call_model('console1', v(-.6,1.4,-12.9), v(1,0,0), v(0,1,0), .3)
					call_model('console1', v(1,1.4,-12.9), v(1,0,0), v(0,1,0), .3)
					call_model('console1', v(.2,1.4,-13.1), v(1,0,0), v(0,1,0), .3)
				end
				
				
				set_local_lighting(false)
				texture('models/ships/panther/panthernew_m.png')
				use_material('glass')
				load_obj('models/ships/panther/panther_glass.obj')
				use_material('top')
			else
				texture(nil)
				use_material('bot')
				load_obj('models/ships/panther/pant_coll_bot.obj')
				use_material('top')
				load_obj('models/ships/panther/pant_coll_top.obj')
			end
		end
	    
--		i call them here to make use of colorvariable material
--		they have no special collision mesh so they are excludet and used in all lod
--		therefore texture is limited to "lod > 1"
--		the material "top" is taken from the last command above, so it don't need to be repeatet here

		local rot = get_arg(0)	
		local v1 = v(-6,-3.627+3*rot,-7.017+2*rot) 
		local v2 = v(6,-3.627+3*rot,-7.017+2*rot)  
		local v3 = v(-6,-3.627+3*rot,10.983-2*rot) 
		local v4 = v(6,-3.627+3*rot,10.983-2*rot)
		call_model('pantengl', v1, v(-1,0,0), v(0,1-1.2*rot,.1+rot), 1)
		call_model('pantengr', v2, v(-1,0,0), v(0,1-1.2*rot,.1+rot), 1)
		call_model('pantengl', v3, v(-1,0,0), v(0,-1+1.2*rot,.1+rot), 1)
		call_model('pantengr', v4, v(-1,0,0), v(0,-1+1.2*rot,.1+rot), 1)
	end
})  

define_model('panther', {     
	info = {
            scale = 2.8,
            lod_pixels = { .1, 100 ,500, 0 },
			bounding_radius = 70,
            materials={'top', 'bot', 'steel', 'text1', 'text2', 'glow'},
            tags = { 'ship' },
            ship_defs = {
					{
					name='Panther',
					forward_thrust = -10e7,
					reverse_thrust = 3e7,
					up_thrust = 3e7,
					down_thrust = -1e7,
					left_thrust = -1e7,
					right_thrust = 1e7,
					angular_thrust = 25e7,
					gun_mounts = 
					{
					{ v(0,-0.5,0), v(0,0,-1) },
					{ v(0,0,0), v(0,0,1) },
					},
					max_cargo = 740,
					max_laser = 2,
					max_missile = 20,
					capacity = 740,
					hull_mass = 700,
					price = 2.1e6,
					hyperdrive_class = 7,
                   	}
                }
            },
	static = function(lod)
		call_model('pant_sub',v(0,0,0),v(1,0,0),v(0,1,0),1)
				
		if lod > 1 then
		    if lod > 2 then
				set_material('text1', .45,.45,.45,1,.1,.1,.1,10)
	   			set_material('text2', .55,.55,.1,1,.1,.1,.1,10)
                set_material('steel', .2,.23,.25,1,.35,.38,.4,30)

                use_material('steel')
				load_obj('panther_steel.obj')
					
				if lod > 3 then
					texture('panthernew.png')
				else
					texture('panthernew_m.png')
				end	
				use_material('glow')
				load_obj('panther_glow.obj')
			end
							
			-- possibly i will change the posl to a function that accepts a color argument
			
		end
    end,
            
    dynamic = function(lod)
	    if lod > 1 then
			if lod > 2 then
				set_material('glow', lerp_materials(get_arg(1)*0.5,	{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1.5 },
																	{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1 }))
				local reg = get_arg_string(0)
				use_material('text1')
				zbias(1, v(0, -1.127, 18.783), v(0,0.25,1))
				text(reg, v(0, -1.127, 18.783), v(0,0.25,1), v(1,0,0), 1.2, {center=true})
				zbias(0)
				use_material('text2')
				zbias(1, v(0, -1.527, -18.867), v(0,.2,-1))
				text(reg, v(0, -1.527, -18.867), v(0,.2,-1), v(-1,0,0), 1.0, {center=true})
			    zbias(0)

			    if get_arg(10) > 0 then
					use_material('steel')
					call_model('largegun1',v(0,-2.56,-19),v(1,0,0),v(0,1,0),.34)
				end
				
				if get_arg(11) > 0 then
					use_material('steel')
					call_model('largegun2',v(0,3.55,15.6),v(1,0,0),v(0,1,0),.34)
				end
				
				if get_arg(8) > 0 then
					call_model('scanner', v(0,-4.2,-14.546), v(1,0,0), v(0,-1,0), 1)
					call_model('scanner', v(0,3.45,4.6), v(1,0,0), v(0,1,0), 1)
					call_model('antenna_1', v(3,-2.56,-19), v(1,0,0), v(0,1,0), 1)
				end
			
				if get_arg(7) > 0 then
					call_model('ecm_1', v(-9.5,-1.789,-7.958), v(0,1,0), v(-1,0,0), 1)
					call_model('ecm_1', v(9.5,-1.789,-7.958), v(0,-1,0), v(1,0,0), 1)
				end
				
			local M_1 = v(-5,-4.75,-11.604)
            local M_2 = v(5,-4.75,-11.604)
            local M_3 = v(-4,-4.75,-12)
            local M_4 = v(4,-4.75,-12)
            local M_5 = v(-3,-4.75,-12.4)
            local M_6 = v(3,-4.75,-12.4)
            local M_7 = v(-2,-4.75,-12.8)
            local M_8 = v(2,-4.75,-12.8)
            
               
            if get_arg(12) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_1,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(12) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_1,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(12) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_1,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(12) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_1,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(13) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_2,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(13) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_2,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(13) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_2,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(13) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_2,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(14) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_3,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(14) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_3,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(14) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_3,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(14) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_3,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(15) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_4,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(15) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_4,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(15) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_4,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(15) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_4,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(16) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_5,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(16) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_5,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(16) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_5,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(16) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_5,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(17) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_6,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(17) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_6,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(17) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_6,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(17) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_6,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(18) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_7,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(18) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_7,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(18) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_7,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(18) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_7,v(1,0,0), v(0,.95,.05),1)
            end
            
            if get_arg(19) == Equip.MISSILE_UNGUIDED  then
                call_model('m_unguided',M_8,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(19) == Equip.MISSILE_GUIDED  then
                call_model('m_guided',M_8,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(19) == Equip.MISSILE_SMART  then
                call_model('m_smart',M_8,v(1,0,0), v(0,.95,.05),1)
            elseif get_arg(19) == Equip.MISSILE_NAVAL  then
                call_model('m_naval',M_8,v(1,0,0), v(0,.95,.05),1)
            end
				
				
		    end
	        
	        if get_arg(0) ~= 0 then
				call_model('headlight',v(0,-2.78,-18.7),v(1,0,0),v(0,0,-1),3)
			
				call_model('posl_green', v(9.88,-2.6,-5), v(0,0,1), v(1,-0.27,0),1)
				call_model('posl_red', v(-9.88,-2.6,-5), v(0,0,1), v(-1,-0.27,0),1)
			end
	        
			local M_T = v(-7,-1.127,16.017)
			xref_thruster(M_T, v(0,0,1), 20, true) -- i set thrusters dynamic, else the billboard function (posl.) messes with them

			local rot = get_arg(0)	
			local LFB_T = v(-6,-4.7-.8*rot,-6+5*(1-rot))
	        local RFB_T = v(6,-4.7-.8*rot,-6+5*(1-rot))
	        local LRB_T = v(-6,-4.7-.8*rot,10-5*(1-rot))
	        local RRB_T = v(6,-4.7-.8*rot,10-5*(1-rot))
			thruster(LFB_T, v(0,-rot,-rot+.8), 8)
			thruster(RFB_T, v(0,-rot,-rot+.8), 8)
			thruster(LRB_T, v(0,-rot,rot-.8), 8)
			thruster(RRB_T, v(0,-rot,rot-.8), 8)
	   	end
	end
})
--]]

			--[[
			useful abbreviations for thruster vectors
			
			M_T(n) 		= main thruster(n)
			R_T(n)		= retro thruster
			
			RF_T(n)		= right front thruster
			LF_T(n)		= left front thruster
			RR_T(n)		= right rear thruster
			LR_T(n)		= left rear thruster
			
			RFT_T(n)	= right front top thruster	
			LFT_T(n)	= left front top thruster
			or 
			FT_T(n)		= front top thruster, uses xref_thruster for r and l
			
			RRT_T(n)	= right rear top thruster
			LRT_T(n)	= left rear top thruster
			or
			RT_T(n)		= rear top thruster
			
			RFB_T(n)	= right front bottom thruster
			LFB_T(n)	= left front bottom thruster
			or
			FB_T(n)		= front bottom thruster
			
			RRB_T(n)	= right rear bottom thruster
			LRB_T(n)	= left rear bottom thruster
			or
			RB_T(n)		= rear botom thruster
			--]]			
