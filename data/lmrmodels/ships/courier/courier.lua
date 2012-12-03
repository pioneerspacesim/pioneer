-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

-- model & textures, gernot

define_model('courier_tip', {     -- engine tips
	info = {
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials={'courier', 'glow', 'grey'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		set_material('glow', 0,0,0,1,1,1,1.5,100,1.5,2,1)
		set_material('grey', .5,.5,.5,1,.3,.3,.3,10)

		texture('courier.png')
		use_material('courier')
		load_obj('c_tip1.obj')

		if lod > 2 then
			load_obj('c_nazzle.obj')
			load_obj('c_tip_n_out.obj')

			use_material('grey')
			load_obj('c_tip_n_in.obj')
		end

		use_material('glow')
		load_obj('c_tip2.obj')

		local vMainThruster = v(0,-15,0)
		local vRetroThruster1 = v(3.5,6.3,0)
		local vRetroThruster2 = v(-3.5,6.3,0)
		local vRetroThruster3 = v(0,6.3,3.5)
		local vRetroThruster4 = v(0,6.3,-3.5)

		thruster(vMainThruster, v(0,-1,0), 30, true)
		thruster(vRetroThruster1, v(0,1,0), 6, true)
		thruster(vRetroThruster2, v(0,1,0), 6, true)
		thruster(vRetroThruster3, v(0,1,0), 6, true)
		thruster(vRetroThruster4, v(0,1,0), 6, true)
	end
})

define_model('courier_lwp5', {  -- leftwing part 5 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		load_obj('c_lwp_5.obj')
	end
})

define_model('courier_rwp5', {  -- rightwing part 5 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		load_obj('c_rwp_5.obj')
	end
})

define_model('courier_lwp4', {  -- leftwing part 4 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(1,v(0,0,0),v(0,0,0)) -- each wing part needs a higher zbias, to avoid z-fighting when wings are fold in
		load_obj('c_lwp_4.obj')
	end
})

define_model('courier_rwp4', {  -- rightwing part 4 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(1,v(0,0,0),v(0,0,0))
		load_obj('c_rwp_4.obj')
	end
})

define_model('courier_lwp3', {  -- leftwing part 3 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,0,0))
		load_obj('c_lwp_3.obj')
	end
})

define_model('courier_rwp3', {  -- rightwing part 3 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,0,0))
		load_obj('c_rwp_3.obj')
	end
})

define_model('courier_lwp2', {  -- leftwing part 2 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(3,v(0,0,0),v(0,0,0))
		load_obj('c_lwp_2.obj')
	end
})

define_model('courier_rwp2', {  -- rightwing part 2 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(3,v(0,0,0),v(0,0,0))
		load_obj('c_rwp_2.obj')
	end
})

