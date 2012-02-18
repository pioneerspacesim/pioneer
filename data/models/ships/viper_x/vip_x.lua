define_model('vipx_singleg', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = { 'ncv' }
	},
	static = function(lod)
		set_material('ncv', .25,.3,.35,1,1.26,1.4,1.66,30)
		use_material('ncv')
		texture('vipx_extra1.png')
		load_obj('vipx_singleg.obj')
	end
})

define_model('vipx_gun2_base', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = { 'ncv', 'black' }
	},
	static = function(lod)
		set_material('ncv', .25,.3,.35,1,1.26,1.4,1.66,30)
		set_material('black', .1,.12,.12,1,.1,.15,.15,5)
		use_material('ncv')
		texture('vipx_extra1.png')
		load_obj('vipx_gun2_base0.obj')
		use_material('black')
		texture('body2.png')
		load_obj('vipx_gun2_base1.obj')
	end
})

define_model('vipx_gun1_base', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = { 'ncv', 'black' }
	},
	static = function(lod)
		set_material('ncv', .25,.3,.35,1,1.26,1.4,1.66,30)
		set_material('black', .1,.12,.12,1,.1,.15,.15,5)
		use_material('ncv')
		texture('vipx_extra1.png')
		load_obj('vipx_gun1_base.obj')
		use_material('black')
		texture('body2.png')
		load_obj('vipx_gun1_base1.obj')
	end
})

define_model('vipx_gun2', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = { 'chrome', 'black' }
	},
	static = function(lod)
		set_material('chrome', .35,.4,.45,1,1.26,1.4,1.66,30)
		set_material('black', .1,.12,.12,1,.1,.15,.15,5)
		use_material('black')
		texture('body2.png')
		load_obj('vipx_gun2_1.obj')
		use_material('chrome')
		texture('chrome.png')
		load_obj('vipx_gun2_2.obj')
	end
})

define_model('vipx_gun1', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = { 'chrome', 'black' }
	},
	static = function(lod)
		set_material('chrome', .35,.4,.45,1,1.26,1.4,1.66,30)
		set_material('black', .1,.12,.12,1,.1,.15,.15,5)
		use_material('black')
		load_obj('vipx_gun1_1.obj')
		texture('chrome.png')
		use_material('chrome')
		load_obj('vipx_gun1_2.obj')
	end
})

define_model('vipx_cage', {
	info = {
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = {'black'},
	},
	static = function(lod)
		set_material('black', .1,.12,.12,1,.1,.15,.15,5)
		use_material('black')
		texture('body2.png')
		load_obj('vipx_cage.obj')
	end
})

define_model('vipx_pad', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
		materials = {'ncv'},
	},

	static = function(lod)
		if lod > 1 then
			set_material('ncv',  .35,.4,.45,1,1.26,1.4,1.66,30)
		end
		use_material('ncv')
		texture('steel.png')
		load_obj('vipx_pad0.obj',Matrix.rotate(.5*math.pi,v(0,0,1)))

		texture('pad_1.png')
		load_obj('vipx_pad1.obj',Matrix.rotate(.5*math.pi,v(0,0,1)))

		texture('pad_2.png')
		load_obj('vipx_pad2.obj',Matrix.rotate(.5*math.pi,v(0,0,1)))
	end
})

