define_model('lub_c0', {                   -- plain,
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_0g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_c1', {                   -- red
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_1g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_c2', {                   -- yellow
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_2g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_c3', {                   -- blue
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_3g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_c4', {                   -- green
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_4g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_c5', {                   -- crimson
	info =	{
		lod_pixels = {20, 50, 100, 0},
		bounding_radius = 40,
	},
	static = function(lod)
		texture('lanner_5g.png')
		load_obj('deggel.obj')
	end
})

define_model('lub_gun1', {
	info =	{
		lod_pixels = {10, 30, 100, 0},
		materials = {'chrome', 'hole'},
		bounding_radius = 40,
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
		use_material('chrome')
		texture('iise_g.png')
		tapered_cylinder(3*lod, v(0,-2.5,-33.5), v(0,-2.5,-39), v(0,1,0), .25, .15)
		sphere_slice(3*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,-2.4,-33.5)) * Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(.6,.6,.7)))
		if lod > 3 then
			set_material('hole', 0,0,0,1,0,0,0,0)
			use_material('hole')
			xref_circle(3*lod, v(0,-2.5,-39.001), v(0,0,-1), v(0,1,0), .12)
		end
	end
})

define_model('lub_gun2', {
	info =	{
		lod_pixels = {10, 30, 100, 0},
		materials = {'chrome', 'hole'},
		bounding_radius = 40,
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
		use_material('chrome')
		texture('iise_g.png')
		xref_tapered_cylinder(3*lod, v(4,-1.9,-35), v(4,-1.9,-39), v(0,1,0), .25, .15)
		sphere_slice(3*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(3.9,-1.8,-35)) * Matrix.rotate(0.75*math.pi,v(0,.2,-1)) * Matrix.scale(v(.5,.5,.6)))
		sphere_slice(3*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(-3.9,-1.8,-35)) * Matrix.rotate(0.75*math.pi,v(0,-.2,1)) * Matrix.scale(v(.5,.5,.6)))
		if lod > 3 then
			set_material('hole', 0,0,0,1,0,0,0,0)
			use_material('hole')
			xref_circle(3*lod, v(4,-1.9,-39.001), v(0,0,-1), v(0,1,0), .12)
		end
	end
})

