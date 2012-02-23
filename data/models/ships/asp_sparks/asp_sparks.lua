define_model('asps_gun_f', {
	info = {
		bounding_radius = 2,
		materials = { 'metal' }
	},
	static = function(lod)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
		use_material('metal')
		texture('asps_gun.png')
		load_obj('asps_gun_f.obj')
	end
})

define_model('asps_gun_r', {
	info = {
		bounding_radius = 2,
		materials = { 'metal' }
	},
	static = function(lod)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
		use_material('metal')
		texture('asps_gun.png')
		load_obj('asps_gun_r.obj')
	end
})

define_model('asps_flap_r_0', {
	info = {
		lod_pixels = {.1,10,20,0},
		bounding_radius = 1,
	},
	static = function(lod)
		if lod > 1 then
			texture('asps_skin.png')
		end
		load_obj('asps_flap_r.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
	end
})

define_model('asps_flap_l_0', {
	info = {
		lod_pixels = {.1,10,20,0},
		bounding_radius = 1,
	},
	static = function(lod)
		if lod > 1 then
			texture('asps_skin.png')
		end
		load_obj('asps_flap_l.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
	end
})

define_model('asps_flap_r', {
	info = {
		bounding_radius = 1,
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		local flap = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
		call_model('asps_flap_r_0',v(0,0,0),v(0,0,1),v(math.sin(flap),math.cos(flap),0),1)
	end
})

define_model('asps_flap_l', {
	info = {
		bounding_radius = 1,
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		local flap = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
		call_model('asps_flap_l_0',v(0,0,0),v(0,0,1),v(-math.sin(flap),math.cos(flap),0),1)
	end
})

define_model('asps_wheel_f', {
	info = {
		lod_pixels = {.1,10,20,0},
		bounding_radius = 1,
	},
	static = function(lod)
		if lod == 1 then
			load_obj('asps_wf_coll.obj')
		else
			texture('asps_skin.png')
			load_obj('asps_wheel_f.obj')
		end
	end
})

define_model('asps_wheel_r_r', {
	info = {
		lod_pixels = {.1,10,20,0},
		bounding_radius = 1,
		materials = {'non_cv', 'cv_0'},
	},
	static = function(lod)
		if lod == 1 then
			load_obj('asps_wrr_coll.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
		else
			texture('asps_skin.png')
			use_material('cv_0')
			load_obj('asps_wheel_r_r_1.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))

			set_material('non_cv', .63,.7,.83,1,.83,.9,1.03,30)
			use_material('non_cv')
			load_obj('asps_wheel_r_r_0.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			selector2()
			if select2 < 51 then
				set_material('cv_0', .63,.7,.83,1,.83,.9,1.03,30)
			else
				set_material('cv_0', get_arg_material(0))
			end
		end
	end
})

define_model('asps_wheel_r_l', {
	info = {
		lod_pixels = {.1,10,20,0},
		bounding_radius = 1,
		materials = {'non_cv', 'cv_0'},
	},
	static = function(lod)
		if lod == 1 then
			load_obj('asps_wrl_coll.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
		else
			texture('asps_skin.png')
			use_material('cv_0')
			load_obj('asps_wheel_r_l_1.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))

			set_material('non_cv',.63,.7,.83,1,.83,.9,1.03,30)
			use_material('non_cv')
			load_obj('asps_wheel_r_l_0.obj',Matrix.rotate(.5*math.pi,v(0,1,0)))
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			selector2()
			if select2 < 51 then
				set_material('cv_0', .63,.7,.83,1,.83,.9,1.03,30)
			else
				set_material('cv_0', get_arg_material(0))
			end
		end
	end
})

define_model('asps_pyl_open', {
	info = {
		bounding_radius = 2,
		materials = {'cut', 'metal'},
	},
	static = function(lod)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
		set_material('cut', .63,.7,.83,.99,.83,.9,1.03,30)

		texture('asps_pyl_open.png')
		use_material('metal')
		load_obj('asps_pylon.obj')

		use_material('cut')
		zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('asps_pyl_open.obj')
		zbias(0)
	end
})

local LASER_SCALE = {
	PULSECANNON_1MW       =  1/7.5,
	PULSECANNON_DUAL_1MW  =  2/7.5,
	PULSECANNON_2MW       =  3/7.5,
	PULSECANNON_RAPID_2MW =  4/7.5,
	PULSECANNON_4MW       =  5/7.5,
	PULSECANNON_10MW      =  6/7.5,
	PULSECANNON_20MW      =  7/7.5,
	MININGCANNON_17MW     =  8/7.5,
	SMALL_PLASMA_ACCEL    =  9/7.5,
	LARGE_PLASMA_ACCEL    = 10/7.5,
}

define_model('asps_sub0', {
	info = {
		lod_pixels = {.1,20,50,0},
		bounding_radius = 4,
		materials = {'cv_0', 'chrome', 'non_cv', 'metal', 'squad'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('non_cv', .63,.7,.83,1,.83,.9,1.03,30)
		set_material('metal', .15,.16,.18,1,.22,.25,.25,10)

		texture('asps_skin.png')
		use_material('squad')
		load_obj('asps_squad_c.obj')

		use_material('cv_0')
		load_obj('asps_cv_0.obj')
		texture(nil)
	end,
	dynamic = function(lod)
		selector1()
		if select1 < 201 then
			set_material('squad', .5,0,0,1,.6,.6,.6,30)
		else
			if select1 < 401 then
				set_material('squad', .45,.35,.01,1,.6,.6,.6,30)
			else
				if select1 < 601 then
					set_material('squad', 0,.15,.7,1,.6,.6,.6,30)
				else
					if select1 < 801 then
						set_material('squad', .06,.35,0,1,.6,.6,.6,30)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,1,.6,.6,.6,30)
						end
					end
				end
			end
		end

		if lod > 2 then
			if get_equipment('LASER',1) then
				local scale = LASER_SCALE[get_equipment('LASER',1)] or 0.1
				local pos = v(.003,.246,-1.681)
				call_model('asps_gun_f', pos, v(1,0,0), v(0,1,0), 1+scale/4)
			end
			if get_equipment('LASER',2) then
				local scale = LASER_SCALE[get_equipment('LASER',2)] or 0.1
				local pos = v(.001,-.431,1.964)
				call_model('asps_gun_r', pos, v(1,0,0), v(0,1,0), 1+scale/4)
			end
		end

		if lod > 3 then
			local M_0 = v(.34,.08,-1.4)
			if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_0,v(1,0,0),v(0,1,0),.1)
				call_model('asps_pyl_open',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('d_guided',M_0,v(1,0,0),v(0,1,0),.1)
					call_model('asps_pyl_open',v(0,0,0),v(1,0,0),v(0,1,0),1)
				else
					if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
						call_model('d_smart',M_0,v(1,0,0),v(0,1,0),.1)
						call_model('asps_pyl_open',v(0,0,0),v(1,0,0),v(0,1,0),1)
					else
						if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
							call_model('d_naval',M_0,v(1,0,0),v(0,1,0),.1)
							call_model('asps_pyl_open',v(0,0,0),v(1,0,0),v(0,1,0),1)
						end
					end
				end
			end

			selector2()
			if select2 < 51 then
				set_material('cv_0', .63,.7,.83,1,.83,.9,1.03,30)
				use_material('cv_0')
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_-',v(1,.57,.38),v(1,0,0),v(0,1,0),.15)
					call_model('antenna_1',v(-.83,.272,-1.94),v(1,0,0),v(0,1,0),.15)
				end
				if get_equipment('ECM') == 'ECM_BASIC' then
					call_model('ecm_1',v(-1,.57,.38),v(1,0,0),v(0,1,0),.1)
				else
					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2',v(-1,.57,.38),v(1,0,0),v(0,1,0),.1)
					end
				end
			else
				set_material('cv_0', get_arg_material(0))
				use_material('cv_0')
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_+',v(-1,.57,.38),v(1,0,0),v(0,1,0),.15)
					call_model('antenna_1',v(.83,.272,-1.94),v(1,0,0),v(0,1,0),.15)
				end
				if get_equipment('ECM') == 'ECM_BASIC' then
					call_model('ecm_1',v(1,.57,.38),v(1,0,0),v(0,1,0),.1)
				else
					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2',v(1,.57,.38),v(1,0,0),v(0,1,0),.1)
					end
				end
			end
		end
	end
})

define_model('asp_sparks', {
	info = {
		scale = 8.5,
		lod_pixels = {.1,20,50,0},
		bounding_radius = 38,
		materials = {'chrome', 'non_cv', 'metal', 'layer', 'win', 'glow_0', 'e_glow', 'scoop', 'text'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Asp Explorer',
				forward_thrust = -220e5,
				reverse_thrust = 100e5,
				up_thrust = 60e5,
				down_thrust = -60e5,
				left_thrust = -60e5,
				right_thrust = 60e5,
				angular_thrust = 600e5,
				gun_mounts =
				{
					{ v(0,2.57,-21.35), v(0,0,-1) },
					{ v(0,-4.42,26.04), v(0,0,1) },
				},
				max_cargo = 120,
				max_missile = 1,
				max_laser = 2,
				max_cargoscoop = 0,
				max_fuelscoop = 1,
				capacity = 120,
				hull_mass = 60,
				fuel_tank_mass = 40,
				thruster_fuel_use = 0.00014,
				price = 187000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod == 1 then
			load_obj('asps_coll.obj')
		else
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('non_cv', .63,.7,.83,1,.83,.9,1.03,30)
			set_material('metal', .15,.16,.18,1,.22,.25,.25,10)
			set_material('layer', .63,.7,.83,.99,.83,.9,1.03,30)
			set_material('glow_0', .5,.5,.5,1,1,1,1,100,.5,1.4,1.5)
			set_material('win', 0,0,.01,1,1,1,2,100)
			--set_material('text', .6,.6,.6,1,.3,.3,.3,5)

			call_model('asps_sub0',v(0,0,0),v(1,0,0),v(0,1,0),1)

			texture('asps_skin.png')
			use_material('non_cv')
			load_obj('asps_ncv.obj')

			use_material('metal')
			load_obj('asps_matte.obj')

			use_material('chrome')
			load_obj('asps_chrome.obj')

			use_material('glow_0')
			load_obj('asps_glow_0.obj')

			if lod > 2 then
				use_material('layer')
				zbias(1,v(0,0,0),v(0,1,0))
				load_obj('asps_layer_1.obj')
				zbias(0)
			end

			texture('asps_scoop.png')
			use_material('scoop')
			load_obj('asps_scoop.obj')

			texture('asps_glow.png')
			use_material('e_glow')
			load_obj('asps_e_glow.obj')

			texture(nil)
			use_material('win')
			load_obj('asps_win.obj')

			if lod > 3 then
				zbias(1,v(-1.9,.401,.45),v(0,1,.36))
				call_model('decal',v(-1.9,.401,.45),v(0,1,.36),v(1,.25,1),.3)
				zbias(0)

				call_model('posl_green',v(2.688,.092,1.028),v(0,1,0),v(1,0,0),.2)
				call_model('posl_red',v(-2.686,.093,1.028),v(0,1,0),v(-1,0,0),.2)
				call_model('coll_warn',v(0,.39,2.5),v(1,0,0),v(0,1,0),.2)
			end

			local M_T  = v(0,0,2.822)
			local R_T1 = v(.24,-.436,-.288)
			local R_T2 = v(.486,-.437,-.218)

			local TF_T = v(1.022,.441,-1.341)
			local TB_T = v(1.961,.245,1.883)

			local BF_T = v(1.03,-.041,-1.354)
			local BB_T = v(1.958,-.125,2.103)

			local RF_T = v(1.604,.282,-.852)
			local RB_T = v(2.408,.171,.861)
			local LF_T = v(-1.602,.283,-.852)
			local LB_T = v(-2.405,.172,.858)

			call_model('blank',v(0,0,0),v(1,0,0),v(0,1,0),0)

			thruster(M_T,v(0,0,1),4, true)
			xref_thruster(R_T1,v(0,0,-1),1, true)
			xref_thruster(R_T2,v(0,0,-1),1, true)

			xref_thruster(TF_T,v(0,1,0),.7)
			xref_thruster(TB_T,v(0,1,0),.7)

			xref_thruster(BF_T,v(0,-1,0),.7)
			xref_thruster(BB_T,v(0,-1,0),.7)

			thruster(RF_T,v(1,0,0),.7)
			thruster(LF_T,v(-1,0,0),.7)
			thruster(RB_T,v(1,0,0),.7)
			thruster(LB_T,v(-1,0,0),.7)
		end
	end,
	dynamic = function(lod)
		local flap = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
		local rot = .5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'),.3,1)-.46

		if lod > 1 then
			set_material('e_glow', lerp_materials(get_time('SECONDS')*.4,{0, 0, 0, 1, 0, 0, 0, 1, .5, 2, 2.5 },
			{0, 0, 0, 1, 0, 0, 0, 1, 1, 2.5, 2.5 }))

			if get_equipment('FUELSCOOP') == 'FUEL_SCOOP' then
				set_material('scoop', lerp_materials(get_time('SECONDS')*.4,{0, 0, 0, 1, 0, 0, 0, 1, .5, 2, 2.5 },
				{0, 0, 0, 1, 0, 0, 0, 1, 1, 2.5, 2.5 }))
			else
				set_material('scoop', .15,.16,.18,1,.22,.25,.25,10)
			end

			texture(nil)
			--use_material('text')
			if lod > 3 then
				call_model('squad_color',v(0,0,0),v(1,0,0),v(0,1,0),1)
				local reg = get_label()
				zbias(1,v(1.764,.448,.453),v(.2,1,0))
				text(reg,v(1.764,.448,.453),v(.2,1,0),v(-1,.37,-1),.2,{center = true})
				zbias(1,v(-1.89,-.282,.75),v(-.4,-1,0))
				text(reg,v(-1.89,-.282,.75),v(-.4,-1,0),v(0,.18,1),.2,{center = true})
				zbias(0)
			end

			use_material('non_cv')

			call_model('asps_flap_r',v(.95,-.398,1.619),v(1,0,.27+math.sin(.1*flap)),v(0,1,-math.sin(.1*flap)),1)
			call_model('asps_flap_l',v(-.943,-.398,1.618),v(1,0,-.27-math.sin(.1*flap)),v(0,1,-math.sin(.1*flap)),1)
		end

		call_model('asps_wheel_f',v(.002,-.252,-.659),v(1,0,0),v(0,math.cos(1.6*rot),math.sin(1.6*rot)),1)

		call_model('asps_wheel_r_r',v(1.615,-.168,1.754),v(0,math.sin(.25*rot),1),v(-math.sin(1.7*rot),math.cos(1.7*rot),0),1)
		call_model('asps_wheel_r_l',v(-1.607,-.164,1.752),v(0,math.sin(.25*rot),1),v(math.sin(1.7*rot),math.cos(1.7*rot),0),1)

		if lod > 2 then
			if get_animation_position('WHEEL_STATE') ~= 0 then
				billboard('smoke.png', .05, v(1,1,1), { v(-.299,.067,-1.687),v(-.416,.067,-1.687) })
				billboard('smoke.png', .025, v(.6,1.2,1.2), { v(-.202,.045,-1.659) })
			end
		end
	end
})