define_model('courier_top', {    -- courier top
	info =	{
			bounding_radius = 95,
			materials = {'courier'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1, 1,1.7,1.8,2,30)

		use_material('courier')
		texture('courier.png')
		zbias(4,v(0,0,0),v(0,1,0)) -- also the body needs a higher zbias rather the wingparts to avoid z-fighting
		load_obj('c_top.obj')
	end
})

define_model('trader_top', {    -- trader top
	info =	{
			bounding_radius = 95,
			materials = {'courier'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1, 1,1.7,1.8,2,30)

		use_material('courier')
		texture('courier.png')
		zbias(4,v(0,0,0),v(0,1,0))
		load_obj('t_top.obj')
	end
})

define_model('courier_body', {    -- courier shell
	info =	{
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials = {'courier','text'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		set_material('text', .7,.7,.7,.7,.3,.3,.3,10)

		call_model('courier_top', v(0,0,0), v(1,0,0), v(0,1,0),1)

		use_material('courier')
		texture('courier.png')
		zbias(4,v(0,0,0),v(0,-1,0))
		load_obj('c_bot.obj')
		zbias(0)

		if lod > 2 then
			call_model('imp_sign_1', v(-9,1.82,6), v(-.08,1,0), v(0,.02,-1),2.5)
			call_model('imp_sign_1', v(9,0.3,4), v(0.044,-1,0), v(0,0,-1),2.5)
		end
	end,

	dynamic = function(lod)
		if lod > 2 then
				local reg = get_label()
				use_material('text')
				zbias(6,v(0,0,0),v(0,1,0))
				text(reg, v(10, 1.8, 1.3), v(0,1,0), v(1,-0.083,0), 1.3, {center=true})
				zbias(6,v(0,0,0),v(0,-1,0))
				text(reg, v(-8.7, .28, 1.6), v(0,-1,0), v(-1,0.052,0), 1.3, {center=true})
				zbias(0)
		end
	end
})

define_model('trader_body', {    --trader shell
	info =	{
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials = {'courier', 'text'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		set_material('text', .7,.7,.7,.7,.3,.3,.3,10)

		call_model('trader_top', v(0,0,0), v(1,0,0), v(0,1,0),1)

		use_material('courier')
		texture('courier.png')
		zbias(4,v(0,0,0),v(0,-1,0))
		load_obj('c_bot.obj')
		zbias(0)

		if lod > 2 then
			call_model('imp_sign_1', v(-9,1.82,6), v(-.08,1,0), v(0,.02,-1),2.5)
			call_model('imp_sign_1', v(9,0.3,4), v(0.044,-1,0), v(0,0,-1),2.5)
		end
	end,

	dynamic = function(lod)
		if lod > 2 then
				local reg = get_label()
				use_material('text')
				zbias(6,v(0,0,0),v(0,1,0))
				text(reg, v(10, 1.8, 1.3), v(0,1,0), v(1,-0.083,0), 1.3, {center=true})
				zbias(6,v(0,0,0),v(0,-1,0))
				text(reg, v(-8.7, .28, 1.6), v(0,-1,0), v(-1,0.052,0), 1.3, {center=true})
				zbias(0)
		end
	end
})

define_model('courier_flap_ll', {  -- left flap1 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'}
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,-1,0))
		load_obj('c_flap_ll.obj', matrix.rotate(0.5*math.pi, v(-1,0,0)))
		zbias(0)
	end
})

define_model('courier_flap_lr', {  -- left flap2 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'}
			},

	static = function(lod)
	    set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,-1,0))
		load_obj('c_flap_lr.obj', matrix.rotate(0.5*math.pi, v(1,0,0)))
		zbias(0)
	end
})

define_model('courier_flap_rr', {  -- right flap1 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'}
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,-1,0))
		load_obj('c_flap_rr.obj', matrix.rotate(0.5*math.pi, v(1,0,0)))
		zbias(0)
	end
})

define_model('courier_flap_rl', {  -- right flap2 all models
	info = {
			bounding_radius = 95,
			materials = {'courier'}
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		zbias(2,v(0,0,0),v(0,-1,0))
		load_obj('c_flap_rl.obj', matrix.rotate(0.5*math.pi, v(-1,0,0)))
		zbias(0)
	end
})

define_model('courier_wheels_l', {    -- wheels left all models
	info =	{
			bounding_radius = 95,
			materials = {'black', 'courier'},
			},

	static = function(lod)
		set_material('black', .05,.05,.05,1,.3,.3,.3,10)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)

		texture('courier.png')
		use_material('courier')
		load_obj('c_sledge_l.obj')

		use_material('black')
		load_obj('c_wheels_l.obj')
	end
})

define_model('courier_wheels_r', {    -- wheels right all models
	info =	{
			bounding_radius = 95,
			materials = {'black', 'courier'},
			},

	static = function(lod)
		set_material('black', .05,.05,.05,1,.3,.3,.3,10)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)

		texture('courier.png')
		use_material('courier')
		load_obj('c_sledge_r.obj')

		use_material('black')
		load_obj('c_wheels_r.obj')
	end
})