define_model('vipx_uc_f', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
		materials = {'ncv'},
	},

	static = function(lod)
		if lod > 1 then
			set_material('ncv',  .35,.4,.45,1,1.26,1.4,1.66,30)
		end
	end,
	dynamic = function(lod)
		local divs = 3*lod
		local trans = 6.6*(math.clamp(get_animation_position('WHEEL_STATE'),.4,1)-.4)   -- 3*1.43*
		--local rot_x = 9.55*(math.pi/180)*(math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5) -- for rotation when call a function instead of a model, math.pi/360 = 1°!
		--local rot_z = 9.55*(math.pi/180)*(math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5) -- (math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5) subtracts .5 from clamp value to start at 0
		-- start time = .5, duration = .5, initial value = 0!

		local rot = 2*(1/45*9.55)*(math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5)          -- 1:1 = 45°. 1/45 = 1°

		call_model('vipx_pad',v(0,.2-trans,0),v(0,-1,0),v(0,rot,1),1)


		if lod > 1 then
			use_material('ncv')
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,0,0),v(0,1-.1*trans,0))
		end
		tapered_cylinder(divs,v(0,2,0),v(0,-trans,0),v(1,0,0),.2,.12)
		if lod > 1 then
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,0,0),v(0,1-.1*trans,-.3))
		end
		tapered_cylinder(divs,v(0,2,1),v(0,-trans,0),v(1,0,0),.12,.08)
	end
})

define_model('vipx_uc_r', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
		materials = {'ncv', 'chrome', 'black'},
	},

	static = function(lod)
		if lod > 1 then
			set_material('black', .1,.12,.12,1,.1,.15,.15,5)
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('ncv',  .35,.4,.45,1,1.26,1.4,1.66,30)
		end
	end,
	dynamic = function(lod)
		local divs = 3*lod
		local trans = 5*(math.clamp(get_animation_position('WHEEL_STATE'),.4,1)-.4)

		local rot_x = 2*(1/45*16.5)*(math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5)
		local rot_z = math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5

		call_model('vipx_pad',v(0,.1-trans,0),v(-rot_z,-1,0),v(0,-rot_x,1),1)

		if lod > 1 then
			use_material('ncv')
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,0,0),v(0,1-.1*trans,0))
		end
		tapered_cylinder(divs,v(0,2,0),v(0,-trans,0),v(1,0,0),.2,.12)
		if lod > 1 then
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,-.3+.05*trans,0),v(0,1-.1*trans,0))
		end
		tapered_cylinder(divs,v(-1,2,0),v(0,-trans,0),v(1,0,0),.12,.08)
	end
})

define_model('vipx_uc_l', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
		materials = {'ncv', 'chrome', 'black'},
	},

	static = function(lod)
		if lod > 1 then
			set_material('black', .1,.12,.12,1,.1,.15,.15,5)
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('ncv',  .35,.4,.45,1,1.26,1.4,1.66,30)
		end
	end,
	dynamic = function(lod)
		local divs = 3*lod
		local trans = 5*(math.clamp(get_animation_position('WHEEL_STATE'),.4,1)-.4)

		local rot_x = 2*(1/45*16.5)*(math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5)
		local rot_z = math.clamp(get_animation_position('WHEEL_STATE'),.5,1)-.5

		call_model('vipx_pad',v(0,.1-trans,0),v(rot_z,-1,0),v(0,-rot_x,1),1)

		if lod > 1 then
			use_material('ncv')
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,0,0),v(0,1-.1*trans,0))
		end
		tapered_cylinder(divs,v(0,2,0),v(0,-trans,0),v(1,0,0),.2,.12)
		if lod > 1 then
			texture('models/ships/viper_x/piston.png',v(.5,-.5*trans,0),v(1,.3-.05*trans,0),v(0,1-.1*trans,0))
		end
		tapered_cylinder(divs,v(1,2,0),v(0,-trans,0),v(1,0,0),.12,.08)
	end
})

--[[
too sad that Matrix.translate will move the .obj out of it's normals :(
--]]

define_model('vipx_fl_q', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_f.png')
		load_obj('vipx_fl_q.obj')
	end
})

define_model('vipx_fr_q', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_f.png')
		load_obj('vipx_fr_q.obj')
	end
})

define_model('vipx_rr_q', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_f.png')
		load_obj('vipx_rr_q.obj')
	end
})

define_model('vipx_rl_q', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_f.png')
		load_obj('vipx_rl_q.obj')
	end
})

define_model('vipx_flap_r', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_rr.png')
		load_obj('vipx_flap_r.obj')
	end
})

