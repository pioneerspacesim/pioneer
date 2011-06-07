define_model('nazzle_s', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'e_glow', 'nazzle'},
			},
	static = function(lod)
	    set_material('nazzle', 	.75, .8, 1,1, .8, 1, 1.5, 50)
		use_material('nazzle')
        if lod > 1 then
			texture('naz_2.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		else
			texture('naz_2s.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		end
		sphere_slice(6,2*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.1547,1.7,1.1547)))
		tube(6,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
		use_material('e_glow')
		if lod > 1 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .9)
	end,
	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_arg(1)*0.5, 	{.3, .3, .3, 1, 0, 0, 0, 0, .7, 1, 1.5 },
																{.3, .3, .3, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
	end
})

define_model('nazzle_l', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'e_glow', 'nazzle'},
			},
	static = function(lod)
	    set_material('nazzle',	.75, .8, 1,1, .8, 1, 1.5, 50)
		use_material('nazzle')
        if lod > 2 then
			texture('naz_2.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		else
			texture('naz_2s.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		end
		sphere_slice(18,3*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.01542,1.6,1.01542)))
		tube(18,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
		use_material('e_glow')
		if lod > 2 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .85)
	end,
	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_arg(1)*0.5,  {.3, .3, .3, 1, 0, 0, 0, 0, .7, 1, 1.5 },
																{.3, .3, .3, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
	end
})

define_model('nazzle_n', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'nazzle', 'naz_no'},
			},
	static = function(lod)
	    set_material('nazzle', 	.75, .8, 1,1, .8, 1, 1.5, 50)
		use_material('nazzle')
        if lod > 1 then
			texture('naz_2.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		else
			texture('naz_2s.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		end
		sphere_slice(6,2*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.1547,1.7,1.1547)))
		tube(6,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
        set_material('naz_no', 	.45, .5, .7)
		use_material('naz_no')
		if lod > 1 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .9)
	end
})

-- for the following types define material in model script

define_model('nazzle1_s', {
	info = 	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius = 1,
			},
			
	static = function(lod)
		if lod > 1 then
			texture('naz_2.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		else
			texture('naz_2s.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		end
		load_obj('nazzle_s.obj', Matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1)))
	end
})

define_model('nazzle1_l', {
	info = 	{
			lod_pixels = {10, 20, 50, 0},
            bounding_radius = 1,
			},
			
	static = function(lod)
		if lod > 2 then
			texture('naz_2.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		else
			texture('naz_2s.png', v(.5,.5,0), v(.2,0,0), v(0,0,1))
		end
		load_obj('nazzle_l.obj', Matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1)))
	end
})

define_model('nazzle2_s', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'e_glow'},
			},
	static = function(lod)
	    if lod > 1 then
			texture('naz_n.png', v(0,.7,0), v(.1,0,0), v(0,-.4,0))
		else
			texture('naz_ns.png', v(0,.7,0), v(.1,0,0), v(0,-.4,0))
		end
		sphere_slice(6,2*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.1547,1.7,1.1547)))
		tube(6,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
		use_material('e_glow')
		if lod > 1 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .9)
	end,
	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_arg(1)*0.5, 	{.3, .3, .3, 1, 0, 0, 0, 0, .7, 1, 1.5 },
																{.3, .3, .3, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
	end
})

define_model('nazzle2_l', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'e_glow'},
			},
	static = function(lod)
	    if lod > 2 then
			texture('naz_n.png', v(0,.68,0), v(.1,0,0), v(0,-.42,0))
		else
			texture('naz_ns.png', v(0,.68,0), v(.1,0,0), v(0,-.42,0))
		end
		sphere_slice(18,3*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.01542,1.6,1.01542)))
		tube(18,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
		use_material('e_glow')
		if lod > 2 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .85)
	end,
	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_arg(1)*0.5,  {.3, .3, .3, 1, 0, 0, 0, 0, .7, 1, 1.5 },
																{.3, .3, .3, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
	end
})

define_model('nazzle2_n', {
	info =	{
            lod_pixels = {10, 20, 50, 0},
			bounding_radius =1,
			materials = {'naz_no'},
			},
	static = function(lod)
	    if lod > 1 then
			texture('naz_n.png', v(0,.7,0), v(.1,0,0), v(0,-.4,0))
		else
			texture('naz_ns.png', v(0,.7,0), v(.1,0,0), v(0,-.4,0))
		end
		sphere_slice(6,2*lod, 0, 0.5*math.pi, Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(1.1547,1.7,1.1547)))
		tube(6,v(0,0,0), v(0,.75,0), v(0,0,1), .85, 1)
        set_material('naz_no', 	.45, .5, .7)
		use_material('naz_no')
		if lod > 1 then
			texture('naz_1.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		else
		    texture('naz_1s.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		end
		circle(6, v(0,.001,0), v(0,1,0), v(0,0,1), .9)
	end
})