define_model('courier_uc_l', {
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
	end,
	dynamic = function(lod)
		local rot = math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

		call_model('courier_flap_ll', v(-32.419, -13.172,13.141), v(math.cos(-rot),math.sin(-rot),0), v(0.0175,0.0435,1),1)
		call_model('courier_flap_lr', v(-30.749, -13.172,13.141), v(math.cos(rot),math.sin(rot),0), v(0.0175,-0.0435,-1),1)

		if get_animation_position('WHEEL_STATE') ~= 0 then
			local trans1 = 8*math.clamp(get_animation_position('WHEEL_STATE')-.5, 0, 1)
			local trans2 = 18*math.clamp(get_animation_position('WHEEL_STATE')-.7, 0, 1)

			texture('lmrmodels/ships/courier/piston.png',v(.5,.75,0),v(3,0,0),v(0,-.15,0))
			use_material('courier')
			lathe(3*lod,v(-31.584,-11,15),v(-31.584,-12.043-trans1,13.139-trans2),v(1,0,0),{0,.28, .25,.28, .25,.23, .5,.23, .5,.18, .75,.18, .75,.13, 1,.13})
			lathe(3*lod,v(-31.584,-11,9),v(-31.584,-12.043-trans1,13.139-trans2),v(1,0,0), {0,.28, .25,.28, .25,.23, .5,.23, .5,.18, .75,.18, .75,.13, 1,.13})

			call_model('courier_wheels_l',v(0,-trans1,-trans2), v(1,0,0), v(0,1,0), 1)
		end
	end
})

define_model('courier_uc_r', {
	info = {
			bounding_radius = 95,
			materials = {'courier'},
			},
	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
	end,
	dynamic = function(lod)

		local rot = math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

		call_model('courier_flap_rl', v(30.749, -13.172,13.141), v(math.cos(-rot),math.sin(-rot),0), v(0.0175,0.0435,1),1)
		call_model('courier_flap_rr', v(32.419, -13.172,13.141), v(math.cos(rot),math.sin(rot),0), v(0.0175,-0.0435,-1),1)

		if get_animation_position('WHEEL_STATE') ~= 0 then
			local trans1 = 8*math.clamp(get_animation_position('WHEEL_STATE')-.5, 0, 1)
			local trans2 = 18*math.clamp(get_animation_position('WHEEL_STATE')-.7, 0, 1)

			texture('lmrmodels/ships/courier/piston.png',v(.5,.75,0),v(3,0,0),v(0,-.15,0))
			use_material('courier')
			lathe(3*lod,v(31.584,-11,15),v(31.584,-12.043-trans1,13.139-trans2),v(1,0,0),{0,.28, .25,.28, .25,.23, .5,.23, .5,.18, .75,.18, .75,.13, 1,.13})
			lathe(3*lod,v(31.584,-11,9),v(31.584,-12.043-trans1,13.139-trans2),v(1,0,0), {0,.28, .25,.28, .25,.23, .5,.23, .5,.18, .75,.18, .75,.13, 1,.13})

			call_model('courier_wheels_r',v(0,-trans1,-trans2), v(1,0,0), v(0,1,0), 1)
		end
	end
})

