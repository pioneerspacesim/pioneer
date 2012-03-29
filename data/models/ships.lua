define_model('missile', {
	info = {
		bounding_radius = 4,
		materials={ 'body' },
		tags = {'missile'},
	},
	static = function(lod)
		set_material('body', 1,1,1,1)
		use_material('body')
		cylinder(8, v(0,0,1), v(0,0,-3), v(0,1,0), .25)
		thruster(v(0,0,1), v(0,0,1), 10, true)
	end
})



define_model('nosewheel', {
	info = {
		lod_pixels={5,15,30,0},
		bounding_radius = 7,
		materials={'leg','tyre'}
	},
	static = function(lod)
		set_material('leg', .5, .5, .5, 1, .5, .5, .5, 2.0, 0, 0, 0)
		set_material('tyre', .3, .3, .3, 1, 0,0,0, 1, 0, 0, 0)
		use_material('leg')
		local v6 = v(0, 0, 0)
		local v7 = v(0, 3, 0)
		local v8 = v(0, 5, 0)
		local divs = lod*4
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		use_material('tyre')
		xref_cylinder(divs, v(.5,5,0), v(1,5,0), v(0,0,1), 1.0)
	end
})

define_model('nosewheelunit', {
	info = {
		bounding_radius = 9,
		materials={'inside', 'matvar2'}
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1, 0,0,0)
	end,
	dynamic = function(lod)
		-- flaps
		local v6 = v(1.5, 0, 6)
		local v7 = v(1.5, 0, -1)
		local v8 = v(0, 0, 6)
		local v9 = v(0, 0, -1)
		set_material('matvar2', get_arg_material(2))

		use_material('inside')
		zbias(1, v(0,0,0), v(0,1,0))
		-- flap internal
		xref_quad(v8, v6, v7, v9)
		-- SHould use parameter material(2) here but param materials not done yet
		use_material('matvar2')
		local flap_ang = 0.5*math.pi*math.clamp(3*get_animation_position('WHEEL_STATE'),0,1)
		local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_animation_position('WHEEL_STATE')-0.34), 0, 1)
		local vrot = 1.5*v(-math.cos(flap_ang), math.sin(flap_ang), 0)
		xref_quad(v7, v6, v6+vrot, v7+vrot)
		xref_quad(v7, v7+vrot, v6+vrot, v6)

		call_model('nosewheel', v(0,0,0), v(1,0,0),
		v(0,math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
		zbias(0)
	end
})

define_model('mainwheel', {
	info = {
		lod_pixels = {5,15,30,0},
		bounding_radius = 8,
		materials = {'leg', 'tyre'}
	},
	static = function(lod)
		local v6 = v(0,0,0)
		local v7 = v(0,3,0)
		local v8 = v(0,5,0)
		-- crossbar
		local v13 = v(0, 5, 1.4)
		local v14 = v(0, 5, -1.4)
		local divs = 4*lod
		set_material('leg', .5,.5,.5,1, 1,1,1, 2, 0,0,0)
		use_material('leg')
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		cylinder(4, v13, v14, v(1,0,0), .5)
		set_material('tyre', .3,.3,.3,1, 0,0,0, 1, 0,0,0)
		use_material('tyre')
		xref_cylinder(divs, v(.5, 5, 1.1), v(1, 5, 1.1), v(0,0,1), 1)
		xref_cylinder(divs, v(.5, 5, -1.1), v(1, 5, -1.1), v(0,0,1), 1)
	end
})

define_model('mainwheelunit', {
	info = {
		bounding_radius = 9,
		materials={'inside','matvar2'}
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1, 0,0,0)
	end,
	dynamic = function(lod)
		-- flaps
		local v6 = v(1.5, 0, 6)
		local v7 = v(1.5, 0, -1)
		local v8 = v(0, 0, 6)
		local v9 = v(0, 0, -1)
		set_material('matvar2', get_arg_material(2))

		use_material('inside')
		zbias(1, v(0,0,0), v(0,1,0))
		-- flap internal
		xref_quad(v8, v6, v7, v9)
		-- SHould use parameter material(2) here but param materials not done yet
		use_material('matvar2')
		local flap_ang = 0.5*math.pi*math.clamp(3*get_animation_position('WHEEL_STATE'),0,1)
		local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_animation_position('WHEEL_STATE')-0.34), 0, 1)
		local vrot = 1.5*v(-math.cos(flap_ang), math.sin(flap_ang), 0)
		xref_quad(v7, v6, v6+vrot, v7+vrot)
		xref_quad(v7, v7+vrot, v6+vrot, v6)

		call_model('mainwheel', v(0,0,0), v(1,0,0),
		v(0,math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
		zbias(0)
	end
})

