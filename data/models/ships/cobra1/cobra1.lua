define_model('cobra1', {
	info = {
		scale = 1.5,
		lod_pixels = { .1, 10, 100, 0 },
		bounding_radius = 20,
		materials = {'text', 'top', 'bot', 'posl', 'gun', 'engine_inside'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Cobra Mk I',
				forward_thrust = -12e6,
				reverse_thrust = 5e6,
				up_thrust = 4e6,
				down_thrust = -4e6,
				left_thrust = -4e6,
				right_thrust = 4e6,
				angular_thrust = 28e6,
				gun_mounts =
				{
					{ v(0,0,-13), v(0,0,-1) },
					{ v(0,0,10), v(0,0,1) }
				},
				max_cargo = 60,
				max_laser = 2,
				max_missile = 2,
				max_cargoscoop = 0,
				capacity = 60,
				hull_mass = 40,
				fuel_tank_mass = 20,
				thruster_fuel_use = 0.0002,
				price = 97000,
				hyperdrive_class = 2,
			}
		}
	},
	static = function(lod)

		set_material('gun', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('text', .7, .7, .7,1, .3, .3, .3, 10)
		set_material('posl', .5,.5,.5,.2,1.2,1.8,2,100)
		set_material('text', .7,.7,.7,1,.3,.3,.3,5)

		if lod > 1 then
			use_material('top')
			texture('cobra_x.png')
		end
		load_obj('cobra1_top.obj')

		if lod > 1 then
			use_material('bot')
			texture('cob_x_bot.png')
		end
		load_obj('cobra1_bot.obj')

		if lod > 1 then
			use_material('engine_inside')
			texture('cob_x_e.png')
		end
		load_obj('cobra1_eng.obj')

		if lod > 1 then
			use_material('gun')
			texture('cob_x_c.png')
			load_obj('cobra1_gun.obj')

			use_material('posl')
			texture('cob_x_l.png')
			load_obj('cobra1_posl.obj')

			if lod > 2 then
				use_material('bot')
				local factor = 0.05*math.pi*os.clock() -- get_time('SECONDS') causes an asset error here.. strange!?!
				call_model('scanner_+', v(0.013,3.3,5.641), v(math.cos(factor),0,math.sin(factor)), v(0,1,0),1.4)
			end
		end
	end,

	dynamic = function(lod)

		set_material('top', get_arg_material(0))
		set_material('bot', get_arg_material(1))
		set_material('engine_inside', lerp_materials(get_time('SECONDS')*.5, {0, 0, 0, 1, 0, 0, 0, 10, .6, .9, 1 }, {0, 0, 0, 1, 0, 0, 0, 10, 1, 0, 1 }))
		if lod > 2 then
			local reg = get_label()
			use_material('text')
			zbias(1,v(13.26,.5,4), v(.67,1,0))
			text(reg,v(13.26,.5,4), v(.67,1,0), v(0,0,-1),1.2, {center = true})
			zbias(1,v(-13.26,-.5,4), v(-.67,-1,0))
			text(reg,v(-13.26,-.5,4), v(-.67,-1,0), v(0,0,1),1.2, {center = true})
			zbias(0)
		end

		if get_animation_position('WHEEL_STATE') == 0 then
			if lod > 2 then
				-- posl
				local lightphase = math.fmod(get_time('SECONDS'), 1)
				if lightphase < .4 then
					billboard('smoke.png', 5, v(1,0.8,0), { v(0, -0.8, -7) })
				elseif lightphase  > .4 then
					if lightphase < .7 then
						billboard('smoke.png', 5,  v(.3,1,0), { v(13.5, 0, 7.4) })
					elseif lightphase > .7 then
						billboard('smoke.png', 5, v(1,0,0), { v(-13.5, 0, 7.4) })
					end
				end
			end
		end

		if get_animation_position('WHEEL_STATE') ~= 0 then
			-- wheels
			zbias(1,v(0, -.77, -5), v(0,-1,-.18))
			call_model('nosewheelunit', v(0, -.71, -5.5), v(-1,0,0), v(0,-1,-.18), .9)
			zbias(1, v(0, -2.2, 3), v(0,-1,-.185))
			call_model('nosewheelunit', v(-8, -2.2, 3), v(-1,0,0), v(0,-1,-.185), .6)
			call_model('nosewheelunit', v(8, -2.2, 3), v(-1,0,0), v(0,-1,-.185), .6)
			zbias(0)



			if lod > 2 then
				-- posl
				local lightphase = math.fmod(get_time('SECONDS'), 1)
				if lightphase < .4 then
					billboard('smoke.png', 5, v(1,1,1), { v(0, -0.8, -7) })
				elseif lightphase  > .4 then
					if lightphase < .7 then
						billboard('smoke.png', 5,  v(1,1,1), { v(13.5, 0, 7.4) })
					elseif lightphase > .7 then
						billboard('smoke.png', 5, v(1,1,1), { v(-13.5, 0, 7.4) })
					end
				end
			end


		end

		local M_T = v(4.2, 0.5, 9) -- last number is how far back down the ship it is
		local R_T = v(5, 0, -9.5)

		xref_thruster(M_T, v(0,0,1), 20, true)
		xref_thruster(R_T, v(0,0,-1), 15, true)

		local TLF_T = v(-13, 1, -1)
		local TRF_T = v(13, 1, -1)

		thruster(TLF_T, v(-1,1,0), 5)
		thruster(TRF_T, v(1,1,0), 5)

		local TLB_T = v(-13, 1,6)
		local TRB_T = v(13, 1, 6)

		thruster(TLB_T, v(-1,1,0), 5)
		thruster(TRB_T, v(1,1,0), 5)

		local BLB_T = v(-13, -1, 6)
		local BRB_T = v(13, -1, 6)

		thruster(BLB_T, v(-1,-1,0), 5)
		thruster(BRB_T, v(1,-1,0), 5)

		local BLF_T = v(-13, -1,-1)
		local BRF_T = v(13, -1,-1)

		thruster(BLF_T, v(-1,-1,0), 5)
		thruster(BRF_T, v(1,-1,0), 5)
	end
})