define_model('courier_eng_l', {      -- engine part left all models
	info = {
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials={'hole', 'courier', 'e_glow1', 'e_glow2', 'iron', 'nazzle'},
			},
	static = function(lod)
		set_material('hole',0,0,0,0,0,0,0,0)
		set_material('iron', .3,.33,.35,1,.1,.1,.1,10)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)

		texture('courier.png')
		use_material('iron')
		load_obj('c_cage_l.obj')

		call_model('courier_uc_l',v(0,0,0),v(1,0,0),v(0,1,0),1)

		texture(nil)
		use_material('hole')
		zbias(1,v(0,0,0),v(0,-1,0))
		load_obj('c_cage_cut_l.obj')
		zbias(0)

		texture('courier.png')
		use_material('courier')
		load_obj('c_eng_l.obj')

		use_material('e_glow1')
		load_obj('c_e_inside_l.obj')

		use_material('e_glow2')
		load_obj('c_e_glow_l.obj')

		if lod > 2 then
			set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('nazzle')
			call_model('nazzle_n', v(-31.58,-8.9,19), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(-31.58,-13.4,19), v(1,0,0), v(0,-1,0), .3)
			call_model('nazzle_n', v(-29.33,-11.16,19), v(0,1,0), v(1,0,0), .3)
			call_model('nazzle_n', v(-33.83,-11.16,19), v(0,1,0), v(-1,0,0), .3)
		end

		local LeftBackTopThrust = v(-31.58, -8.7, 19)
		local LeftBackBottomThrust = v(-31.58, -13.6, 19)

		thruster(LeftBackTopThrust, v(0,1,0), 5)
		thruster(LeftBackBottomThrust, v(0,-1,0), 5)

		local BackLeftThrust = v(-34.03, -11.16, 19)
		local BackRightThrust = v(-29.13, -11.16, 19)

		thruster(BackLeftThrust, v(-1,0,0), 5)
		thruster(BackRightThrust, v(1,0,0), 5)

		if lod > 2 then
			call_model('posl_red', v(-33.7,-11.156,13.6), v(0,0,1), v(-1,0,0),1.5)
			call_model('coll_warn', v(-33.6,-11.156,15.2), v(0,0,1), v(-1,0,0),1.5)
		end
	end,

	dynamic = function(lod)

		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
	set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }))

		if get_animation_position('WHEEL_STATE') >= .99 then
		    call_model('courier_tip', v(-31.584, -11.156, 6.25), v(0,1,0), v(0,0,-1),1)
		else
		    local rot = math.pi*get_time('SECONDS')/1.5
		    call_model('courier_tip', v(-31.584, -11.156, 6.25), v(math.cos(rot),math.sin(rot),0), v(0,0,-1),1)
		end
	end
})

define_model('courier_eng_r', {      -- engine part right all models
	info = {
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials={'hole', 'courier', 'e_glow1', 'e_glow2', 'iron', 'nazzle'},
		},
	static = function(lod)
	    set_material('hole',0,0,0,0,0,0,0,0)
		set_material('iron', .3,.33,.35,1,.1,.1,.1,10)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)

		texture('courier.png')
		use_material('iron')
		load_obj('c_cage_r.obj')

		call_model('courier_uc_r',v(0,0,0),v(1,0,0),v(0,1,0),1)

		texture(nil)
		use_material('hole')
		zbias(1,v(0,0,0),v(0,-1,0))
		load_obj('c_cage_cut_r.obj')
		zbias(0)

		texture('courier.png')
		use_material('courier')
		load_obj('c_eng_r.obj')

		use_material('e_glow1')
		load_obj('c_e_inside_r.obj')

		use_material('e_glow2')
		load_obj('c_e_glow_r.obj')

		if lod > 2 then
			set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('nazzle')
			call_model('nazzle_n', v(31.58,-8.9,19), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(31.58,-13.4,19), v(1,0,0), v(0,-1,0), .3)
			call_model('nazzle_n', v(29.33,-11.16,19), v(0,1,0), v(-1,0,0), .3)
			call_model('nazzle_n', v(33.83,-11.16,19), v(0,1,0), v(1,0,0), .3)
		end

		local LeftBackTopThrust = v(31.58, -8.7, 19)
		local LeftBackBottomThrust = v(31.58, -13.6, 19)

		thruster(LeftBackTopThrust, v(0,1,0), 5)
		thruster(LeftBackBottomThrust, v(0,-1,0), 5)

		local BackLeftThrust = v(34.03, -11.16, 19)
		local BackRightThrust = v(29.13, -11.16, 19)

		thruster(BackLeftThrust, v(1,0,0), 5)
		thruster(BackRightThrust, v(-1,0,0), 5)

		if lod > 2 then
			call_model('posl_green', v(33.7,-11.156,13.6), v(0,0,1), v(1,0,0),1.5)
			call_model('coll_warn', v(33.6,-11.156,15.2), v(0,0,1), v(1,0,0),1.5)
		end
    end,

	dynamic = function(lod)

		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1.2, 1.5, 2 }, {0, 0, 0, 1, 0, 0, 0, 0, 1.5, 1.2, 2 }))
		set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1.5, 1.2, 2 }, {0, 0, 0, 1, 0, 0, 0, 0, 1.2, 1.5, 2 }))

		if get_animation_position('WHEEL_STATE') >= .99 then
		    call_model('courier_tip', v(31.584, -11.156, 6.25), v(1,0,0), v(0,0,-1),1)
		else
		    local rot = math.pi*get_time('SECONDS')/1.5
		    call_model('courier_tip', v(31.584, -11.156, 6.25), v(math.sin(rot),math.cos(rot),0), v(0,0,-1),1)
		end
	end
})

