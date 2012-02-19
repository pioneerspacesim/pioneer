define_model('eye_0', {
	info = {
		scale = 1,
		lod_pixels = { 1, 20, 100, 0 },
		bounding_radius = 25,
		materials = {'text', 'engine', 'matvar0', 'win', 't_glow', 'e_glow', 'chrome', 'cutout'},
	},

	static = function(lod)

		set_material('engine', .6,.65,.65,1,.5,.5,.5,50)
		set_material('text', .7,.68,.48,1,.3,.3,.3, 5)
		set_material('chrome', .43,.5,.63,1,1,1.2,1.4,30)
		set_material('cutout', .7,.7,.7,.9,.3,.3,.3,50)
		set_material('t_glow', 1,1,1,.7,1,1,2,100,.5,.5,.5)
		set_material('win', .5,.48,.2,.3,1,1,2,100,.4,.38,.2)

		use_material('matvar0')
		texture('eye1.png')
		load_obj('body1.obj', Matrix.rotate(math.pi, v(0,1,0)))

		use_material('engine')
		texture('eye8.png')
		load_obj('eng2.obj', Matrix.rotate(math.pi, v(0,1,0)))
		use_material('chrome')
		load_obj('gun7.obj', Matrix.rotate(math.pi, v(0,1,0)))

		if lod > 1 then
			use_material('win')
			texture('eye4.png')
			load_obj('win4.obj', Matrix.rotate(math.pi, v(0,1,0)))

			use_material('t_glow')
			texture('eye2.png')
			load_obj('bulb3.obj', Matrix.rotate(math.pi, v(0,1,0)))
			texture('eye3.png')
			load_obj('proj8.obj', Matrix.rotate(math.pi, v(0,1,0)))

			use_material('cutout')
			texture('eye6.png')
			load_obj('text6.obj', Matrix.rotate(math.pi, v(0,1,0)))

			use_material('chrome')
			texture('eye9.png')
			load_obj('naz9.obj', Matrix.rotate(math.pi, v(0,1,0)))

			use_material('e_glow')
			texture('eye5.png')
			load_obj('thrust5.obj', Matrix.rotate(.5*math.pi, v(0,-1,0)) * Matrix.orient(v(1,0,0),v(0,0,1),v(0,1,0)) * Matrix.translate(v(0,0,1)))

			-- poslights
			call_model('posl_white', v(0,9.9,0), v(1,0,0), v(0,1,0), 2)
			call_model('coll_warn', v(0,-8.216,5.5), v(1,0,0), v(0,-1,.7), 2)
			call_model('headlight', v(0,-5.483,-8), v(1,0,0), v(0,-.6,-1), 3)
			call_model('posl_green', v(15.9,5.2,0), v(0,1,0), v(1,-.2,0), 2)
			call_model('coll_warn', v(13.555,3.25,0), v(1,0,0), v(0,-1,0), 2)
			call_model('posl_red', v(-15.9,5.2,0), v(0,1,0), v(-1,.2,0), 2)
			call_model('coll_warn', v(-13.555,3.25,0), v(1,0,0), v(0,-1,0), 2)
			billboard('smoke.png',10, v(.8,.9,1), {v(13.55,5.712,-4.5),v(-13.55,5.712,-4.5)})
		end

	end,
	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5,   {.5,.5,.5,1,0,0,0,0,.5,1,1},
		{.5,.5,.5,1,0,0,0,0,2.5,.5,1}))
		set_material('matvar0', get_arg_material(0))

		if lod > 1 then
			texture(nil)
			use_material('text')
			local reg = get_label()
			zbias(1,v(15.9431,6.2,0), v(1,.19891,0))
			text(reg, v(15.9431,6.2,0), v(1,.19891,0), v(0,0,-1), 1.3, {center=true})
			zbias(1,v(-15.9431,6.2,0), v(-1,.19891,0))
			text(reg, v(-15.9431,6.2,0), v(-1,.19891,0), v(0,0,1), 1.3, {center=true})
			zbias(0)
		end

		if get_animation_position('WHEEL_STATE') ~= 0 then
			local uc_trans = 2*math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
			use_material('chrome')
			texture('models/ships/eye/eye8.png')
			tapered_cylinder(3*lod, v(3.2,-7,-5.3), v(3.2+uc_trans,-7-(2*uc_trans),-5.3-uc_trans), v(0,0,1), .3,.2)
			tapered_cylinder(3*lod, v(-3.2,-7,-5.3), v(-3.2-uc_trans,-7-(2*uc_trans),-5.3-uc_trans), v(0,0,1), .3,.2)

			tapered_cylinder(3*lod, v(3.2,-7,5.3), v(3.2+uc_trans,-7-(2*uc_trans),5.3+uc_trans), v(0,0,1), .3,.2)
			tapered_cylinder(3*lod, v(-3.2,-7,5.3), v(-3.2-uc_trans,-7-(2*uc_trans),5.3+uc_trans), v(0,0,1), .3,.2)
		end

		texture(nil)
		local MT = v(13.57,5.7,9.5)
		local RFTT = v(4.5,4.5,-8.75)
		local LFTT = v(-4.5,4.5,-8.75)
		local RFBT = v(4.5,-4.5,-8.75)
		local LFBT = v(-4.5,-4.5,-8.75)
		local RBTT = v(4.5,4.5,9)
		local LBTT = v(-4.5,4.5,9)
		local RBBT = v(4.5,-4.5,9)
		local LBBT = v(-4.5,-4.5,9)
		xref_thruster(MT, v(0,0,1), 30, true)
		thruster(RFTT, v(.5,.75,-.75), 5)
		thruster(LFTT, v(-.5,.75,-.75), 5)
		thruster(RFBT, v(.5,-.75,-.75), 5)
		thruster(LFBT, v(-.5,-.75,-.75), 5)
		thruster(RBTT, v(.5,.75,.75), 5)
		thruster(LBTT, v(-.5,.75,.75), 5)
		thruster(RBBT, v(.5,-.75,.75), 5)
		thruster(LBBT, v(-.5,-.75,.75), 5)
	end
})

define_model('peye', {
	info = {
		scale = 1,
		lod_pixels = { 1, 20, 100, 0 },
		bounding_radius = 30,
		tags = {'ship'},
		ship_defs = {
			{
				name='E.Y.E',
				forward_thrust = -20e6,
				reverse_thrust = 10e6,
				up_thrust = 5e6,
				down_thrust = -4e6,
				left_thrust = -4e6,
				right_thrust = 4e6,
				angular_thrust = 20e6,
				gun_mounts =
				{
					{ v(0,-5.8,-11), v(0,0,-1) },
					{ v(0,0,10), v(0,0,1) },
				},
				max_cargo = 80,
				max_laser = 2,
				max_missile = 0,
				max_cargoscoop = 0,
				capacity = 80,
				hull_mass = 25,
				fuel_tank_mass = 55,
				thruster_fuel_use = 0.00025,
				price = 100000,
				hyperdrive_class = 3,
			}
		},
	},

	static = function(lod)
		call_model('eye_0',v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})
