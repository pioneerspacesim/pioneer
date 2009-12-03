
function nosewheel_info()
	return {
		lod_pixels={5,50,0},
		bounding_radius = 7,
		materials={'leg','tyre'}
	}
end
function nosewheel_static(lod)
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

function nosewheelunit_info()
	return {
		bounding_radius = 7,
		materials={'inside', 'matvar2'}
	}
end
function nosewheelunit_static(lod)
	set_material('inside', .2,.2,.2,1, 0,0,0, 1, 0,0,0)
end
function nosewheelunit_dynamic(lod)
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
	local flap_ang = 0.5*math.pi*math.clamp(3*get_arg(0),0,1)
	local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_arg(0)-0.34), 0, 1)
	local vrot = 1.5*v(-math.cos(flap_ang), math.sin(flap_ang), 0)
	xref_quad(v7, v6, v6+vrot, v7+vrot)
	xref_quad(v7, v7+vrot, v6+vrot, v6)

	call_model('nosewheel', v(0,0,0), v(1,0,0),
	v(0,math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
	zbias(0)
end

function mainwheel_info()
	return {
		lod_pixels = {5,50,0},
		bounding_radius = 8,
		materials = {'leg', 'tyre'}
	}
end
function mainwheel_static(lod)
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


function mainwheelunit_info()
	return {
		bounding_radius = 7,
		materials={'inside','matvar2'}
	}
end
function mainwheelunit_static(lod)
	set_material('inside', .2,.2,.2,1, 0,0,0, 1, 0,0,0)
end
function mainwheelunit_dynamic(lod)
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
	local flap_ang = 0.5*math.pi*math.clamp(3*get_arg(0),0,1)
	local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_arg(0)-0.34), 0, 1)
	local vrot = 1.5*v(-math.cos(flap_ang), math.sin(flap_ang), 0)
	xref_quad(v7, v6, v6+vrot, v7+vrot)
	xref_quad(v7, v7+vrot, v6+vrot, v6)

	call_model('mainwheel', v(0,0,0), v(1,0,0),
	v(0,math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
	zbias(0)
end

function ladybird_info()
	return {
		lod_pixels = {50,100,200,0},
		bounding_radius = 35,
		materials={'white','engines','matvar0', 'matvar2',
		'engine_inside'}
	}
end

function ladybird_static(lod)

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
	quad(v06,v08,v09,v07)
	quad(v09,v08,v11,v29)
	xref_tri(v08,v06,v10)

	local divs = lod*2

	local wingtip_rear = v(30,-5,10)
	local cpoint_rear = v(20,4,10)

	local leadingedge_mid = v(24,-5,-3)
	local tmp = v(5,0,5)
	local cpoint_leadingedge1 = leadingedge_mid - tmp
	local cpoint_leadingedge2 = leadingedge_mid + tmp


	-- body flat side piece
	local normal = ((v29-v09):cross(v30-v09)):norm()
	local cpoint_bodycurve = 0.5*(v29+v30) + 3.0*(v29-v30):cross(normal):norm()
	xref_flat(divs, normal,
		{ v09 },
		{ v29 },
		{ cpoint_bodycurve, v30 }
		)

	-- top wing bulge
	xref_bezier_3x3(divs,divs,
		wingtip_rear, cpoint_leadingedge2, leadingedge_mid,
		cpoint_rear, v(17,5,0), cpoint_leadingedge1,
		v29, cpoint_bodycurve, v30)


	-- rear
	xref_flat(divs, v(0,0,1),
		{ wingtip_rear },
		{ cpoint_rear, v29 },
		{ v32 }
	)
	quad(v29,v11,v31,v32) -- rear
	use_material('matvar2')
	quad(v10,v06,v07,v30)
	quad(v32,v31,v10,v30)
	-- underside of wing
	xref_tri(v30, wingtip_rear, v32)
	-- wing leading edge underside
	xref_flat(divs, v(0,-1,0),
		{ v30 },
		{ cpoint_leadingedge1, leadingedge_mid },
		{ cpoint_leadingedge2, wingtip_rear }
	)

	zbias(1, v33, v(0,0,1))
	set_material('engines',.3,.3,.3,1,.3,.3,.3,20)
	use_material('engines')
	xref_tube(4*lod, v33, v34, v(0,1,0), 2.5, 3.0)
	use_material('engine_inside')
	-- matanim!!
	xref_circle(4*lod, v33, v(0,0,1), v(0,1,0), 2.5)
	-- wheels my friend
	
end
function ladybird_dynamic(lod)
	set_material('matvar0', get_arg_material(0))
	set_material('matvar2', get_arg_material(2))
	set_material('engine_inside', lerp_materials(get_arg(2)*30.0, {0, 0, 0, 1, 0, 0, 0, 10, .5, .5, 1 },
				{0, 0, 0, 1, 0, 0, 0, 10, 0, 0, .5 }))
	if get_arg(0) ~= 0 then
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

function __walruswing_info()
	return {
		scale = 25,
		bounding_radius = 2.0,
		materials = {'matvar0'}
	}
end
function __walruswing_static(lod)
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
	xref_bezier_3x3(1,16, v07, 0.5*(v07+v09), v09,
			0.5*(v06+v07)+bend, bend, 0.5*(v08+v09)+bend,
			v06, 0.5*(v06+v08), v08)
	flat(16, v(0,1,0), { 0.5*(v08+v09)+bend, v09 },
			{ 0.5*(v08+v09)-bend, v08 })

end
function __walruswing_dynamic(lod)
	set_material('matvar0', get_arg_material(0))
end
function walrus_info()
	return {
		scale = 1.0,
		bounding_radius = 70,
		materials = {'matvar0', 'text'}
	}
end
function walrus_static(lod)

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


	local v20 = v(0.0, 10.0, 0.0)
			-- 20, top wing
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
end
function walrus_dynamic(lod)
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
	local reg = get_arg_string(0)
	zbias(1, v16, v54)
	text(reg, v16, v54, v(0,0,-1), 10.0, {xoffset=1, yoffset=.3})
	zbias(1, v10, v55)
	text(reg, v10, v55, v(0,0,1), 10.0, {xoffset=.8, yoffset=.3})
	if get_arg(0) > 0 then
		zbias(1, v40, v(0,-1,0))
		call_model('nosewheelunit', v40, v(-1,0,0), v(0,-1,0), 2.0)
		call_model('mainwheelunit', v41, v(-1,0,0), v(0,-1,0), 2.0)
	end
	zbias(0)
	local ang = math.pi - 0.5 + 0.5*get_arg(0)
	local xaxis = v(math.sin(ang), math.cos(ang), 0)
	call_model('__walruswing', v23, xaxis, v(0,0,-1):cross(xaxis), 1.0)
	ang = 0.5 - 0.5*get_arg(0)
	local xaxis = v(math.sin(ang), math.cos(ang), 0)
	call_model('__walruswing', v26, xaxis, v(0,0,-1):cross(xaxis), 1.0)
end

function bigtrader_info()
	return {
		scale=2.0,
		lod_pixels = {25,50,0},
		bounding_radius = 100,
		materials = {'matvar0','gray','text','engine_inside'}
	}
end
function bigtrader_static(lod)
	local v06 = v(4.0, -3.0, -35.0)
--} },			// 6, nose vertices
	local v07 = v(-4.0, -3.0, -35.0)