define_model('trader_eng', {   -- trader middle engine part
	info = {
			bounding_radius = 95,
		materials = {'courier', 'e_glow1', 'e_glow2'},
		},

	static = function(lod)

	set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		use_material('courier')
		texture('courier.png')
		load_obj('t_eng.obj')

		use_material('e_glow2')
		load_obj('t_e_inside.obj')

		use_material('e_glow1')
		load_obj('t_e_glow.obj')
	end,

    dynamic = function(lod)
		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1.2, 1.5, 2 }, {0, 0, 0, 1, 0, 0, 0, 0, 1.5, 1.2, 2 }))
		set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1.5, 1.2, 2 }, {0, 0, 0, 1, 0, 0, 0, 0, 1.2, 1.5, 2 }))

		if get_animation_position('WHEEL_STATE') >= .99 then
		    call_model('courier_tip', v(0,11.467,13.2), v(1,-1,0), v(0,0,-1),1)
		else
		    local rot = math.pi*get_time('SECONDS')/1.5
		    call_model('courier_tip', v(0,11.467,13.2), v(math.sin(rot),math.cos(rot),0), v(0,0,-1),1)
		end
	end
})

define_model('courier_pit', {
	info = {
			bounding_radius = 95,
			materials = {'pit'},
			},

	static = function(lod)
		set_material('pit', .5,.5,.45,1,.3,.3,.3,10)
		set_light(1, 0.05, v(0,3.2,-17), v(6,6,6))
		set_local_lighting(true)
		use_light(1)

		texture('courier.png')
		use_material('pit')
		load_obj('c_pit.obj')

		set_local_lighting(false)
	end
})


define_model('courier_gun', {
	info = {
			bounding_radius = 95,
			},

	static = function(lod)
		texture('courier.png')
		load_obj('c_gun.obj')
	end
})

