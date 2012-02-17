define_model('adder_uc_cage', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 14.2,
		materials = {'metal'},
	},
	static = function(lod)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
		use_material('metal')
		texture('basic.png')
		load_obj('uc_cage.obj')
	end
})

define_model('adder_piston_f', {
	info = {
		lod_pixels = {1,5,10,0},
		bounding_radius = 8,
		materials = {'chrome'}
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('chrome')
		texture('steel.png')
		sphere_slice(3*lod,lod, 0, .75*math.pi,Matrix.rotate(math.pi,v(1,0,0))*Matrix.translate(v(0,-.3,0)))
	end,
	dynamic = function(lod)
		local trans = 3*math.clamp((get_animation_position('WHEEL_STATE')),.2,1)
		use_material('chrome')
		--texture('models/ships/adder/steel.png')
		ring(3*lod,v(0,0,0),v(0,-.3-trans,0),v(1,0,0),.3)
		ring(3*lod,v(0,0,0),v(0,-.2-2*trans,0),v(1,0,0),.25)
		ring(3*lod,v(0,0,0),v(0,-.1-3*trans,0),v(1,0,0),.2)
	end
})

define_model('adder_piston_r', {
	info = {
		lod_pixels = {1,5,10,0},
		bounding_radius = 8,
		materials = {'chrome'}
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('chrome')
		texture('steel.png')
		sphere_slice(3*lod,lod, 0, .75*math.pi,Matrix.rotate(math.pi,v(1,0,0))*Matrix.translate(v(0,-.3,0)))
	end,
	dynamic = function(lod)
		local trans = 2.45*math.clamp((get_animation_position('WHEEL_STATE')),.2,1)
		use_material('chrome')
		--texture('models/ships/adder/steel.png')
		ring(3*lod,v(0,0,0),v(0,-.3-trans,0),v(1,0,0),.3)
		ring(3*lod,v(0,0,0),v(0,-.2-2*trans,0),v(1,0,0),.25)
		ring(3*lod,v(0,0,0),v(0,-.05-3*trans,0),v(1,0,0),.2)
	end
})

define_model('adder_f_wheel', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 1,
		materials = {'tire'},
	},
	static = function(lod)
		set_material('tire', .25,.3,.25,1,.3,.3,.3,5)

		texture('tire.png')
		load_obj('f_w_centre.obj')
		use_material('tire')
		load_obj('f_wheel.obj')
	end
})

define_model('adder_uc_f', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 3.3,
		materials = {'non_cv'},
	},
	static = function(lod)
		set_material('non_cv', .45,.55,.6,1,.5,.5,.6,50)
		use_material('non_cv')
		texture('steel.png')
		load_obj('f_leg.obj')
	end,
	dynamic = function(lod)
		local w_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		call_model('adder_f_wheel',v(-0.35,0,2.693),v(1,0,0),v(0,math.sin(w_rot),math.cos(w_rot)-0.3),1)
	end
})

define_model('adder_r_wheel', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 5,
		materials = {'tire'},
	},
	static = function(lod)
		set_material('tire', .25,.3,.25,1,.3,.3,.3,5)
		texture('steel.png')
		load_obj('r_yoke.obj')
		texture('tire.png')
		load_obj('r_w_centre.obj')
		use_material('tire')
		load_obj('r_wheels.obj')
	end
})

define_model('adder_uc_r', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 6.1,
		materials = {'non_cv'},
	},
	static = function(lod)
		set_material('non_cv', .45,.55,.6,1,.5,.5,.6,50)
		use_material('non_cv')
		texture('steel.png')
		load_obj('r_leg.obj')
	end,
	dynamic = function(lod)
		local w_rot = 0.525*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		call_model('adder_r_wheel',v(0,.227,2.353),v(1,0,0),v(0,math.cos(w_rot),-math.sin(w_rot)+0.3),1)
	end
})