define_model('vipx_flap_l', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		texture('flap_rl.png')
		load_obj('vipx_flap_l.obj')
	end
})

define_model('vipx_hole', {
	info = {
		lod_pixels = { .1, 10,20, 0 },
		bounding_radius = 5,
	},

	static = function(lod)
		load_obj('vipx_hole_f.obj')
	end
})

define_model('vipx_uc_all', {
	info = {
		lod_pixels = { .1, 50,100, 0 },
		bounding_radius = 35,
		materials = {'cv0', 'ncv', 'chrome', 'black', 'null'},
	},

	static = function(lod)
		if lod > 1 then
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('ncv',  .25,.3,.35,1,1.26,1.4,1.66,30)
			set_material('null',0,0,0,0,0,0,0,0)
		end
	end,
	dynamic = function(lod)
		if lod > 2 then
			if get_animation_position('WHEEL_STATE') ~= 0 then
				set_material('cv0',get_arg_material(0))

				local trans = 2*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)


				call_model('vipx_cage', v(0,0,0), v(1,0,0), v(0,1,0),1)
				call_model('vipx_uc_f',v(0,-2.21,-12.625),v(1,0,0),v(0,1,1/45*9.55),1)

				call_model('vipx_uc_r',v(4.837,-3.436,8.547),v(1,0,0),v(-1/45*26,1,-1/45*14),1)
				call_model('vipx_uc_l',v(-4.999,-3.436,8.547),v(1,0,0),v(1/45*26,1,-1/45*14),1)


				use_material('cv0')
				call_model('vipx_fl_q',v(-.912*trans,.15*trans,-.891*trans),v(1,0,0),v(0,1,0),1)
				call_model('vipx_fr_q',v(.912*trans,.15*trans,-.891*trans),v(1,0,0),v(0,1,0),1)
				call_model('vipx_rr_q',v(.912*trans,-.15*trans,.891*trans),v(1,0,0),v(0,1,0),1)
				call_model('vipx_rl_q',v(-.912*trans,-.15*trans,.891*trans),v(1,0,0),v(0,1,0),1)
				call_model('vipx_flap_r',v(2.228*trans,1.507*trans,1.477*trans),v(1,0,0),v(0,1,0),1)
				call_model('vipx_flap_l',v(-2.241*trans,1.531*trans,1.536*trans),v(1,0,0),v(0,1,0),1)

				call_model('vipx_hole', v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
		end
	end
})

define_model('vipx_galmap', {
	info = {
		lod_pixels = { .1, 1,10,0 },
		bounding_radius = 1,
		materials = {'glow','black','win'}
	},

	static = function(lod)
		set_material('glow', .5,.5,.5,.99,.5,.5,.5,5,1.5,2,2)
		set_material('win', 0,0,0,.4,1,1,1.5,100)
		set_material('black', .05,.05,.05,1,.1,.15,.2,5)

		set_light(1, 0.02, v(2.7,2.6,2), v(1.2,1.15,1.1))
		set_light(2, 0.02, v(-2.7,2.6,2), v(1.2,1.15,1.1))
		set_local_lighting(true)
		use_light(1)
		use_light(2)

		texture(nil)
		use_material('black')
		set_insideout(true)
		load_obj('vipx_galmap_0.obj')
		texture('galmap.png')
		use_material('glow')
		load_obj('vipx_galmap_1.obj')
		texture(nil)
		use_material('win')
		load_obj('vipx_galmap_0.obj')
		set_local_lighting(false)
	end
})