define_model('courier_sub', {     -- courier sub-model, all models
	info = {
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials = {'courier', 'matte', 'nazzle'},
			},

	static = function(lod)
		set_material('courier', .85,.9,1,1,1.7,1.8,2,30)
		set_material('matte', .3, .3, .3,1, .5, .5, .5, 10)

		use_material('matte')
		texture('courier.png')
		load_obj('c_back.obj')

		if lod > 2 then
			set_material('nazzle', .85,.9,1,1,1.7,1.8,2,10)
			use_material('nazzle') -- needs to be called for each model, else the material gets lost, if that's still true
			call_model('nazzle_n', v(-3.5,1.2,-15), v(1,0,0), v(0,1,0), .3)
			use_material('nazzle')
			call_model('nazzle_n', v(-3.5,.5,-15), v(1,0,0), v(0,-1,0), .3)
			use_material('nazzle')
			call_model('nazzle_n', v(3.5,1.2,-15), v(1,0,0), v(0,1,0), .3)
			use_material('nazzle')
			call_model('nazzle_n', v(3.5,.5,-15), v(1,0,0), v(0,-1,0), .3)
			use_material('nazzle')
			call_model('nazzle_n', v(-4,.82,-15), v(0,1,0), v(-1,0,0), .3)
			use_material('nazzle')
			call_model('nazzle_n', v(4,.82,-15), v(0,1,0), v(1,0,0), .3)
		end

		local LeftFrontTopThrust = v(-3.5,1.4,-15)
		local RightFrontTopThrust = v(3.5,1.4, -15)
		local LeftFrontBottomThrust = v(-3.5,.3, -15)
		local RightFrontBottomThrust = v(3.5,.3, -15)

		thruster(LeftFrontTopThrust, v(0,1,0), 5)
		thruster(RightFrontTopThrust, v(0,1,0), 5)
		thruster(LeftFrontBottomThrust, v(0,-1,0), 5)
		thruster(RightFrontBottomThrust, v(0,-1,0), 5)

		local FrontLeftThrust = v(-4.2,.82, -15)
		local FrontRightThrust = v(4.2,.82, -15)

		thruster(FrontLeftThrust, v(-1,0,0), 5)
		thruster(FrontRightThrust, v(1,0,0), 5)
	end,

	dynamic = function(lod)
		local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
		local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
		local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

		call_model('courier_eng_l', v(trans1*9.5,trans1*8,trans1*-5), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp5', v(trans1*9.5,trans1*7.7,trans1*-4.5), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp4', v(trans1*7.3,trans1*5.9,trans1*-3.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp3', v(trans1*4.95,trans1*4,trans1*-2.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp2', v(trans1*2.4,trans1*1.95,trans1*-1.15), v(1,0,0), v(0,1,0),1)

		call_model('courier_eng_r', v(trans1*-9.5,trans1*8,trans1*-5), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp5', v(trans1*-9.5,trans1*7.7,trans1*-4.5), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp4', v(trans1*-7.3,trans1*5.9,trans1*-3.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp3', v(trans1*-4.95,trans1*4,trans1*-2.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp2', v(trans1*-2.4,trans1*1.95,trans1*-1.15), v(1,0,0), v(0,1,0),1)

		if lod > 2 then
			call_model('headlight', v(0,.48,-23), v(1,0,0), v(0,-1,-.15),1.5) -- keep dynamic, because of hierarchy, when called earlier the "light" will be swallowed by the wings
			call_model('coll_warn', v(0,-1.85,14), v(1,0,0), v(0,-1,0),1.5)

			if get_animation_position('WHEEL_STATE') ~= 0 then -- the bridge gets only processed when the window is transparent
				call_model('courier_pit',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end

			if get_equipment('LASER',1) then
				use_material('courier')
				call_model('courier_gun',v(0,.6,-23.1),v(1,0,0),v(0,1,0),1)
			end

			if get_equipment('SCANNER') == 'SCANNER' then
				call_model('antenna_1',v(13.5,.824,-.5),v(1,0,0),v(0,1,0),2)
			end

			if get_equipment('ECM') == 'ECM_ADVANCED' then
				call_model('ecm_2a',v(-13.5,.824,-.5),v(1,0,0),v(0,1,0),1)
			elseif get_equipment('ECM') == 'ECM_BASIC' then
				call_model('ecm_1a',v(-13.5,.824,-.5),v(1,0,0),v(0,1,0),1)
			end
		end
	end
})

define_model('courier', {
	info = {
		scale = 1.3,
		lod_pixels = {.1, 80, 250, 0},
		materials = {'win'},
		bounding_radius = 85,
		tags = {'ship'},
	},
	static = function(lod)
		if lod == 1 then
--[[
scripted geometry for the collision mesh, i promised to use it for lod2 also,
but texturing gets nearly impossible this way with the used texture sheet.
it's not such a high poly model that it really would be needed to do so
--]]
			local  v1 = v(0,1,-24)
			local v1a = v(7,1,-8)
			local v1b = v(-7,1,-8)
			local  v2 = v(16,1,1)
			local  v3 = v(16,1,7)
			local v3a = v(7,2.2,13)
			local  v4 = v(5,-1,14)
			local  v5 = v(-5,-1,14)
			local  v6 = v(-16,1,7)
			local  v7 = v(-16,1,1)
			local  v8 = v(0,0,0)
			local  v9 = v(0,5.5,-4)
			local v9a = v(6,2.2,0)
			local v10 = v(0,6.5,17)

			xref_quad(v8,v7,v1b,v1)
			xref_quad(v8,v5,v6,v7)
			tri(v8,v4,v5)
			tri(v10,v5,v4)
			xref_tri(v1,v9,v9a)
			xref_tri(v1,v9a,v1a)
			xref_quad(v1a,v9a,v2,v1a)
			xref_quad(v9,v10,v3a,v9a)
			xref_quad(v9a,v3a,v3,v2)
			xref_tri(v10,v4,v3a)
			xref_tri(v3a,v4,v3)
		else
			call_model('courier_body', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('courier_sub', v(0,0,0), v(1,0,0), v(0,1,0),1)

			if lod > 2 then
				----[[
				call_model('pilot_1_lit',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_2_lit',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_3_lit',v(0,2.17,-15),v(1,0,0),v(0,1,0),.75)

				call_model('pilot_seat_lit',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_seat_lit',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_seat_lit',v(0,2.17,-15),v(1,0,0),v(0,1,0),.75)
				----]]
				--[[ --i left the old pilot here for comparison
				call_model('pilot_3_m',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_3_m',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.75)
				call_model('pilot_4_m',v(0,2.17,-15),v(1,0,0),v(0,1,0),.75)
				--]]
			end

			texture(nil)
			use_material('win')
			zbias(1,v(0,0,0),v(0,1,0))
			load_obj('c_win.obj')
			zbias(0)

			if lod > 2 then
				call_model('posl_white', v(0,6.35,11), v(1,0,0), v(0,1,0),1.5)
			end
		end
	end,
	dynamic = function(lod)
		if lod == 1 then
			local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
			local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
			local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

			local v20 = v(16,1,1)
			local v21 = v(16,1,7)
			local v22 = v(30-trans1*9.5,-10+trans1*7.7,9-trans1*4.5)
			local v23 = v(30-trans1*9.5,-10+trans1*7.7,12-trans1*4.5)
			local v24 = v(31.6-trans1*9.5,-11.2+trans1*8,-18-trans1*5) -- new
			local v25 = v(31.6-trans1*9.5,-11.2+trans1*8,-7-trans1*5)
			local v26 = v(31.6-trans1*9.5,-11.2+trans1*8,2.7-trans1*5)
			local v27 = v(31.6-trans1*9.5,-11.2+trans1*8,6.3-trans1*5) -- new
			local v28 = v(31.6-trans1*9.5,-11.2+trans1*8,21.5-trans1*5)
			local v29 = v(31-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v30 = v(32-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v31 = v(32-trans1*9.5,-12+trans1*3.5,10-trans1*4)
			local v32 = v(31-trans1*9.5,-12+trans1*3.5,10-trans1*4)

			xref_quad(v20,v21,v23,v22)
			xref_quad(v20,v22,v23,v21)
			xref_tapered_cylinder(3, v24, v25, v(0,1,0), .1, .6)
			xref_tapered_cylinder(3, v25, v26, v(0,1,0), .6, 2)
			xref_tapered_cylinder(3, v26, v27, v(0,1,0), 2, 1.5)
			xref_tapered_cylinder(3, v27, v28, v(0,1,0), 1.3, .9)
			xref_quad(v29,v30,v31,v32)
		else
			if lod > 2 then
				if get_animation_position('WHEEL_STATE') ~= 0 then
					set_material('win', 0,0,.05,.91-math.clamp(.5*get_animation_position('WHEEL_STATE'),0,1),1,1,1.5,100)
				else
					set_material('win', 0,0,.05,1,1,1,1.5,100)
				end
			else
				set_material('win', 0,0,.05,1,1,1,1.5,100)
			end
		end
	end
})

define_model('trader', {
	info = {
			scale = 1.5,
			bounding_radius = 95,
			lod_pixels = {.1,80,250,0},
			materials = {'win'},
			tags = {'ship'},
		},

	static = function(lod)
		if lod == 1 then
			local  v1 = v(0,1,-24)
			local v1a = v(7,1,-8)
			local v1b = v(-7,1,-8)
			local  v2 = v(16,1,1)
			local  v3 = v(16,1,7)
			local v3a = v(7,2.2,13)
			local  v4 = v(5,-1,14)
			local  v5 = v(-5,-1,14)
			local  v6 = v(-16,1,7)
			local  v7 = v(-16,1,1)
			local  v8 = v(0,0,0)
			local  v9 = v(0,5.5,-4)
			local v9a = v(6,2.2,0)
			local v10 = v(0,6.5,17)
			local v11 = v(0,11.5,-9)
			local v12 = v(0,11.5,13)
			local v13 = v(0,11.5,28)
			local v14 = v(0,6,14)  -- trader
			local v15 = v(0,10,18) -- trader
			local v16 = v(0,10,20) --trader

			xref_quad(v8,v7,v1b,v1)
			xref_quad(v8,v5,v6,v7)
			tri(v8,v4,v5)
			tri(v10,v5,v4)
			xref_tri(v1,v9,v9a)
			xref_tri(v1,v9a,v1a)
			xref_quad(v1a,v9a,v2,v1a)
			xref_quad(v9,v10,v3a,v9a)
			xref_quad(v9a,v3a,v3,v2)
			xref_tri(v10,v4,v3a)
			xref_tri(v3a,v4,v3)
			tapered_cylinder(3, v11, v12, v(0,1,0), 0.5, 3.5)
			tapered_cylinder(3, v12, v13, v(0,1,0), 2.5, 1.5)
			xref_quad(v10,v14,v15,v16)
		else

			call_model('trader_body', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('trader_eng', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('courier_sub', v(0,0,0), v(1,0,0), v(0,1,0),1)

			if lod > 2 then
				call_model('pilot_1_lit',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_2_lit',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_3_lit',v(0,2.17,-15),v(1,0,0),v(0,1,0),.667)

				call_model('pilot_seat_lit',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_seat_lit',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_seat_lit',v(0,2.17,-15),v(1,0,0),v(0,1,0),.667)
				--[[ old pilot to compare
				call_model('pilot_3_m',v(.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_3_m',v(-.4,2.2,-17.2),v(1,0,0),v(0,1,0),.667)
				call_model('pilot_4_m',v(0,2.17,-15),v(1,0,0),v(0,1,0),.667)
				--]]
			end

			texture(nil)
			use_material('win')
			zbias(1,v(0,0,0),v(0,1,0))
			load_obj('c_win.obj')

			if lod > 2 then
				call_model('posl_white', v(0,13.55,20.6), v(1,0,0), v(0,1,0),1.5)
			end
		end
	end,

	dynamic = function(lod)
	if lod == 1 then
			local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
			local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
			local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

			local v20 = v(16,1,1)
			local v21 = v(16,1,7)
			local v22 = v(30-trans1*9.5,-10+trans1*7.7,9-trans1*4.5)
			local v23 = v(30-trans1*9.5,-10+trans1*7.7,12-trans1*4.5)
			local v24 = v(31.5-trans1*9.5,-11+trans1*8,-17-trans1*5)
			local v25 = v(31.5-trans1*9.5,-11+trans1*8,6-trans1*5)
			local v26 = v(31.5-trans1*9.5,-11+trans1*8,21-trans1*5)
			local v27 = v(31-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v28 = v(32-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v29 = v(32-trans1*9.5,-12+trans1*3.5,10-trans1*4)
			local v30 = v(31-trans1*9.5,-12+trans1*3.5,10-trans1*4)

			xref_quad(v20,v21,v23,v22)
			xref_quad(v20,v22,v23,v21)
			xref_tapered_cylinder(3, v24, v25, v(0,1,0), 0.5, 3.5)
			xref_tapered_cylinder(3, v25, v26, v(0,1,0), 2.5, 1.5)
			xref_quad(v27,v28,v29,v30)
		else
		    if lod > 2 then
				if get_animation_position('WHEEL_STATE') ~= 0 then
					set_material('win', 0,0,.05,.91-math.clamp(.5*get_animation_position('WHEEL_STATE'),0,1),1,1,1.5,100)
				else
					set_material('win', 0,0,.05,1,1,1,1.5,100)
				end
			else
				set_material('win', 0,0,.05,1,1,1,1.5,100)
			end
		end
	end
})