define_model('ladybird', {
	info = {
		scale = 0.8,
		lod_pixels = {50,100,200,0},
		bounding_radius = 35,
		materials={'white','engines','matvar0', 'matvar2',
		'engine_inside','text'},
		tags = {'ship'},
	},
	static = function(lod)

		local v06 = v(-4.0, -5.0, -20.0);

		local v07 = v(4.0, -5.0, -20.0);
		local v08 = v(-6.0, 4.0, -10.0);

		local v09 = v(6.0, 4.0, -10.0);
		local v10 = v(-14.0, -5.0, -10.0);

		local v11 = v(-10.0, 5.0, 10.0);

		local v29 = v(10.0, 5.0, 10.0);
		local v30 = v(14.0, -5.0, -10.0);
		local v31 = v(-10.0, -5.0, 10.0);
		local v32 = v(10.0, -5.0, 10.0);

		local v33 = v(-12.0, 0.0, 10.0);
		local v34 = v(-12.0, 0.0, 13.0);

		--// thruster jets

		local v38 = v(-12.0, 0.0, 13.0);
		local v39 = v(-15.0, -3.0, -9.0);
		local v40 = v(-30.0, -4.0, 9.0);
		local v41 = v(-29.0, -5.5, 9.0);
		local v42 = v(-29.0, -4.0, 9.0);
		local v43 = v(-10.0, 0.0, -11.0);
		xref_thruster(v38, v(0,0,1), 50, true)
		xref_thruster(v43, v(0,0,-1), 25)

		set_material('white',.5,.5,.5,1,1,1,1,100)
		use_material('matvar0')
		-- matvar(0)
		texture('ships/default_ship_textures/lbfront_l.png', v(.5,.445,0), v(.085,0,0), v(0,.111,0))
		quad(v06,v08,v09,v07)
		texture('ships/default_ship_textures/lbtop_l.png', v(.5,.5,0), v(.05,0,0), v(0,0,1))
		quad(v09,v08,v11,v29)
		texture('ships/default_ship_textures/lbside_l.png', v(1,.5,0), v(0,0,1), v(0,.1,0))
		xref_tri(v08,v06,v10)
		local divs = lod*2
		local wingtip_rear = v(30,-5,10)
		local cpoint_rear = v(20,4,10)
		local leadingedge_mid = v(24,-5,-3)
		local tmp = v(5,0,5)
		local cpoint_leadingedge1 = leadingedge_mid - tmp
		local cpoint_leadingedge2 = leadingedge_mid + tmp
		-- body flat side piece
		texture('ships/default_ship_textures/lbside_l.png', v(.5,.5,0), v(0,0,.5), v(0,.1,0))
		local normal = ((v29-v09):cross(v30-v09)):norm()
		local cpoint_bodycurve = 0.5*(v29+v30) + 3.0*(v29-v30):cross(normal):norm()
		xref_flat(divs, normal,
		{ v09 },
		{ v29 },
		{ cpoint_bodycurve, v30 }
		)
		-- top wing bulge
		texture('ships/default_ship_textures/lbwing_l.png', v(.5,.5,0), v(.8,0,0), v(0,0,.8))
		xref_quadric_bezier_quad(divs,divs,
		wingtip_rear, cpoint_leadingedge2, leadingedge_mid,
		cpoint_rear, v(17,5,0), cpoint_leadingedge1,
		v29, cpoint_bodycurve, v30)

		-- rear
		texture('ships/default_ship_textures/lbrear_l.png', v(.5,.5,0), v(.017,0,0), v(0,.017,0))
		xref_flat(divs, v(0,0,1),
		{ wingtip_rear },
		{ cpoint_rear, v29 },
		{ v32 }
		)
		quad(v29,v11,v31,v32) 
		use_material('matvar2')
		texture('ships/default_ship_textures/lbrear_l.png', v(.5,.4,0), v(.0165,0,0), v(0,0,1.2))
		quad(v10,v06,v07,v30)
		texture('ships/default_ship_textures/lbrear_l.png', v(.5,.8,0), v(.0165,0,0), v(0,0,1.2))
		quad(v32,v31,v10,v30)
		-- underside of wing
		xref_tri(v30, wingtip_rear, v32)
		-- wing leading edge underside
		texture('ships/default_ship_textures/lbrear_l.png', v(.5,.31,0), v(.0165,0,0), v(0,0,.6))
		xref_flat(divs, v(0,-1,0),
		{ v30 },
		{ cpoint_leadingedge1, leadingedge_mid },
		{ cpoint_leadingedge2, wingtip_rear }
		)
		texture('ships/default_ship_textures/lbwing_l.png', v(.5,.5,0), v(.1,0,0), v(0,0,15))	
		zbias(1, v33, v(0,0,1))
		set_material('engines',.3,.3,.3,1,.3,.3,.3,20)
		use_material('engines')
		xref_tube(4*lod, v33, v34, v(0,1,0), 2.5, 3.0)
		use_material('engine_inside')
		-- matanim!!
		xref_circle(4*lod, v33, v(0,0,1), v(0,1,0), 2.5)
		-- wheels my friend
	end,
	dynamic = function(lod)
		set_material('matvar0', get_arg_material(0))
		set_material('matvar2', get_arg_material(2))
		set_material('engine_inside', lerp_materials(get_time('MINUTES')*30.0, {0, 0, 0, 1, 0, 0, 0, 10, .5, .5, 1 },
		{0, 0, 0, 1, 0, 0, 0, 10, 0, 0, .5 }))
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		if lod > 1 then
			use_material('text')
			local label = get_label()
			zbias(1,v(8.9,3.6,.07),v(1.8,1,1))
			text(label,v(8.9,3.6,.07),v(1.8,1,1),v(-.25,0,-1),1.5, {center=true})
			zbias(1,v(-8.9,3.6,.07),v(1.8,1,1))
			text(label,v(-8.9,3.6,.07),v(-1.8,1,1),v(-.25,0,1),1.5, {center=true})
			zbias(0)
		end
		if get_animation_position('WHEEL_STATE') ~= 0 then
			local v35 = v(0.0, -5.0, -13.0);
			local v36 = v(-15.0, -5.0, 3.0);
			local v37 = v(15.0, -5.0, 3.0);

			zbias(1, v35, v(0,-1,0))
			call_model('nosewheelunit', v35, v(-1,0,0), v(0,-1,0), 1)
			call_model('mainwheelunit', v36, v(-1,0,0), v(0,-1,0), 1)
			call_model('mainwheelunit', v37, v(-1,0,0), v(0,-1,0), 1)
			zbias(0)
		end
	end
})