define_model('adder_f_flap', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 2.2,
	},
	static = function(lod)
		texture('f_flap0.png')
		load_obj('f_flap.obj',Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('adder_rl_flap1', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 2.2,
	},
	static = function(lod)
		texture('rl_flap0.png')
		load_obj('rl_flap1.obj',Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('adder_rl_flap2', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 2.2,
	},
	static = function(lod)
		texture('rl_flap0.png')
		load_obj('rl_flap2.obj',Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('adder_rr_flap1', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 2.2,
	},
	static = function(lod)
		texture('rr_flap0.png')
		load_obj('rr_flap1.obj',Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('adder_rr_flap2', {
	info = {
		lod_pixels = {1,10,100,0},
		bounding_radius = 2.2,
	},
	static = function(lod)
		texture('rr_flap0.png')
		load_obj('rr_flap2.obj',Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('adder_gun', {
	info = {
		lod_pixels = {1,5,10,0},
		bounding_radius = 3,
		materials = {'chrome', 'black'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('black' ,0,0,0,1,0,0,0,0)

		use_material('chrome')
		texture('gun.png')
		load_obj('gun.obj')

		use_material('black')
		zbias(1,v(0,0,-25.327),v(0,0,-1))
		circle(lod*2,v(0,0,-25.327),v(0,0,-1),v(0,1,0),.12)
		zbias(0)
	end
})


define_model('adder_sub', {
	info = {
		lod_pixels = {5,30,200,0},
		bounding_radius = 34,
		materials = {'text', 'head', 'body', 'non_cv', 'metal', 'chrome', 'matte', 'glow1', 'e_glow', 'wins', 'black', 'scoop'},
	},

	static = function(lod)
		set_material('text',.7,.7,.7,1,.3,.3,.3,5)
		set_material('non_cv', .45,.55,.6,1,.5,.5,.6,50)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
		set_material('matte', .2,.15,.3,1,.2,.15,.3,5)
		set_material('wins', .02,0,.05,1,1,1.5,2,100)
		set_material('black' ,0,0,0,1,0,0,0,0)
		set_material('glow1', .5,.5,.5,1,1,1.5,2,100,.6,1.2,1.2)

		texture('basic.png')
		use_material('chrome')
		load_obj('chrome.obj')

		use_material('non_cv')
		load_obj('non_cv0.obj')

		use_material('metal')
		load_obj('metal.obj')

		texture('naz.png')
		load_obj('nazzles.obj')

		use_material('non_cv')
		texture('back.png')
		load_obj('back.obj')

		use_material('matte')
		texture('tech.png')
		load_obj('tech.obj')

		use_material('scoop')
		texture('scoop.png')
		load_obj('glow2.obj')

		texture('e_glow.png')
		load_obj('e_glow.obj')

		texture(nil)
		use_material('wins')
		load_obj('win.obj')

		use_material('head')
		texture('head.png')
		load_obj('head.obj')

		texture('doors.png')
		load_obj('door.obj')

		use_material('body')

		load_obj('hatch.obj')

		texture('body.png')
		load_obj('body.obj')

		texture('basic.png')
		load_obj('door_frame.obj')

		if lod > 2 then
			zbias(1,v(-8.009,1.34,-10.3),v(-1,.7945,0))
			call_model('decal',v(-8.009,1.34,-10.3),v(-1,0,0),v(.7945,1,0),1.8)
			zbias(1,v(8.009,1.34,-12.1),v(1,.7945,0))
			call_model('squadsign_1',v(8.009,1.34,-12.1),v(1,0,0),v(-.7945,1,0),1.8)
			zbias(0)
		end

		call_model('coll_warn',v(8.9,0,-9.5),v(0,1,0),v(1,0,0),2)
		call_model('coll_warn',v(-8.9,0,-9.5),v(0,1,0),v(-1,0,0),2)
		call_model('coll_warn',v(0,-3.35,7.9),v(1,0,0),v(0,-1,0),2)
		call_model('posl_green',v(8.9,0,-10),v(0,1,0),v(1,0,0),2)
		call_model('posl_red',v(-8.9,0,-10),v(0,1,0),v(-1,0,0),2)
		call_model('posl_white',v(0,3.37,7.9),v(1,0,0),v(0,1,0),2)


	end,

	dynamic = function(lod)

		set_material('e_glow', lerp_materials(get_time('SECONDS')*.5,{0, 0, 0, 1, 0, 0, 0, 0, .9, 1.4, 1.5 },
		{0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.7 }))

		if get_equipment('FUELSCOOP') == 'FUEL_SCOOP' then
			set_material('scoop', lerp_materials(get_time('SECONDS')*.5,{0, 0, 0, 1, 0, 0, 0, 0, .9, 1.4, 1.5 },
			{0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.7 }))
		else
			set_material('scoop', .1,.1,.1,1,0,0,0,1)
		end

		--select2 = 20
		selector2()
		if select2 < 21 then
			set_material('head', .45,.55,.6,1,.5,.5,.6,50)
			set_material('body', .45,.55,.6,1,.5,.5,.6,50)
		else
			if select2 < 41 then
				set_material('head', .45,.5,.6,1,.5,.5,.6,50)
				set_material('body', get_arg_material(0))
			else
				if select2 < 61 then
					set_material('head', get_arg_material(0))
					set_material('body', .45,.55,.6,1,.5,.5,.6,50)
				else
					if select2 < 81 then
						set_material('head', get_arg_material(0))
						set_material('body', get_arg_material(1))
					else
						if select2 > 80 then
							set_material('head', get_arg_material(1))
							set_material('body', get_arg_material(0))
						end
					end
				end
			end
		end

		local flap = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
		use_material('head')
		call_model('adder_f_flap',v(-.598,-3.349,-11.163),v(0,0,1), v(math.sin(flap),math.cos(flap),0), 1)
		use_material('body')
		call_model('adder_rl_flap1',v(-4.791,-3.351,5.087),v(0,0,1), v(math.sin(flap),math.cos(flap*1.1),0), 1)
		call_model('adder_rr_flap1',v(3.355,-3.351,5.087),v(0,0,1), v(math.sin(flap),math.cos(flap*1.1),0), 1)
		call_model('adder_rl_flap2',v(-3.353,-3.351,5.087),v(0,0,1), v(-math.sin(flap),math.cos(flap*1.1),0), 1)
		call_model('adder_rr_flap2',v(4.79,-3.351,5.088),v(0,0,1), v(-math.sin(flap),math.cos(flap*1.1),0), 1)

		if get_animation_position('WHEEL_STATE') ~= 0 then
			call_model('adder_uc_cage',v(0,0,0),v(1,0,0),v(0,1,0),1)

			local uc_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)

			call_model('adder_uc_f',v(.336,-2.686,-12.788),v(1,0,0), v(0,math.cos(uc_rot),math.sin(uc_rot)-.3),1)
			call_model('adder_piston_f',v(.24,-2.1,-11.21),v(1,0,0), v(0,math.cos(uc_rot/1.82),math.sin(uc_rot/1.82)-.3),.3)

			call_model('adder_uc_r',v(0,-2.686,3.173),v(1,0,0), v(0,math.cos(uc_rot*1.05),math.sin(uc_rot*1.05)-.3),1)
			call_model('adder_piston_r',v(3.9,-2.1,4.35),v(1,0,0), v(0,math.cos(uc_rot/1.9),math.sin(uc_rot/1.9)-.3),.3)
			call_model('adder_piston_r',v(-3.9,-2.1,4.35),v(1,0,0), v(0,math.cos(uc_rot/1.9),math.sin(uc_rot/1.9)-.3),.3)
		end


		local  v0 = v(7.546,1.511,4.811)
		local  v1 = v(7.546,1.511,-4.649)
		local  v2 = v(7.401,1.694,4.757)
		local  v3 = v(7.401,1.694,-4.595)
		local  v4 = v(7.605,1.723,4.757)
		local  v5 = v(7.605,1.723,-4.595)
		local  v6 = v(7.774,1.627,4.757)
		local  v7 = v(7.774,1.627,-4.595)
		local  v8 = v(7.81,1.464,4.757)
		local  v9 = v(7.81,1.464,-4.595)
		local v10 = v(7.691,1.328,4.757)
		local v11 = v(7.691,1.328,-4.595)
		local v12 = v(7.195,1.953,4.374)
		local v13 = v(7.195,1.953,-4.212)
		local v14 = v(7.688,2.022,4.374)
		local v15 = v(7.688,2.022,-4.212)
		local v16 = v(8.098,1.791,4.374)
		local v17 = v(8.098,1.791,-4.212)
		local v18 = v(8.184,1.397,4.374)
		local v19 = v(8.184,1.397,-4.212)
		local v20 = v(7.896,1.069,4.374)
		local v21 = v(7.856,1.069,-4.212)

		local v30 = v(-7.546,-1.514,4.811)
		local v31 = v(-7.546,-1.514,-4.649)
		local v32 = v(-7.401,-1.697,4.757)
		local v33 = v(-7.401,-1.697,-4.595)
		local v34 = v(-7.605,-1.726,4.757)
		local v35 = v(-7.605,-1.726,-4.595)
		local v36 = v(-7.774,-1.63,4.757)
		local v37 = v(-7.774,-1.63,-4.595)
		local v38 = v(-7.81,-1.467,4.757)
		local v39 = v(-7.81,-1.467,-4.595)
		local v40 = v(-7.691,-1.331,4.757)
		local v41 = v(-7.691,-1.331,-4.595)
		local v42 = v(-7.195,-1.956,4.374)
		local v43 = v(-7.195,-1.956,-4.212)
		local v44 = v(-7.688,-2.025,4.374)
		local v45 = v(-7.688,-2.025,-4.212)
		local v46 = v(-8.098,-1.794,4.374)
		local v47 = v(-8.098,-1.794,-4.212)
		local v48 = v(-8.184,-1.4,4.374)
		local v49 = v(-8.184,-1.4,-4.212)
		local v50 = v(-7.896,-1.072,4.374)
		local v51 = v(-7.856,-1.072,-4.212)

		local textrans = get_time('SECONDS')*.1

		use_material('glow1')
		if lod > 2 then
			texture('models/ships/adder/glow1.png', v(textrans,textrans*.25,0),v(0,.3,0),v(0,0,.5))
		else
			texture('models/ships/adder/glow1.png', v(.5,.5,0),v(0,.3,0),v(0,0,.5))
		end

		xref_tri(v0,v4,v2)
		xref_tri(v0,v6,v4)
		xref_tri(v0,v8,v6)
		xref_tri(v0,v10,v8)
		xref_quad(v2,v4,v14,v12)
		xref_quad(v4,v6,v16,v14)
		xref_quad(v6,v8,v18,v16)
		xref_quad(v8,v10,v20,v18)
		xref_quad(v12,v14,v15,v13)
		xref_quad(v14,v16,v17,v15)
		xref_quad(v16,v18,v19,v17)
		xref_quad(v18,v20,v21,v19)
		xref_quad(v13,v15,v5,v3)
		xref_quad(v15,v17,v7,v5)
		xref_quad(v17,v19,v9,v7)
		xref_quad(v19,v21,v11,v9)
		xref_tri(v1,v3,v5)
		xref_tri(v1,v5,v7)
		xref_tri(v1,v7,v9)
		xref_tri(v1,v9,v11)

		xref_tri(v30,v34,v32)
		xref_tri(v30,v36,v34)
		xref_tri(v30,v38,v36)
		xref_tri(v30,v40,v38)
		xref_quad(v32,v34,v44,v42)
		xref_quad(v34,v36,v46,v44)
		xref_quad(v36,v38,v48,v46)
		xref_quad(v38,v40,v50,v48)
		xref_quad(v42,v44,v45,v43)
		xref_quad(v44,v46,v47,v45)
		xref_quad(v46,v48,v49,v47)
		xref_quad(v48,v50,v51,v49)
		xref_quad(v43,v45,v35,v33)
		xref_quad(v45,v47,v37,v35)
		xref_quad(v47,v49,v39,v37)
		xref_quad(v49,v51,v41,v39)
		xref_tri(v31,v33,v35)
		xref_tri(v31,v35,v37)
		xref_tri(v31,v37,v39)
		xref_tri(v31,v39,v41)

		texture(nil)


		if get_equipment('SCANNER') == 'SCANNER' then
			use_material('non_cv')
			call_model('scanner_+',v(0,3.3,6.2),v(1,0,0),v(0,1,0),1.2)
			call_model('antenna_1',v(-2,0,-24.55),v(1,0,0),v(0,1,0),1)
		end

		if lod > 2 then
			selector3()
			use_material('head')
			if select3 < 51 then
				if get_equipment('ECM') == 'ECM_BASIC' then
					call_model('ecm_1',v(4.5,-2.43,-16), v(-1,0,0),v(0,-1,0),1)
				end
				if get_equipment('ECM') == 'ECM_ADVANCED' then
					call_model('ecm_2',v(4.5,-2.43,-16), v(-1,0,0),v(0,-1,0),1)
				end
			else
				if get_equipment('ECM') == 'ECM_BASIC' then
					call_model('ecm_1',v(-4.5,-2.43,-16),v(-1,0,0),v(0,-1,0),1)
				end
				if get_equipment('ECM') == 'ECM_ADVANCED' then
					call_model('ecm_2',v(-4.5,-2.43,-16),v(-1,0,0),v(0,-1,0),1)
				end
			end



			local reg = get_label()
			use_material('text')
			zbias(1,v(-8.439,.8,-11.2),v(-1,.7945,0))
			text(reg,v(-8.439,.8,-11.2),v(-1,.7945,0),v(0,0,1),.8, {center = true})
			zbias(1,v(8.439,.8,-11.2),v(1,.7945,0))
			text(reg,v(8.439,.8,-11.2),v(1,.7945,0),v(0,0,-1),.8, {center = true})
			zbias(0)
		end

		if get_equipment('LASER',1) then
			if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
				call_model('adder_gun',v(3.5,0,0),v(1,0,0),v(0,1,0),1)
				call_model('adder_gun',v(-3.5,0,0),v(1,0,0),v(0,1,0),1)
			else
				call_model('adder_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
		end
	end
})

define_model('adder', {
	info = {
		scale = 1.3,
		lod_pixels = {.1,30,100,0},
		bounding_radius = 44,
		tags = {'ship'},
		ship_defs = {
			{
				name='Adder',
				forward_thrust = -100e5,
				reverse_thrust = 51e5,
				up_thrust = 3e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 22e6,
				gun_mounts =
				{
					{ v(0,0,-26), v(0,0,-1) },
					{ v(0,-2,9), v(0,0,1) },
				},
				max_cargo = 50,
				max_laser = 1,
				max_missile = 2,
				max_cargoscoop = 1,
				max_fuelscoop = 1,
				capacity = 50,
				hull_mass = 30,
				fuel_tank_mass = 10,
				thruster_fuel_use = 0.0003,
				price = 60000,
				hyperdrive_class = 2,
			}
		}
	},
	static = function(lod)
		if lod == 1 then
			local  v0 = v(6.57,3.36,8.715)
			local  v1 = v(-6.57,3.36,8.715)
			local  v2 = v(8.884,.24,8.715)
			local  v3 = v(-8.884,.24,8.715)
			local  v4 = v(8.884,-.24,8.715)
			local  v5 = v(-8.884,-.24,8.715)
			local  v6 = v(6.57,-3.36,8.715)
			local  v7 = v(-6.57,-3.36,8.715)
			local  v8 = v(6.57,3.36,-13.074)
			local  v9 = v(-6.57,3.36,-13.074)
			local v10 = v(8.884,.24,-13.074)
			local v11 = v(-8.884,.24,-13.074)
			local v12 = v(8.884,-.24,-13.074)
			local v13 = v(-8.884,-.24,-13.074)
			local v14 = v(6.57,-3.36,-13.074)
			local v15 = v(-6.57,-3.36,-13.074)
			local v16 = v(3.835,.24,-24.815)
			local v17 = v(-3.835,.24,-24.815)
			local v18 = v(3.835,-.24,-24.815)
			local v19 = v(-3.835,-.24,-24.815)

			quad(v0,v1,v3,v2)
			quad(v2,v3,v5,v4)
			quad(v4,v5,v7,v6)
			quad(v0,v8,v9,v1)
			quad(v6,v7,v15,v14)
			quad(v8,v16,v17,v9)
			quad(v14,v15,v19,v18)
			quad(v16,v18,v19,v17)
			xref_quad(v0,v2,v10,v8)
			xref_quad(v2,v4,v12,v10)
			xref_quad(v4,v6,v14,v12)
			xref_quad(v10,v12,v18,v16)
			xref_tri(v8,v10,v16)
			xref_tri(v12,v14,v18)
			geomflag(0x100)
			quad(v(-2.75,-3.307,-8),v(2.75,-3.307,-8),v(1.75,-5,-8),v(-1.75,-5,-8))
			geomflag(0)
		end

		if lod > 1 then

			call_model('adder_sub',v(0,0,0),v(1,0,0),v(0,1,0),1)

			local  M_T = v(0,0,9.5)
			local  R_T = v(5.4,0,-21.6) -- xref
			local RF_T = v(9.3,0,-6.96)
			local LF_T = v(-9.3,0,-6.96)
			local RB_T = v(9.3,0,6.96)
			local LB_T = v(-9.3,0,6.96)
			local TF_T = v(0,3.9,-6.96)
			local TB_T = v(4.8,3.9,6.96) -- xref
			local BF_T = v(4.8,-3.9,-6.96) -- xref
			local BB_T = v(0,-3.9,6.96)

			thruster(M_T,v(0,0,1),25, true)
			xref_thruster(R_T,v(0,0,-1),7, true)
			thruster(RF_T,v(1,0,0),4)
			thruster(LF_T,v(-1,0,0),4)
			thruster(RB_T,v(1,0,0),4)
			thruster(LB_T,v(-1,0,0),4)
			thruster(TF_T,v(0,1,0),4)
			xref_thruster(TB_T,v(0,1,0),4)
			xref_thruster(BF_T,v(0,-1,0),4)
			thruster(BB_T,v(0,-1,0),4)


		end
	end,
	dynamic = function(lod)
		if lod == 1 then
			if get_animation_position('WHEEL_STATE') ~= 0 then
				cylinder(4,v(0,-5.5,-11.5),v(0,-5.5,-13.5),v(0,1,0),.5)
				xref_cylinder(4,v(4.5,-5.5,4.5),v(4.5,-5.5,1.5),v(0,1,0),.5)
			end
		end
	end
})