define_model('viper_x', {
	info = {
		scale = 1.0,
		lod_pixels = { .1, 50, 100, 0 },
		bounding_radius = 35,
		materials = {'win', 'cv0', 'ncv', 'chrome', 'black', 'layer', 'glow1', 'glow2a',
		'glow2b', 'glow2c', 'glow2d', 'glow2e', 'glow2f', 'e_glow', 'sc_glow', 'glow', 'pit_0', 'pit_1', 'light'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Viper X',
				forward_thrust = -9e6,
				reverse_thrust = 3e6,
				up_thrust = 3e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 20e6,
				gun_mounts =
				{
					{ v(0,-1.4,-28), v(0,0,-1) },
					{ v(0,0,0), v(0,0,1) }
				},
				max_cargo = 55,
				max_laser = 1,
				max_missile = 4,
				max_cargoscoop = 0,
				max_fuelscoop = 1,
				capacity = 55,
				hull_mass = 25,
				fuel_tank_mass = 15,
				thruster_fuel_use = 0.0002,
				price = 90000,
				hyperdrive_class = 2,
			}
		}
	},

	static = function(lod)
		call_model('vipx_uc_all',v(0,0,0),v(1,0,0),v(0,1,0),1)

		if lod == 1 then
			load_obj('vipx_coll.obj')
		else
			set_material('win', 0,0,0,.4,1,1,1.5,100)
			set_material('black', .1,.12,.12,1,.1,.15,.15,5)
			set_material('chrome', .35,.4,.45,1,1.26,1.4,1.66,30) --.63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('ncv',  .25,.3,.35,1,1.26,1.4,1.66,30)
			set_material('layer',  .63,.7,.83,.99,1.26,1.4,1.66,30)
			set_material('pit_0', .35,.33,.3,1,.3,.33,.35,10)
			set_material('pit_1', .6,.58,.55,1,.3,.33,.35,30)
			set_material('glow', 0,0,0,.99,1,1,1.5,100,1.8,1.8,1.5)

			--set_material('light', 0,0,0,.1,0,0,0,1,100,100,100)

			set_light(1, 0.02, v(2.7,2.6,2), v(1.2,1.15,1.1))
			set_light(2, 0.02, v(-2.7,2.6,2), v(1.2,1.15,1.1))

			set_local_lighting(true)
			use_light(1)
			use_light(2)

			use_material('pit_0')
			load_obj('vipx_pit_0.obj')

			use_material('glow')
			load_obj('vipx_pit_glo.obj')

			use_material('pit_1')
			texture('vipx_pit.png')
			load_obj('vipx_pit_1.obj')

			use_material('glow')
			load_obj('vipx_pit_d_glo.obj')

			if lod > 3 then
				call_model('pilot_3_m',v(2.2,.7,1.5),v(1,0,0),v(0,1,0),1.25)
				call_model('vipx_galmap',v(0,1.6,1.3),v(1,0,0),v(0,1,0),2)
			end

			set_local_lighting(false)

			texture('vipx1.png')
			use_material('cv0')
			load_obj('vipx_cv0.obj')

			use_material('ncv')
			load_obj('vipx_ncv.obj')

			use_material('layer')
			load_obj('vipx_layer1.obj')

			texture('body2.png')
			use_material('black')
			load_obj('vipx_black.obj')

			texture('vipx_side.png')
			use_material('ncv')
			load_obj('vipx_ncv2.obj')

			use_material('layer')
			load_obj('vipx_layer2.obj')

			texture('vipx_extra1.png')
			use_material('chrome')
			load_obj('vipx_thrust_m.obj')

			texture('thrust.png')
			use_material('chrome')
			load_obj('vipx_thrust_s.obj')

			texture('door.png')
			use_material('black')
			load_obj('vipx_door.obj')

			texture('chrome.png')
			use_material('chrome')
			load_obj('vipx_chrome.obj')

			texture('e_glow.png')
			use_material('e_glow')
			load_obj('vipx_eglow.obj')
			use_material('sc_glow')
			load_obj('vipx_sc_glow.obj')

			texture('glow.png')
			use_material('glow1')
			load_obj('vipx_glow1.obj')

			use_material('glow2a')
			load_obj('vipx_glow2a.obj')

			use_material('glow2b')
			load_obj('vipx_glow2b.obj')

			use_material('glow2c')
			load_obj('vipx_glow2c.obj')

			use_material('glow2d')
			load_obj('vipx_glow2d.obj')

			use_material('glow2e')
			load_obj('vipx_glow2e.obj')

			use_material('glow2f')
			load_obj('vipx_glow2f.obj')

			call_model('posl_green',v(12,1.49,6.61),v(1,0,0),v(.6,1,0),1.5)
			call_model('posl_red',v(-12,1.49,6.61),v(1,0,0),v(-.6,1,0),1.5)

			call_model('coll_warn',v(12,-1.44,6.61),v(1,0,0),v(.6,-1,0),1.5)
			call_model('coll_warn',v(-12,-1.44,6.61),v(1,0,0),v(-.6,-1,0),1.5)

			call_model('headlight',v(1.7,-.464,-22.456),v(1,0,0),v(0,-.3,-1),2)
			call_model('headlight',v(-1.7,-.464,-22.456),v(1,0,0),v(0,-.3,-1),2)



			texture(nil)
			use_material('win')
			load_obj('vipx_win.obj')

			local M_T1 = v(0,0,17)
			local M_T2 = v(2,0,17)
			local M_T3 = v(4,0,17)

			local R_T  = v(7.05,0,-6)

			local RF_T = v(8.9,0,-4.65)
			local LF_T = v(-8.9,0,-4.65)

			local RR_T = v(12.5,0,5.56)
			local LR_T = v(-12.5,0,5.56)

			local TF_T = v(0,4,-8.09)
			local TR_T = v(10.2,2.4,8.08)

			local BF_T = v(0,-3.7,-8.09)
			local BR_T = v(10.2,-2.2,8.08)

			local rot_T = 1/45

			call_model('blank',v(0,0,0),v(1,0,0),v(0,1,0),0)

			thruster(M_T1,v(0,0,1),10,true)
			xref_thruster(M_T2,v(0,0,1),10,true)
			xref_thruster(M_T3,v(0,0,1),10,true)
			xref_thruster(R_T,v(0,0,-1),7,true)

			thruster(RF_T,v(1,0,0),5)
			thruster(LF_T,v(-1,0,0),5)
			thruster(RR_T,v(1,0,0),5)
			thruster(LR_T,v(-1,0,0),5)
			thruster(TF_T,v(0,1,0),5)
			thruster(BF_T,v(0,-1,0),5)
			xref_thruster(TR_T,v(0,1,0),5)
			xref_thruster(BR_T,v(0,-1,0),5)

			--use_material('light')
			--tapered_cylinder(24,v(0,-.464,-22.456),v(0,-500,-500),v(0,0,1),.5,100)

		end
	end,

	dynamic = function(lod)
		if lod > 1 then
			local time = math.fmod(get_time('SECONDS')*.5,1)
			local g2off = {.3,.4,.5,1,1,1,1,100,0,0,0}
			local g2on = lerp_materials(get_time('SECONDS')*0.1, {0,0,0,1,1,1,1,100,.85,.8,1.5}, {0,0,0,1,1,1,1,100,.5,.5,2})

			set_material('cv0', get_arg_material(0))
			set_material('glow1',lerp_materials(get_time('SECONDS')*0.3, {0,0,0,1,1,1,1,100,1,2,.5}, {0,0,0,1,1,1,1,100,1.4,1.8,.5}))
			set_material('e_glow',lerp_materials(get_time('SECONDS')*0.3, {0,0,0,1,.1,.1,.1,1,.5,.5,2}, {0,0,0,1,.1,.1,.1,1,.85,.8,1.5}))

			if get_equipment('FUELSCOOP') == 'FUEL_SCOOP' then
				set_material('sc_glow',lerp_materials(get_time('SECONDS')*0.3, {0,0,0,1,.1,.1,.1,1,.5,.5,2}, {0,0,0,1,.1,.1,.1,1,.85,.8,1.5}))
			else
				set_material('sc_glow', .1,.12,.12,1,.1,.15,.15,5)
			end

			if time < .167 then
				set_material('glow2a',g2on)
				set_material('glow2b',g2off)
				set_material('glow2c',g2off)
				set_material('glow2d',g2off)
				set_material('glow2e',g2off)
				set_material('glow2f',g2off)
			elseif time < .333 then
				set_material('glow2a',g2off)
				set_material('glow2b',g2on)
				set_material('glow2c',g2off)
				set_material('glow2d',g2off)
				set_material('glow2e',g2off)
				set_material('glow2f',g2off)

			elseif time < .5 then
				set_material('glow2a',g2off)
				set_material('glow2b',g2off)
				set_material('glow2c',g2on)
				set_material('glow2d',g2off)
				set_material('glow2e',g2off)
				set_material('glow2f',g2off)
			elseif time < .667 then
				set_material('glow2a',g2off)
				set_material('glow2b',g2off)
				set_material('glow2c',g2off)
				set_material('glow2d',g2on)
				set_material('glow2e',g2off)
				set_material('glow2f',g2off)
			elseif time < .833 then
				set_material('glow2a',g2off)
				set_material('glow2b',g2off)
				set_material('glow2c',g2off)
				set_material('glow2d',g2off)
				set_material('glow2e',g2on)
				set_material('glow2f',g2off)
			elseif time > .832 then
				set_material('glow2a',g2off)
				set_material('glow2b',g2off)
				set_material('glow2c',g2off)
				set_material('glow2d',g2off)
				set_material('glow2e',g2off)
				set_material('glow2f',g2on)
			end

			if get_equipment('LASER',1) then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					-- twin gun socket
					call_model('vipx_gun2_base',v(0,0,0),v(1,0,0),v(0,1,0),1)
					-- twin gun
					call_model('vipx_gun2',v(0,0,0),v(1,0,0),v(0,1,0),1)
				else
					-- single gun socket
					call_model('vipx_gun1_base',v(0,0,0),v(1,0,0),v(0,1,0),1)
					-- single gun
					call_model('vipx_gun1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
			end

			if get_equipment('ECM') == 'ECM_BASIC' then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					-- twin gun ECM1 pos
					call_model('ecm_1',v(0,-1.7,-21.228),v(-1,0,0),v(0,-1,0),1)
				else
					-- ECM1 pos
					call_model('ecm_1a',v(-4.2,0,-18.8),v(0,1,0),v(-1,0,0),1)
				end
			end

			if get_equipment('ECM') == 'ECM_ADVANCED' then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					-- twin gun ECM2 pos
					call_model('ecm_2',v(0,-1.7,-21.228),v(-1,0,0),v(0,-1,0),1)
				else
					-- ECM2 pos
					call_model('ecm_2a',v(-4.2,0,-18.8),v(0,1,0),v(-1,0,0),1)
				end
			end

			if get_equipment('SCANNER') == 'SCANNER' then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then

					-- if twin gun, single gun socket for antenna if ECM not present
					if not get_equipment('ECM') then
						call_model('vipx_gun1_base',v(0,0,0),v(1,0,0),v(0,1,0),1)
					end

					-- twin gun antenna pos
					call_model('antenna_2',v(0,-1.4,-22.213),v(1,0,0),v(0,1,0),1)
				else
					-- antenna pos
					call_model('antenna_2',v(4.2,0,-18.8),v(1,0,0),v(0,1,0),1)
				end

				-- scanner pos
				use_material('ncv')
				call_model('scanner',v(0,5.7,6.66),v(1,0,0),v(0,1,0),1)
			end

			if get_equipment('LASER',1) then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then

				else
					if not get_equipment('ECM') and not get_equipment('SCANNER') then
						call_model('vipx_singleg', v(0,0,0),v(1,0,0),v(0,1,0),1)
					end
				end
			else
				if not get_equipment('ECM') and not get_equipment('SCANNER') then
					call_model('vipx_singleg', v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
			end
		end
	end
})