define_model('__walruswing', {
	info = {
		lod_pixels = { 20, 50, 0 },
		scale = 25,
		bounding_radius = 2.0,
		materials = {'matvar0'}
	},
	static = function(lod)
		-- bottom front
		local v06 = v(0.0, 0.0, 1.0)
		-- bottom back
		local v07 = v(0.0, 0.0, -1.0)
		-- top front
		local v08 = v(0.0, 1.5, 0.0)
		-- top back
		local v09 = v(0.0, 1.5, -1.5)
		use_material('matvar0')
		local bend = v(0.175,0,0)
		xref_quadric_bezier_quad(1,lod*4, v07, 0.5*(v07+v09), v09,
		0.5*(v06+v07)+bend, bend, 0.5*(v08+v09)+bend,
		v06, 0.5*(v06+v08), v08)
		flat(lod*4, v(0,1,0), { 0.5*(v08+v09)+bend, v09 },
		{ 0.5*(v08+v09)-bend, v08 })

	end,
	dynamic = function(lod)
		set_material('matvar0', get_arg_material(0))
	end
})

define_model('walrus', {
	info = {
		scale = 0.5,
		bounding_radius = 70,
		materials = {'matvar0', 'text'},
		lod_pixels = {25, 50, 0},
		tags = { 'ship' },
	},
	static = function()
		local v06 = v(-5.0, 10.0, -30.0)
		-- 6, top four body verts
		local v07 = v(5.0, 10.0, -30.0)
		local v08 = v(-5.0, 10.0, 30.0)
		local v09 = v(5.0, 10.0, 30.0)

		local v10 = v(-11.16025, -0.6698729, -25.0)
		-- 10, right four body verts
		local v11 = v(-6.160254, -9.330127, -35.0)
		local v12 = v(-11.16025, -0.6698729, 35.0)
		local v13 = v(-6.160254, -9.330127, 30.0)

		local v14 = v(11.16025, -0.6698729, -25.0)
		-- 14, left four body verts
		local v15 = v(6.160254, -9.330127, -35.0)
		local v16 = v(11.16025, -0.6698729, 35.0)
		local v17 = v(6.160254, -9.330127, 30.0)

		local v18 = v(-5.0, -0.6698729, -60.0)
		-- 18, front two verts
		local v19 = v(5.0, -0.6698729, -60.0)


		-- 20, top wing
		local v20 = v(0.0, 10.0, 0.0)
		local v21 = v(-1.0, 0.0, 0.0)

		local v22 = v(0.0, 1.0, 0.0)

		-- 23, right wing
		local v24 = v(0.5, -0.8660254, 0.0)
		local v25 = v(-0.8660254, -0.5, 0.0)

		-- 26, left wing
		local v27 = v(0.5, 0.8660254, 0.0)
		local v28 = v(0.8660254, -0.5, 0.0)

		local v29 = v(-0.0, 0.0, 40.0)
		-- 29, main thruster
		local v30 = v(-11.0, 0.0, -35.0)
		-- 30, retro
		local v31 = v(11.0, 0.0, -35.0)

		local v32 = v(-9.0, 5.0, -30.0)
		-- 32, right
		local v33 = v(-12.0, -5.0, 30.0)
		local v34 = v(12.0, -5.0, -30.0)
		-- 34, left
		local v35 = v(9.0, 5.0, 30.0)
		local v36 = v(0.0, 12.0, -30.0)
		-- 36, top
		local v37 = v(0.0, 12.0, 30.0)
		local v38 = v(0.0, -12.0, -30.0)
		-- 38, bottom
		local v39 = v(0.0, -12.0, 30.0)


		local v42 = v(-5.0, 10.0, -30.0)
		-- 6, top four body verts
		local v43 = v(-11.16025, -0.6698729, 35.0)

		use_material('matvar0')
		quad(v07, v06, v08, v09)
		quad(v13, v11, v15, v17)
		xref_quad(v08, v06, v10, v12)
		xref_quad(v12, v10, v11, v13)
		quad(v09, v08, v12, v16)
		quad(v16, v12, v13, v17)

		quad(v06, v07, v19, v18)
		quad(v18, v19, v15, v11)
		xref_tri(v06, v18, v10)
		xref_tri(v10, v18, v11)

		thruster(v29, v(0,0,1), 50, true)
		thruster(v30, v(0,0,-1), 35, true)
		thruster(v31, v(0,0,-1), 35, true)
		thruster(v32, v(-1,0,0), 25)
		thruster(v33, v(-1,0,0), 25)
		thruster(v34, v(1,0,0), 25)
		thruster(v35, v(1,0,0), 25)
		thruster(v36, v(0,1,0), 25)
		thruster(v37, v(0,1,0), 25)
		thruster(v38, v(0,-1,0), 25)
		thruster(v39, v(0,-1,0), 25)

		call_model('__walruswing', v20, v(-1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		local v06 = v(-5.0, 10.0, -30.0)
		local v07 = v(5.0, 10.0, -30.0)
		local v08 = v(-5.0, 10.0, 30.0)
		local v10 = v(-11.16025, -0.6698729, -25.0)
		local v12 = v(-11.16025, -0.6698729, 35.0)
		local v14 = v(11.16025, -0.6698729, -25.0)
		local v16 = v(11.16025, -0.6698729, 35.0)
		local v20 = v(0.0, 10.0, 0.0)
		local v23 = v(-8.660254, -5.0, 0.0)
		local v26 = v(8.660254, -5.0, 0.0)

		local v40 = v(0.0, -9.330127, -30.0)
		-- 40, nosewheel
		local v41 = v(0.0, -9.330127, 13.0)
		-- 41, mainwheel
		local v54 = (v07 - v14):cross(v16 - v14):norm()
		local v55 = (v06 - v08):cross(v12 - v08):norm()

		set_material('matvar0', get_arg_material(0))
		set_material('text', .2,.2,.2,1)
		use_material('text')
		if lod > 1 then
			local reg = get_label()
			zbias(1, v16, v54)
			text(reg, v16, v54, v(0,0,-1), 10.0, {xoffset=1, yoffset=.3})
			zbias(1, v10, v55)
			text(reg, v10, v55, v(0,0,1), 10.0, {xoffset=.8, yoffset=.3})
		end
		if get_animation_position('WHEEL_STATE') > 0 then
			zbias(1, v40, v(0,-1,0))
			call_model('nosewheelunit', v40, v(-1,0,0), v(0,-1,0), 2.0)
			call_model('mainwheelunit', v41, v(-1,0,0), v(0,-1,0), 2.0)
		end
		zbias(0)
		local ang = math.pi - 0.5 + 0.5*get_animation_position('WHEEL_STATE')
		local xaxis = v(math.sin(ang), math.cos(ang), 0)
		call_model('__walruswing', v23, xaxis, v(0,0,-1):cross(xaxis), 1.0)
		ang = 0.5 - 0.5*get_animation_position('WHEEL_STATE')
		local xaxis = v(math.sin(ang), math.cos(ang), 0)
		call_model('__walruswing', v26, xaxis, v(0,0,-1):cross(xaxis), 1.0)
	end
})

define_model('flowerfairy', {
	info = {
		scale=1.3,
		lod_pixels = {25,50,0},
		bounding_radius = 60,
		materials = {'matvar0','gray','text','engine_inside'},
		tags = {'ship'},
	},
	static = function(lod)
		--	// 6, nose vertices
		local v06 = v(4.0, -3.0, -35.0)
		local v07 = v(-4.0, -3.0, -35.0)
		local v08 = v(-1.0, -7.0, -32.0)
		local v09 = v(1.0, -7.0, -32.0)

		--	// 10, nose section back
		local v10 = v(6.0, 8.0, -20.0)
		local v11 = v(-6.0, 8.0, -20.0)
		local v12 = v(-10.0, 4.0, -20.0)
		local v13 = v(-10.0, -4.0, -20.0)
		local v14 = v(-6.0, -8.0, -20.0)
		local v15 = v(6.0, -8.0, -20.0)
		local v16 = v(10.0, -4.0, -20.0)
		local v17 = v(10.0, 4.0, -20.0)

		-- midpoints
		local v18 = v(0.0, 0.0, -20.0)
		local v19 = v(0.0, 0.0, -16.0)
		local v20 = v(0.0, 0.0, 4.0)
		local v21 = v(0.0, 0.0, 8.0)
		local v22 = v(0.0, 0.0, 26.0)

		--	// 24, top engine
		local v24 = v(-12.5, 2.0, 10.0)
		local v25 = v(-12.5, 2.0, 30.0)
		local v26 = v(-12.5, 2.0, 13.0)
		local v27 = v(-12.5, 2.0, 27.0)

		--	// 28, bottom engine
		local v28 = v(-12.0, -5.5, 10.0)
		local v29 = v(-12.0, -5.5, 30.0)
		local v30 = v(-12.0, -5.5, 13.0)
		local v31 = v(-12.0, -5.5, 27.0)

		-- 41, extrusion as at 10 but rev order
		local v41 = v(6.0, 8.0, -20.0)
		local v42 = v(-6.0, 8.0, -20.0)
		local v43 = v(-10.0, 4.0, -20.0)
		local v44 = v(-10.0, -4.0, -20.0)
		local v45 = v(-6.0, -8.0, -20.0)
		local v46 = v(6.0, -8.0, -20.0)
		local v47 = v(10.0, -4.0, -20.0)
		local v48 = v(10.0, 4.0, -20.0)

		set_material('text', .20, .20, .20, 1)
		use_material('matvar0')
		quad(v06,v07,v11,v10)
		xref_tri(v07,v12,v11)
		xref_tri(v07,v13,v12)
		xref_tri(v07,v14,v13)
		xref_tri(v07,v08,v14)
		quad(v07,v06,v09,v08)
		quad(v08,v09,v15,v14)
		quad(v10,v11,v14,v15)
		xref_quad(v11,v12,v13,v14)
		extrusion(v19, v20, v(0,1,0), 1.0,
		v41, v42, v43, v44, v45,v46,v47,v48)
		extrusion(v21, v22, v(0,1,0), 1.0,
		v41, v42, v43, v44, v45,v46,v47,v48)
		xref_tube(8, v24, v25, v(0,1,0), 2.0, 2.5)
		xref_tube(8, v28, v29, v(0,1,0), 2.0, 2.5)
		use_material('engine_inside')
		xref_circle(8, v26, v(0,0,-1), v(0,1,0), 2)
		xref_circle(8, v27, v(0,0,1), v(0,1,0), 2)
		xref_circle(8, v30, v(0,0,-1), v(0,1,0), 2)
		xref_circle(8, v31, v(0,0,1), v(0,1,0), 2)
		set_material('gray', .30, .30, .30,1, .10, .10, .10, 10)
		use_material('gray')
		extrusion(v18, v19, v(0,1,0), .85,
		v41, v42, v43, v44, v45,v46,v47,v48)
		extrusion(v20, v21, v(0,1,0), .85,
		v41, v42, v43, v44, v45,v46,v47,v48)

		xref_thruster(v25, v(0,0,1), 30, true)
		xref_thruster(v29, v(0,0,1), 30, true)
		xref_thruster(v24, v(0,0,-1), 20, true)
		xref_thruster(v28, v(0,0,-1), 20, true)
	end,
	dynamic = function(lod)
		set_material('matvar0', get_arg_material(0))
		set_material('engine_inside', lerp_materials(get_time('MINUTES')*30.0, {0, 0, 0, 1, 0, 0, 0, 10, .5, .5, 1 },
		{0, 0, 0, 1, 0, 0, 0, 10, 0, 0, .5 }))
		-- 34, gear pos
		local v34 = v(-5.0, -8.0, -13.0)
		local v35 = v(5.0, -8.0, -13.0)
		local v36 = v(-11.5, -8.0, 25.0)
		local v37 = v(11.5, -8.0, 25.0)
		local v38 = v(-11.5, -8.0, 13.0)
		local v39 = v(11.5, -8.0, 13.0)
		-- 40, dish pos
		local v40 = v(-0.05, 8.0, 15.0)
		if lod > 1 then
			local leftText = v(-10.0, 0, -6.4)
			local rightText = v(10.0, 0, -6.4)
			local reg = get_label()
			use_material('text')
			zbias(1, leftText, v(-1,0,0))
			text(reg, leftText, v(-1,0,0), v(0,0,1), 4, {center=true})
			zbias(1, rightText, v(1,0,0))
			text(reg, rightText, v(1,0,0), v(0,0,-1), 4, {center=true})
		end

		if get_animation_position('WHEEL_STATE') > 0 then
			zbias(1, v34, v(0,-1,0))
			call_model('mainwheelunit', v34, v(-1,0,0), v(0,-1,0), .6)
			call_model('mainwheelunit', v35, v(-1,0,0), v(0,-1,0), .6)
			call_model('mainwheelunit', v36, v(-1,0,0), v(0,-1,0), .5)
			call_model('mainwheelunit', v37, v(-1,0,0), v(0,-1,0), .5)
			call_model('mainwheelunit', v38, v(-1,0,0), v(0,-1,0), .5)
			call_model('mainwheelunit', v39, v(-1,0,0), v(0,-1,0), .5)
		end
		zbias(0)
	end
})

define_model('interdictor', {
	info = {
		scale = 1.2,
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 54,
		materials = {'matvar0', 'matvar2', 'engine', 'engine_inside', 'cockpit', 'text'},
		tags = {'ship'},
	},
	static = function(lod)
		local nose_tip = v(0.0, 0.0, -35.0)
		--f } },			// 6, nose point
		local v07 = norm(0.0, 1.0, -0.2)
		--} },			// nose normal
		local front_lateral = v(-6.0, 0.0, -18.0)
		--} },			// 8, r edge forward mid
		local v09 = norm(-0.2, 1.0, -0.1)
		--} },			// norm
		local v10 = v(-12.0, 0.0, 2.0)
		--} },		// 10, r edge back mid
		local v11 = norm(-0.2, 1.0, -0.1)
		--} },			// norm
		local v12 = v(-7.5, 0.0, 25.0)
		--} },		// 12, r edge back
		local v13 = norm(0.0, 1.0, 0.2)
		--} },			// norm
		local v14 = v(0.0, 3.0, -15.0)
		--} },			// 14, cockpit front
		local v15 = norm(0.0, 1.0, 0.08)
		--} },		// norm
		local v16 = v(-1.5, 3.0, -13.5)
		--} },			// 16, cockpit right
		local v17 = norm(0.0, 1.0, 0.08)
		--} },		// norm
		local v18 = v(0.0, 3.0, -10.0)
		--} },			// 18, cockpit back
		local v19 = norm(0.0, 1.0, 0.08)
		--} },		// norm
		local v20 = v(1.5, 3.0, -13.5)
		--} },		// 20, cockpit left
		local v21 = norm(0.0, 1.0, 0.08)
		--} },		// norm

		local v22 = v(-6.0, 3.0, 5.0)
		--} },			// 22, inner right
		local v23 = norm(-0.2, 1.0, -0.2)
		--} },			// norm
		local v24 = v(0.0, 3.0, 5.0)
		--} },			// 24, inner mid
		local v25 = norm(0.2, 1.0, -0.2)
		--} },		// norm

		local v26 = v(-2.0, 2.0, -23.0)
		--} },			// 26, fwd midpoint
		local v27 = norm(0.0, 1.0, -0.1)
		--} },		// norm
		local v28 = v(-5.0, 2.5, -5.0)
		--} },			// 28, right midpoint
		local v29 = norm(-0.08, 1.0, -0.04)
		--} },		// norm
		local v30 = v(-7.0, 2.0, 14.0)
		--} },		// 30, rear right midpoint
		local v31 = norm(-0.04, 1.0, 0.1)
		--} },		// norm

		local v32 = v(-3.0, 3.0, -5.0)
		--} },			// 32, central midpoint
		local v33 = v(0.0, 4.0, -12.5)
		--} },			// 33, cockpit midpoint
		local v34 = v(-3.75, 4.0, 20.0)
		--} },		// 34, nacelle midpoint

		local v35 = v(-7.5, 0.0, 30.0)
		--} },		// 35, nacelle outer
		local v36 = v(0.0, 0.0, 30.0)
		--} },		// 36, nacelle inner

		-- edge tangents
		local v37 = v(6.0, 4.0, 3.0)
		--} },		// 37, edge to mid
		local v38 = v(6.0, 0.0, 3.0)
		local v39 = v(0.0, 4.0, -20.0)
		--} },			// 39, rear to mid
		local v40 = v(2.5, 0.0, -20.0)

		local v41 = v(0.0, 0.0, -20.0)
		--} },			// 41, mid to nose
		local v42 = v(0.0, -4.0, -20.0)
		local v43 = v(-6.0, 0.0, -3.0)
		--} },			// 43, mid to edge
		local v44 = v(-6.0, -4.0, -3.0)
		local v45 = v(-2.5, 0.0, 20.0)
		--} },			// 45, mid to rear
		local v46 = v(0.0, -4.0, 20.0)

		local v47 = v(-1.5, 0.0, 0.0)
		--} },			// 47, cockpit CW tangents
		local v48 = v(1.5, 0.0, 0.0)
		local v49 = v(0.0, 0.0, -1.5)
		local v50 = v(0.0, 0.0, 1.5)
		local v51 = v(0.0, 0.0, -3.5)
		local v52 = v(0.0, 0.0, 3.5)

		local v53 = v(-10.0, 0.0, 20.0)
		--} },			// 53, rear edge tangents
		local v54 = v(10.0, 0.0, 0.0)
		local v55 = v(4.0, 0.0, -10.0)
		--} },			// 55, CCW
		local v56 = v(-5.0, 0.0, -10.0)

		local v57 = v(0.0, 1.5, 0.0)
		--} },			// 57, nacelle tangents
		local v58 = v(0.0, -1.5, 0.0)
		local v59 = v(0.0, 0.0, -12.0)
		local v60 = v(0.0, 0.0, 12.0)

		local v61 = v(-3.75, 4.0, 30.0)
		--} },			// 61, nacelle rear midpoint
		local v62 = v(-3.0, 0.0, 0.0)
		--} },			// and tangents
		local v63 = v(4.0, 0.0, 0.0)
		--} },			// 

		-- underside points
		local v64 = v(-5.0, 0.0, -5.0)
		--} },			// 64, upper outer vent
		local v65 = v(0.0, 0.0, -5.0)
		--} },			// 65, upper inner vent
		local v66 = v(-5.0, -2.0, -3.0)
		--} },			// 66, lower outer vent
		local v67 = v(0.0, -2.0, -3.0)
		--} },			// 67, lower inner vent
		local v68 = v(-5.0, -2.0, 30.0)
		--} },		// 68, nacelle outer underside
		local v69 = v(0.0, -2.0, 30.0)
		--} },		// 69, nacelle inner underside
		local v70 = v(-13.0, 0.0, 14.0)
		--} },		// 70, rear underside centre
		local v71 = v(-7.5, 0.0, -3.0)
		--} },			// 71, vent outer edge 

		local v72 = v(-3.75, 0.7, 30.0)
		--} },			// 72, engine midpoint


		local v76 = v(-3.75, 0.7, 32.0)
		--} },			// 76, engine end

		local v77 = v(-4.5, -0.3, -4.7)
		--} },			// 77, retro vent
		local v78 = v(-0.5, -0.3, -4.7)
		local v79 = v(-4.5, -1.7, -3.3)
		local v80 = v(-0.5, -1.7, -3.3)

		use_material('matvar0')

		local lvl = lod * 4
		local c14_6_1 = vlerp(.75,nose_tip,v14)
		local c16_8_1 = v(-2.5, 3.0, -13.5)
		local j = v(-2,2.0,-21)
		nose_patch = {nose_tip, vlerp(.25,nose_tip,front_lateral), vlerp(.75,nose_tip,front_lateral), front_lateral,
		v(0.0,0.75,-30.0), v(-2,0.75,-30), v(-4,1.75,-21), v(-4.875,1.75,-16.875),
		v(0.0,2.25,-20.0), v(-2,2.25,-20), v(-1.875,2.8125,-15.125), v(-2.5,3,-13.5),
		v14,v(-0.375,3.0,-14.625),v(-1.125,3.0,-13.875),v16}
		-- top nose bit
		xref_cubic_bezier_quad(lvl, lvl, nose_patch)

		local c22_10_1 = v(-10.0, 3.0, 4.0)
		local c22_10_2 = v(-12.0, 1.0, 4.0)
		-- side thingies
		j = 0.25*(v10+v22+v16+front_lateral)
		side_patch = joined_patch(nose_patch, 1, { vlerp(.75, v16, v22), v(-9,3,0), v(-10,1,0.0), vlerp(.75,front_lateral,v10), v22, c22_10_1, c22_10_2, v10 })
		--local v10 = v(-12.0, 0.0, 2.0)

		xref_cubic_bezier_quad(lvl, lvl, side_patch)

		j = 0.333*(v12+v22+v10)+v(0,5,0)
		local wingtip = v(-14.1, 0, 12)

		top_wing_patch = joined_patch(side_patch, 2, { vlerp(.75,v22,v12),v(-9,1,15),v(-11,2,12),wingtip-v(0,0,5), v12, v12+v(-5,0,-6), wingtip+v(0,0,5), wingtip})
		xref_cubic_bezier_quad(lvl, lvl, top_wing_patch)

		-- top wings
		local engine_back_cp1 = v(-1,5.4,0)
		local engine_back_cp2 = v(1,5.4,0)
		local c1_1 = v(0, 1.0, 25.0)
		local j = 0.25*(c1_1 + v36 + v35 + v12)
		local c2_1 = v(0, 3.0, 5.0)
		-- front bit of top curve of engine
		front_top_engine_patch = {c2_1, c2_1+v(-2,0,-4), v22+v(2,0,-4), top_wing_patch[1],
		vlerp(.25,c2_1,c1_1), vlerp(.25,c2_1,c1_1)+v(0,1.5,0), vlerp(.25,v22,v12)+v(0,3,0), top_wing_patch[5],
		vlerp(.75,c2_1,c1_1), vlerp(.75,c2_1,c1_1)+0.7*engine_back_cp1, vlerp(.75,v22,v12)+0.7*engine_back_cp2, top_wing_patch[9],
		c1_1, c1_1+0.85*engine_back_cp1, v12+0.85*engine_back_cp2, top_wing_patch[13]}
		xref_cubic_bezier_quad(lvl, lvl, front_top_engine_patch)

		rear_top_engine_curve = joined_patch(front_top_engine_patch, 2, { vlerp(0.75,c1_1,v36), vlerp(0.75,c1_1,v36)+engine_back_cp1, vlerp(0.75,v12,v35)+engine_back_cp2, vlerp(0.75,v12,v35), v36, v36+v(-1,5.4,0), v35+v(1,5.4,0), v35})
		xref_cubic_bezier_quad(1, lvl, rear_top_engine_curve)

		-- flat bit where cockpit sits.
		xref_flat(lvl, v(0,1,0),
		{ v16 },
		{ side_patch[5], side_patch[9], v22 },
		{ v22+v(2,0,-4), c2_1+v(-2,0,-4), c2_1 },
		{ v14 })

		use_material('matvar2')
		-- Underside of wings
		xref_flat(lvl, v(0,-1,0),
		{ top_wing_patch[14], top_wing_patch[15], top_wing_patch[16] },
		{ top_wing_patch[12], top_wing_patch[8], top_wing_patch[4] },
		{ v71 }, { v12 })
		-- other underside
		xref_quad(front_lateral,nose_tip,v65,v64)
		xref_quad(front_lateral,v64,v71,v10)
		xref_quad(v64,v65,v67,v66)
		xref_tri(v71,v64,v66)
		xref_quad(v71,v66,v68,v12)
		xref_tri(v12,v68,v35)
		xref_quad(v66,v67,v69,v68)

		-- engine back face
		xref_flat(lvl, v(0,0,1),
		{ v36+engine_back_cp1, v35+engine_back_cp2, v35 }, 
		{ v68 }, { v69 }, { v36 })

		set_material('cockpit', .3,.3,.3,1, .3,.3,.3, 20)
		use_material('cockpit')
		quadric_bezier_quad(lvl, lvl,
		v18, 0.5*(v18+v20)+v(1,0,0), v20,
		0.5*(v16+v18)-v(1,0,0), v(0,6,-10), 0.5*(v20+v14),
		v16, 0.5*(v14+v16),v14)

		set_material('engine', .30, .30, .30,1, .30, .30, .30, 20)
		use_material('engine')
		xref_tube(lvl, v72, v76, v(0,1,0), 2.0, 2.5)

		if lod > 1 then
			zbias(1, v72, v(0,0,1))
			use_material('engine_inside')
			xref_circle(lvl, v72, v(0,0,1), v(0,1,0), 2.0)

			local retro_norm = ((v80-v78):cross(v77-v78)):norm()
			zbias(1, v77, retro_norm)
			xref_quad(v77, v78, v80, v79)
			zbias(0)
		end

		set_material('text', .2,.2,.2,1)

		-- main & retro thrusters
		local v82 = v(3.75, 0.7, 32.0)
		local v84 = v(2.5, -1.0, -5.0)

		-- vertical thrusters
		local v85 = v(-9.0, 1.5, 10.0)
		local v86 = v(-9.0, -0.5, 10.0)
		local v87 = v(9.0, 1.5, 10.0)
		local v88 = v(9.0, -0.5, 10.0)
		local v89 = v(0.0, 3.5, -8.0)
		local v90 = v(0.0, -0.5, -25.0)

		-- horizontal thrusters
		local v91 = v(-8.0, 0.0, 28.0)
		local v92 = v(8.0, 0.0, 28.0)
		local v93 = v(-3.5, 0.0, -25.0)
		local v94 = v(3.5, 0.0, -25.0)

		xref_thruster(v82, v(0,0,1), 30, true)
		xref_thruster(v84, v(0,0,-1), 20, true)
		thruster(v85, v(0,1,0), 15)
		thruster(v86, v(0,-1,0), 15)
		thruster(v87, v(0,1,0), 15)
		thruster(v88, v(0,-1,0), 15)
		thruster(v89, v(0,1,0), 15)
		thruster(v90, v(0,-1,0), 15)
		thruster(v91, v(-1,0,0), 15)
		thruster(v92, v(1,0,0), 15)
		thruster(v93, v(-1,0,0), 15)
		thruster(v94, v(1,0,0), 15)
	end,
	dynamic = function(lod)
		-- text norms
		local v95 = norm(-2.0, -2.5, 0.0)
		local v96 = norm(2.0, -2.5, 0.0)
		local v97 = v(5.0, -2.0, 13.5)
		local v98 = v(-5.0, -2.0, 13.5)

		set_material('matvar0', get_arg_material(0))
		set_material('matvar2', get_arg_material(2))
		set_material('engine_inside', lerp_materials(get_time('MINUTES')*30.0, {0, 0, 0, 1, 0, 0, 0, 10, .5, .5, 1 },
		{0, 0, 0, 1, 0, 0, 0, 10, 0, 0, .5 }))
		if lod > 1 then
			local shipname = get_label()
			use_material('text')
			zbias(1, v98, v95)
			text(shipname, v98, v95, v(0,0,1), 2.5, {center=true, yoffset=0.7})
			zbias(1, v97, v96)
			text(shipname, v97, v96, v(0,0,-1), 2.5, {center=true, yoffset=0.7})
			use_material('text')
		end

		if get_animation_position('WHEEL_STATE') ~= 0 then
			-- lights on wingtips
			local lightphase = math.fmod(get_time('SECONDS'), 1)
			if lightphase > .9 then
				billboard('smoke.png', 10, v(1,1,1), { v(-14.1, 0, 12) })
			elseif lightphase > .8 then
				billboard('smoke.png', 10, v(1,1,1), { v(14.1, 0, 12) })
			end
			-- wheels
			local v73 = v(0.0, 0.0, -15.0)
			local v74 = v(-3.75, -2.0, 15.0)
			local v75 = v(3.75, -2.0, 15.0)
			zbias(1, v73, v(0,-1,0))
			-- nose wheel
			call_model('nosewheelunit', v73, v(-1,0,0), v(0,-1,0), 1)
			zbias(1, v74, v(0,-1,0))
			-- rear wheels
			call_model('nosewheelunit', v74, v(-1,0,0), v(0,-1,0), .64)
			call_model('nosewheelunit', v75, v(-1,0,0), v(0,-1,0), .64)
			zbias(0)
		end
	end
})

--load_lua('models/mods')