define_model('lub_body', {
	info =	{
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 40,
		materials = {'lanner', 'cv0', 'matte', 'text1', 'text2'},
	},
	static = function(lod)
		set_material('lanner', .65,.7,.8,1,.7,.8,1,50)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)

	end,

	dynamic = function(lod)
		local reg = get_label()
		--local select2 = 10
		selector2()
		if select2 < 15 then  -- plain color variable
			set_material('cv0', get_arg_material(0))
			use_material('cv0')
			call_model('lub_c0', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('squadsign_4', v(7.57,1.2,-27.2), v(1,0,-.475), v(-1,.99,.401), 2)

			set_material('text1', .7,.7,.7,.8,0,0,0,0)
			set_material('text2', 0,0,0,.8,0,0,0,0)
			use_material('text1')
			text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
			use_material('text2')
			text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
		else
			if select2 < 29 then  -- plain
				use_material('lanner')
				call_model('lub_c0', v(0,0,0), v(1,0,0), v(0,1,0),1)
				call_model('squadsign_1', v(7.57,1.2,-27.2), v(1,0,-.475), v(-1,.99,.401), 2)

				set_material('text1', 0,0,0,.8,0,0,0,0)
				use_material('text1')
				text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
				text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
			else
				if select2 < 43 then  -- red
					use_material('lanner')
					call_model('lub_c1', v(0,0,0), v(1,0,0), v(0,1,0),1)

					set_material('text1', .2,.005,.01,.8,0,0,0,0)
					set_material('text2', 0,0,0,.8,0,0,0,0)
					use_material('text1')
					text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
					use_material('text2')
					text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
				else
					if select2 < 57 then  -- yellow
						use_material('lanner')
						call_model('lub_c2', v(0,0,0), v(1,0,0), v(0,1,0),1)

						set_material('text1', .3,.2,0,.8,0,0,0,0)
						set_material('text2', 0,0,0,.8,0,0,0,0)
						use_material('text1')
						text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
						use_material('text2')
						text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
					else
						if select2 < 71 then  -- blue
							use_material('lanner')
							call_model('lub_c3', v(0,0,0), v(1,0,0), v(0,1,0),1)

							set_material('text1', 0,0,.3,.8,0,0,0,0)
							set_material('text2', 0,0,0,.8,0,0,0,0)
							use_material('text1')
							text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
							use_material('text2')
							text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
						else
							if select2 < 85 then  -- green
								use_material('lanner')
								call_model('lub_c4', v(0,0,0), v(1,0,0), v(0,1,0),1)

								set_material('text1', 0,.13,.03,.8,0,0,0,0)
								set_material('text2', 0,0,0,.8,0,0,0,0)
								use_material('text1')
								text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
								use_material('text2')
								text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
							else
								if select2 > 84 then  -- crimson
									use_material('lanner')
									call_model('lub_c5', v(0,0,0), v(1,0,0), v(0,1,0),1)

									set_material('text1', .12,0,.25,.8,0,0,0,0)
									set_material('text2', 0,0,0,.8,0,0,0,0)
									use_material('text1')
									text(reg, v(27.2,-3.7,11), v(1,.98,0), v(-.168,0,-1),3, {center=true})
									use_material('text2')
									text(reg, v(-24,-5,10), v(0.1,-1,0), v(.17,-.03,-1),3, {center=true})
								end
							end
						end
					end
				end
			end
		end
		if lod > 2 then
			selector4()
			if select4 > 49 then
				call_model('decal', v(-8.705,1.2,-25.2), v(-1,0,-.475), v(1,.99,.401), 2.5)
				use_material('matte')
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('antenna_1', v(0,-.9,-37.98), v(1,0,0), v(0,1,0), 1.5)
				end
				if get_equipment('LASER', 1) then
					call_model('lub_gun2', v(0,0,0), v(1,0,0), v(0,1,0), 1)
				end
			else
				use_material('matte')
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('antenna_1', v(3.15,-1.6,-37.5), v(1,0,0), v(0,1,0), 1.5)
				end
				if get_equipment('LASER', 1) then
					call_model('lub_gun1', v(0,0,0), v(1,0,0), v(0,1,0), 1)
				end
			end
			if get_equipment('ECM') == 'ECM_BASIC' then
				call_model('ecm_1', v(-23.2,-5.09,2), v(-1,0,0), v(.1,-1,0.01), 1.5)
			end
			if get_equipment('ECM') == 'ECM_ADVANCED' then
				call_model('ecm_2', v(-23.2,-5.09,2), v(-1,0,0), v(.1,-1,0.01), 1.5)
			end
			if get_equipment('SCANNER') == 'SCANNER' then
				call_model('scanner_-', v(23.2,-4.8,2), v(-1,0,0), v(.1,-1,0.01), 2)
			end
		end
	end
})

define_model('lub_cage', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 20,
		materials = {'cage', 'chrome'}
	},
	static = function(lod)
		set_material('cage', .19,.21,.22,1,.2,.2,.2,10)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
		texture('iise_g.png')
		use_material('cage')
		load_obj('schacht.obj')

		use_material('chrome')
		load_obj('maschine.obj')
	end
})

define_model('lub_flap_bl_l', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'matte'}
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')
		texture('klappe_h_g.png')
		load_obj('klappe_hl_l.obj', Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('lub_flap_bl_r', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'matte'}
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')
		texture('klappe_h_g.png')
		load_obj('klappe_hl_r.obj', Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('lub_flap_br_l', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'matte'}
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')
		texture('klappe_h_g.png')
		load_obj('klappe_hr_l.obj', Matrix.rotate(0.5*math.pi, v(0,1,0)))
	end
})

define_model('lub_flap_br_r', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'matte'}
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')
		texture('klappe_h_g.png')
		load_obj('klappe_hr_r.obj', Matrix.rotate(0.5*math.pi, v(0,1.0)))
	end
})

define_model('lub_flap_ff', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'lanner'}
	},

	static = function(lod)
		set_material('lanner', .65,.7,.8,1,.7,.8,1,50)
		use_material('lanner')
		texture('klappe_h_g.png')
		load_obj('klappe_vv.obj')
	end
})

