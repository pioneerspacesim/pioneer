---[[
define_model('ad_cola_1', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,2,.8,1.2)
        set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)
        
		geomflag(0x8000)
		use_material('glow1')
		if lod < 4 then
		  	texture('wtr_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
            
		else
		    texture('wtr.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		end
		quad(v0,v1,v2,v3)
		
		use_material('glow2')
		if lod < 4 then
			texture('coolcola_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
		    texture('coolcola.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		end 
			zbias(1,v(0,.5,0), v(0,0,1))
			quad(v0,v1,v2,v3)  
		   	zbias(0) 
		geomflag(0)
		
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.3, {0,0,0,1,0,0,0,0,.6,1.5,2.5},
															  {0,0,0,1,0,0,0,0,2,.6,1.5}))
		
	
	
	end
})
--]]
---[[
define_model('ad_cola_0', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,2,.8,1.2)
        set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)
        
		if lod < 4 then
			use_material('glow1')
        	texture('wtr_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('coolcola_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0) 
		end
	end,
	dynamic = function(lod)
        if lod > 3 then
            local v0 = v(1,0,0)
		    local v1 = v(1,1,0)
		    local v2 = v(-1,1,0)
		    local v3 = v(-1,0,0)
		    local trans = get_arg(1)*.1

			use_material('glow1')
        	texture('sub_models/adverts/wtr.png', v(.5,math.sin(trans),0),v(.5,0,0),v(0,math.cos(trans),0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('sub_models/adverts/coolcola.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0)
		end
	end
})


define_model('ad_acme_0', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,2,1.8,.6)
        set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

		if lod < 4 then
			use_material('glow1')
        	texture('wtr_x_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('acme_0s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0)
		end
	end,
	dynamic = function(lod)
        if lod > 3 then
            local v0 = v(1,0,0)
		    local v1 = v(1,1,0)
		    local v2 = v(-1,1,0)
		    local v3 = v(-1,0,0)
		    local trans = get_arg(1)*.1

			use_material('glow1')
        	texture('sub_models/adverts/wtr_x.png', v(.5,trans,0),v(.5,0,0),v(0,1,0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('sub_models/adverts/acme_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0)
		end
	end
})
--]]

define_model('ad_acme_1', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,2,1.8,.6)
        set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

        use_material('glow1')
		if lod < 4 then
			texture('wtr_x_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
        else
		    texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		end    
            quad(v0,v1,v2,v3)
        
		use_material('glow2')    
        if lod < 4 then
			texture('acme_0s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
   		    texture('acme_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
   		
   		
   		end
   		zbias(1,v(0,.5,0), v(0,0,1))
   		quad(v0,v1,v2,v3)
   		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.1, {0,0,0,1,0,0,0,0,2.5,1.2,.6},
															  {0,0,0,1,0,0,0,0,1.5,1.8,.6}))
		
	
	
	end
}) 

define_model('ad_acme_2', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,.99,0,0,0,0,2,1.8,.6)
        
		if lod < 4 then
			texture('acme_1s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
			texture('acme_1.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))	
		end	
		use_material('glow1')	
		quad(v0,v1,v2,v3)	
	end
})

define_model('ad_pioneer_0', {
	info = {
			lod_pixels = {1,20,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
        set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		geomflag(0x8000)
		if lod < 4 then
			use_material('glow1')
        	texture('wtr_x_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('pioneer_0_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0)
		end
		geomflag(0)
	end,
	dynamic = function(lod)
        if lod > 3 then
            local v0 = v(1,0,0)
		    local v1 = v(1,1,0)
		    local v2 = v(-1,1,0)
		    local v3 = v(-1,0,0)
		    local trans = get_arg(1)*.1

			geomflag(0x8000)
			use_material('glow1')
        	texture('sub_models/adverts/wtr_x.png', v(math.sin(trans),math.cos(trans),0),v(.5,0,0),v(0,1,0))
            quad(v0,v1,v2,v3)
			if lod < 4 then
            	texture('sub_models/adverts/pioneer_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			else
			    texture('sub_models/adverts/pioneer_0_l.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			end
			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
   			quad(v0,v1,v2,v3)
   			zbias(0)
		end
		geomflag(0)
	end
})
  
---[[
define_model('ad_sirius_0', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
        set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		geomflag(0x8000)
		if lod < 4 then
			use_material('glow1')
        	texture('tie-d_b_s.png', v(.5,.5,0),v(.5,0,0),v(0,.05,0))
            quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('sirius_2_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
   			zbias(0)
		end
		geomflag(0)
	end,
	dynamic = function(lod)
        if lod > 3 then
            local v0 = v(1,0,0)
		    local v1 = v(1,1,0)
		    local v2 = v(-1,1,0)
		    local v3 = v(-1,0,0)
		    local trans = get_arg(1)*.05

			geomflag(0x8000)
			use_material('glow1')
        	texture('sub_models/adverts/tie-d_b.png', v(.5,trans,0),v(.4,0,0),v(0,.05,0))
            quad(v0,v1,v2,v3)
			if lod < 4 then
            	texture('sub_models/adverts/sirius_2_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			else
			    texture('sub_models/adverts/sirius_2.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			end
			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
   			quad(v0,v1,v2,v3)
   			zbias(0)
		end
		geomflag(0)
	end
})
--]]

define_model('ad_sirius_1', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        --set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
        set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		
		--geomflag(0x8000)
		use_material('glow1')
		if lod < 4 then
			texture('tie-d_b_s.png', v(0,0,0),v(.25,0,0),v(0,1,0))
        else
			texture('tie-d_b.png', v(0,0,0),v(.25,0,0),v(0,1,0))
		end
		quad(v0,v1,v2,v3)
        use_material('glow2')
		if lod < 4 then
		    texture('sirius_2_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
			texture('sirius_2.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		end
		zbias(1,v(0,.5,0), v(0,0,1))
		quad(v0,v1,v2,v3)
   		zbias(0)
		geomflag(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.2, {0,0,0,1,0,0,0,0,.5,2.5,1.5},
															  {0,0,0,1,0,0,0,0,.5,1.5,2.5}))
		
	
	
	end
})


define_model('ad_sirius_2', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        set_material('glow1', 0,0,0,.99,0,0,0,0,1,1.5,2)
        
		if lod > 3 then
			texture('sirius_3.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
			texture('sirius_3_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))	
		end	
		use_material('glow1')	
		quad(v0,v1,v2,v3)	
	end
})

define_model('inteloutside', {
	info = {
			lod_pixels = {1,10,30,0},
			bounding_radius = 2,
			materials = {'glow1', 'glow2'},
            },
	static = function(lod)
	    local v0 = v(1,0,0)
	    local v1 = v(1,1,0)
	    local v2 = v(-1,1,0)
	    local v3 = v(-1,0,0)

        
        set_material('glow2', 0,0,0,.9,0,0,0,0,.5,.6,.9)

        use_material('glow1')
		if lod < 4 then
			texture('wtr_x_s.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
        else
		    texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		end    
            quad(v0,v1,v2,v3)
        
		use_material('glow2')    
        if lod < 4 then
			texture('inteloutside_s.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		else
   		    texture('inteloutside.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
   		
   		
   		end
   		zbias(1,v(0,.5,0), v(0,0,1))
   		quad(v0,v1,v2,v3)
   		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.1, {0,0,0,1,0,0,0,0,2.5,2,.6},
															  {0,0,0,1,0,0,0,0,.6,2,2.5}))
		
	
	
	end
		
}) 


