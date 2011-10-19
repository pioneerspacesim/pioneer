define_model('mini_ant', {
	info =  {
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			},
    static = function(lod)
            texture('metal.png')
            ring(3*lod, v(0,0,0), v(0,.15,0), v(1,1,0), .02)
            ring(3*lod, v(0,0,0), v(0,-.15,0), v(-1,1,0), .02)
			cylinder(3*lod, v(0,.15,0), v(-.3,.15,0), v(1,1,0), .02)
			cylinder(3*lod, v(0,-.15,0), v(-.3,-.15,.0), v(-1,1,0), .02)
	end
})

define_model('maxi_ant', {
	info =  {
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
   			},
    static = function(lod)
            texture('metal.png')
            ring(3*lod, v(0,0,0), v(0,.5,0), v(1,1,0), .03)
            ring(3*lod, v(0,0,0), v(0,-.5,0), v(-1,1,0), .03)
			cylinder(3*lod, v(0,.5,0), v(-.6,.5,0), v(1,1,0), .03)
			cylinder(3*lod, v(0,-.5,0), v(-.6,-.5,.0), v(-1,1,0), .03)
	end,
	dynamic = function(lod)
	        local rot = math.pi*get_time('SECONDS')
            call_model('mini_ant', v(-.5,.5,0), v(1,0,0), v(0,math.cos(rot),math.sin(rot)), 1)
   			call_model('mini_ant', v(-.5,-.5,0), v(1,0,0), v(0,math.cos(rot),math.sin(rot)), 1)
	end
})

define_model('antenna_1', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
   			texture('metal.png')
			--sphere_slice(3*lod, 2*lod, 0, 0.4*math.pi, Matrix.scale(v(.3,.2,.3)))
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')
			cylinder(3*lod, v(0,0,.1), v(0,0,-1), v(0,1,1), .03)
    end,
	dynamic = function(lod)
	        local rot = math.pi*get_time('SECONDS')
	        use_material('chrome')
			call_model('maxi_ant', v(0,0,-.6), v(0,0,1), v(math.cos(0.25*rot),math.sin(0.25*rot),0), 1)
   			call_model('mini_ant', v(0,0,-.9), v(0,0,1), v(math.cos(0.5*rot),-math.sin(0.5*rot),0), 1)
	end
})

define_model('antenna_2', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
   			texture('metal.png')
			sphere_slice(3*lod, 2*lod, 0, 0.4*math.pi, Matrix.translate(v(0,0,.13)) * Matrix.rotate(0.5*math.pi, v(-1,0,0)) * Matrix.scale(v(.2,.1,.2)))
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')
			cylinder(3*lod, v(0,0,.1), v(0,0,-1), v(0,1,1), .03)
    end,
	dynamic = function(lod)
	        local rot = math.pi*get_time('SECONDS')
	        use_material('chrome')
			call_model('maxi_ant', v(0,0,-.6), v(0,0,1), v(math.cos(0.25*rot),math.sin(0.25*rot),0), 1)
   			call_model('mini_ant', v(0,0,-.9), v(0,0,1), v(math.cos(0.5*rot),-math.sin(0.5*rot),0), 1)
	end
})