define_model('lub_flap_fb', {
	info = {
		lod_pixels = {10, 30, 100, 0},
		bounding_radius = 10,
		materials = {'lanner'}
	},
	static = function(lod)
		set_material('lanner', .65,.7,.8,1,.7,.8,1,50)
		use_material('lanner')
		texture('klappe_h_g.png')
		load_obj('klappe_vh.obj')
	end
})

define_model('lub_fcyl', {
	info = {
		lod_pixels = { 10, 30 ,100, 0 },
		bounding_radius = 5,
		materials = {'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
		use_material('chrome')
		texture('iise_g.png')
		sphere_slice(3*lod, 2*lod, 0, .6*math.pi, Matrix.scale(v(0.75,0.75,0.75)) * Matrix.rotate(math.pi, v(0,0,1)))
	end,
	dynamic = function(lod)
		use_material('chrome')
		texture('models/ships/lanner_ub/iise_g.png', v(0,0,0), v(0,1,0), v(1,0,0))
		local uc_trans = math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		ring(3*lod, v(0,0,0), v(0,-4*(uc_trans),0), v(0,0,1), .2)
		ring(3*lod, v(0,0,0), v(0,-6*(uc_trans),0), v(0,0,1), .15)
		ring(3*lod, v(0,0,0), v(0,-7.5*(uc_trans-0.05),0), v(0,0,1), .1)
	end
})

define_model('lub_bcyl', {
	info = {
		lod_pixels = { 10, 30 ,100, 0 },
		bounding_radius = 5,
		materials = {'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
		use_material('chrome')
		texture('iise_g.png')
		sphere_slice(3*lod, 2*lod, 0, .6*math.pi, Matrix.translate(v(8.7,0,0)) * Matrix.scale(v(0.75,0.75,0.75)) * Matrix.rotate(math.pi, v(0,0,1)))
		sphere_slice(3*lod, 2*lod, 0, .6*math.pi, Matrix.translate(v(-8.7,0,0)) * Matrix.scale(v(0.75,0.75,0.75)) * Matrix.rotate(math.pi, v(0,0,1)))
	end,
	dynamic = function(lod)
		use_material('chrome')
		texture('models/ships/lanner_ub/iise_g.png', v(0,0,0), v(0,1,0), v(1,0,0))
		local uc_trans = math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		xref_ring(3*lod, v(8.7,0,0), v(8.7,-3*(uc_trans),0), v(0,0,1), .2)
		xref_ring(3*lod, v(8.7,0,0), v(8.7,-4.5*(uc_trans),0), v(0,0,1), .15)
		xref_ring(3*lod, v(8.7,0,0), v(8.7,-5.9*(uc_trans),0), v(0,0,1), .1)
	end
})

define_model('lub_wf', {
	info = {
		bounding_radius = 10,
		materials = {'matte'},
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')

		texture('redli.png')
		load_obj('redli_v.obj')
	end
})

define_model('lub_ucf', {
	info = {
		bounding_radius = 10,
		materials = {'matte'},
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)

		use_material('matte')
		texture('iise_g.png')
		load_obj('glengg_v.obj')
	end,
	dynamic = function(lod)
		local w_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		call_model('lub_wf', v(0,0,7.85), v(1,0,0), v(0,math.sin(w_rot),math.cos(w_rot)-0.3),1)
	end
})

define_model('lub_wb', {
	info = {
		bounding_radius = 10,
		materials = {'matte'},
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
		use_material('matte')

		texture('iise_g.png')
		load_obj('henggi.obj')

		texture('redli.png')
		load_obj('redli_h.obj')
	end
})

define_model('lub_ucb', {
	info = {
		bounding_radius = 10,
		materials = {'matte'},
	},
	static = function(lod)
		set_material('matte', .5,.52,.55,1,.2,.2,.2,10)

		use_material('matte')
		texture('iise_g.png')
		load_obj('glengg_h.obj')
	end,

	dynamic = function(lod)
		local w_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)
		call_model('lub_wb', v(0,0,-5.5), v(1,0,0), v(0,math.cos(w_rot),math.sin(w_rot)-0.3),1)
	end
})

define_model('lanner_ub', {
	info = {
		scale = 1, --1.1 = fit to original FFE scale
		lod_pixels = {.1, 30, 100, 0},
		bounding_radius = 50,
		materials = {'default', 'matte', 'glow', 'e_glow', 'win'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Lanner',
				forward_thrust = -30e6,
				reverse_thrust = 10e6,
				up_thrust = 10e6,
				down_thrust = -5e6,
				left_thrust = -5e6,
				right_thrust = 5e6,
				angular_thrust = 90e6,
				gun_mounts =
				{
				{v(0,-1.9,-38), v(0,0,-1)},
				{v(0,1,38), v(0,0,1)},
				},
				max_cargo = 190,
				max_laser = 2,
				max_missile = 4,
				max_cargoscoop = 0,
				capacity = 190,
				hull_mass = 130,
				fuel_tank_mass = 60,
				thruster_fuel_use = 0.00025,
				price = 280000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod == 1 then
			local  v1 = v(-3.5,-2,-38)
			local  v2 = v(3.5,-2,-38)
			local  v3 = v(2,0.5,-38)
			local  v4 = v(-2,0.5,-38)
			local  v5 = v(13,-3,-24)
			local  v6 = v(3,4,-24)
			local  v7 = v(-3,4,-24)
			local  v8 = v(27,-5.5,0)
			local  v9 = v(12,5,0)
			local v10 = v(-12,5,0)
			local v11 = v(30,-5.5,19)
			local v12 = v(20,4,19)
			local v13 = v(-20,4,19)
			local v14 = v(0,1,38)
			local v15 = v(6,3,25)
			local v16 = v(-6,3,25)
			local v17 = v(-13,-3,-24)
			local v18 = v(-27,-5.5,0)
			local v19 = v(-30,-5.5,19)
			local v20 = v(-6,-2,25)
			local v21 = v(6,-2,25)
			quad(v1,v4,v3,v2)
			xref_quad(v2,v3,v6,v5)
			xref_quad(v5,v6,v9,v8)
			xref_quad(v8,v9,v12,v11)
			quad(v4,v7,v6,v3)
			quad(v6,v7,v10,v9)
			quad(v10,v13,v12,v9)
			quad(v12,v13,v16,v15)
			tri(v15,v16,v14)
			quad(v1,v2,v5,v17)
			quad(v17,v5,v8,v18)
			quad(v18,v8,v11,v19)
			quad(v19,v11,v21,v20)
			tri(v20,v21,v14)
			xref_quad(v20,v16,v13,v19)
			xref_tri(v20,v14,v16)
		end

		if lod > 1 then
			set_material('default', .65,.7,.8,1,.7,.8,1,50)
			set_material('matte', .5,.52,.55,1,.2,.2,.2,10)
			set_material('win', 0,0,.1,1,1,1,2,100)

			use_material('default')
			texture('bode_g.png')
			load_obj('bode.obj')

			use_material('matte')
			texture('matt_g.png')
			load_obj('matt.obj')

			texture('luechte_g.png')
			use_material('glow')
			load_obj('luechte.obj')

			if lod > 2 then
				texture('duese.png')
				use_material('e_glow')
				load_obj('duese.obj')
			end

			texture(nil)
			use_material('win')
			quad(v(.769,3.301,-27.94),v(.769,2.522,-27.94),v(-.773,2.522,-27.94),v(-.773,3.301,-27.94))

			call_model('lub_body', v(0,0,0), v(1,0,0), v(0,1,0),1)
		end
	end,
	dynamic = function(lod)
		set_material('glow', lerp_materials(get_time('SECONDS')*0.3, {0, 0, 0, 1, 0, 0, 0, 0, 1.6, 1.9, 0 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, 2.5,0 }))
		set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }))

		local flap = 1.25*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, 0.4)
		local flap_f = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
		local flap_re = 1.25*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
		local uc_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)  -- uc factor
		local uc_trans = math.clamp(get_animation_position('WHEEL_STATE'), 0.2, 1)

		if lod > 1 then
			call_model('lub_flap_bl_l', v(-10.789,-5.66,2.842), v(0,0,1), v(math.sin(flap),math.cos(flap),0), 1)
			call_model('lub_flap_bl_r', v(-5.711,-5.66,2.842), v(0,0,1), v(-math.sin(flap),math.cos(flap),0), 1)
			call_model('lub_flap_br_l', v(5.712,-5.66,2.842), v(0,0,1), v(math.sin(flap),math.cos(flap),0), 1)
			call_model('lub_flap_br_r', v(10.81,-5.66,2.842), v(0,0,1), v(-math.sin(flap),math.cos(flap),0), 1)
			call_model('lub_flap_ff', v(0,-2.855,-22.949), v(1,0,0), v(0,math.cos(1.1*flap_f),math.sin(1.1*flap_f)), 1)

			if get_animation_position('WHEEL_STATE') >= 0.6 then
				call_model('lub_flap_fb', v(0,-3.267,-13.22), v(1,0,0), v(0,-math.sin(1.1*flap_re+0.4),math.cos(1.1*flap_re+0.4)), 1)
			else
				call_model('lub_flap_fb', v(0,-3.267,-13.22), v(1,0,0), v(0,math.cos(1.1*flap),-math.sin(1.1*flap)), 1)
			end

			if get_animation_position('WHEEL_STATE') ~= 0 then
				call_model('lub_cage', v(0,0,0), v(1,0,0), v(0,1,0), 1)
				call_model('lub_ucf', v(0,-1.5,-22.5), v(1,0,0), v(0,math.cos(uc_rot),math.sin(uc_rot)-0.3), 1)
				call_model('lub_fcyl', v(-0.4,-.3,-18.1), v(1,0,0), v(0,math.cos(0.57*uc_rot),math.sin(0.57*uc_rot)-0.3), 1)
				call_model('lub_ucb', v(0,-3.2,4.9), v(1,0,0), v(0,math.cos(uc_rot),-math.sin(uc_rot)+0.3), 1)
				call_model('lub_bcyl', v(0,-2.1,1.4), v(1,0,0), v(0,math.cos(0.565*uc_rot),-math.sin(0.565*uc_rot)+0.3), 1)
			end
		end

		if lod >= 2 then
			call_model('posl_white', v(0,5.65,15), v(1,0,0), v(0,1,0), 2)   -- load lights after any else to grant not to get hidden
			call_model('posl_green', v(27.15,-5.3,1), v(0,1,0), v(1,1,-0.1), 2)
			call_model('posl_red', v(-27.15,-5.3,1), v(0,1,0), v(-1,1,-0.1), 2)
			call_model('headlight', v(2.4,-0.9,-38), v(1,0,0), v(0,0,-1), 2)
			call_model('headlight', v(-2.4,-0.9,-38), v(1,0,0), v(0,0,-1), 2)
			call_model('coll_warn', v(0,-1.2,31.5), v(1,0,0), v(0,-1,0.1), 2)
			call_model('coll_warn', v(27.15,-5.45,1), v(1,0,0), v(-0.11,-1,0), 2)
			call_model('coll_warn', v(-27.15,-5.45,1), v(1,0,0), v(0.11,-1,0), 2)
		end

		if lod == 1 then
			if get_animation_position('WHEEL_STATE') ~= 0 then
				cylinder(4, v(0,-2-8*uc_trans,-20), v(0,-2-8*uc_trans,-24), v(0,1,0), 1)
				xref_cylinder(4, v(9,-2-8*uc_trans,1), v(9,-2-8*uc_trans,9), v(0,1,0), 1)
			end
		end

		local M_T = v(8.2,-4.3,22) -- best set thrusters dynamic, so they don't "vanish" in certain angles

		local R_T = v(17.65,0.1,-14.5)
		local BFR_T = v(11.5,-3.2,-23.23)
		local BBR_T = v(27.5,-5.2,15.95)
		local TFR_T = v(4.68,5,-13.86)
		local TBR_T = v(14.23,4.9,16.3)
		local RF_T = v(9.2,0.062,-24.93)
		local LF_T = v(-9.2,0.062,-24.93)
		local RB_T = v(2.2,-0.26,31.5)
		local LB_T = v(-2.2,-0.26,31.5)

		xref_thruster(M_T, v(0,0,1), 25, true)
		xref_thruster(R_T, v(0,0,-1), 10, true)
		xref_thruster(BFR_T, v(0,-1,0), 5)
		xref_thruster(BBR_T, v(0,-1,0), 5)
		xref_thruster(TFR_T, v(0,1,0), 5)
		xref_thruster(TBR_T, v(0,1,0), 5)
		thruster(RF_T, v(1,0,0), 5)
		thruster(LF_T, v(-1,0,0), 5)
		thruster(RB_T, v(1,0,0), 5)
		thruster(LB_T, v(-1,0,0), 5)
	end
})
