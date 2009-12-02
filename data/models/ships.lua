
function nosewheel_info()
	return {
		lod_pixels={20,75,0},
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
	v(0,math.sin(wheel_ang),-math.cos(wheel_ang)), 1.0)
	zbias(0)
end

function mainwheel_info()
	return {
		lod_pixels = {50,100,0},
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
	v(0,math.sin(wheel_ang),-math.cos(wheel_ang)), 1.0)
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

register_models('nosewheel', 'nosewheelunit', 'mainwheel',
'mainwheelunit', 'ladybird')