--} },
	local v08 = v(-1.0, -7.0, -32.0)
--} },		
	local v09 = v(1.0, -7.0, -32.0)
--} },

	local v10 = v(6.0, 8.0, -20.0)
--} },			// 10, nose section back
	local v11 = v(-6.0, 8.0, -20.0)
--} },				// and extrusion area
	local v12 = v(-10.0, 4.0, -20.0)
--} },			
	local v13 = v(-10.0, -4.0, -20.0)
--} },			
	local v14 = v(-6.0, -8.0, -20.0)
--} },
	local v15 = v(6.0, -8.0, -20.0)
--} },
	local v16 = v(10.0, -4.0, -20.0)
--} },			
	local v17 = v(10.0, 4.0, -20.0)
--} },

	-- midpoints
	local v18 = v(0.0, 0.0, -20.0)
--} },			// 18
	local v19 = v(0.0, 0.0, -16.0)
--} },			// 
	local v20 = v(0.0, 0.0, 4.0)
--} },			// 
	local v21 = v(0.0, 0.0, 8.0)
--} },			// 
	local v22 = v(0.0, 0.0, 26.0)
--} },		// 

	local v23 = v(-0.3826834, 0.9238795, 0.0)
--} },		// 23, tube norm

	local v24 = v(-12.5, 2.0, 10.0)
--} },			// 24, top engine
	local v25 = v(-12.5, 2.0, 30.0)
--} },
	local v26 = v(-12.5, 2.0, 13.0)
--} },
	local v27 = v(-12.5, 2.0, 27.0)
--} },

	local v28 = v(-12.0, -5.5, 10.0)
--} },			// 28, bottom engine
	local v29 = v(-12.0, -5.5, 30.0)
--} },
	local v30 = v(-12.0, -5.5, 13.0)
--} },
	local v31 = v(-12.0, -5.5, 27.0)
--} },

	local v32 = v(-10.0, -4.0, -16.0)
--} },			// 32, right text pos
	local v33 = v(10.0, -4.0, 4.0)
--} },			// left text pos

	
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
end
function bigtrader_dynamic(lod)
	set_material('matvar0', get_arg_material(0))
	set_material('engine_inside', lerp_materials(get_arg(2)*30.0, {0, 0, 0, 1, 0, 0, 0, 10, .5, .5, 1 },
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
	local leftText = v(-10.0, 0, -6.4)
	local rightText = v(10.0, 0, -6.4)
	local reg = get_arg_string(0)
	use_material('text')
	zbias(1, leftText, v(-1,0,0))
	text(reg, leftText, v(-1,0,0), v(0,0,1), 4, {center=true})
	zbias(1, rightText, v(1,0,0))
	text(reg, rightText, v(1,0,0), v(0,0,-1), 4, {center=true})

	if get_arg(0) > 0 then
		zbias(1, v34, v(0,-1,0))
		call_model('mainwheelunit', v34, v(-1,0,0), v(0,-1,0), .6)
		call_model('mainwheelunit', v35, v(-1,0,0), v(0,-1,0), .6)
		call_model('mainwheelunit', v36, v(-1,0,0), v(0,-1,0), .5)
		call_model('mainwheelunit', v37, v(-1,0,0), v(0,-1,0), .5)
		call_model('mainwheelunit', v38, v(-1,0,0), v(0,-1,0), .5)
		call_model('mainwheelunit', v39, v(-1,0,0), v(0,-1,0), .5)
	end
	--[[
	PTYPE_ZBIAS, 40, 1, 1,
	PTYPE_SUBOBJECT, 0x8000, SUB_DISH, 40, 1, 100, 200,

	PTYPE_ZBIAS, 0x8000, 0, 0,
	--]]
	zbias(0)
end

function radar_dish_info()
	return {
		lod_pixels = {0},
		bounding_radius = 2,
		materials = {'blah'}
	}
end
function radar_dish_static(lod)

end

register_models('nosewheel', 'nosewheelunit', 'mainwheel',
'mainwheelunit', 'ladybird', '__walruswing', 'walrus', 'bigtrader',
'radar_dish')
