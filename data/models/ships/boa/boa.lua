define_model('boa_plate', {
	info = {
		bounding_radius = 14,
		materials={'bsteel'},
	},
	static = function(lod)
		set_material('bsteel', .04,.07,.08,1,.35,.38,.4,30)
		use_material('bsteel')
		load_obj('boa_plate.obj')
		texture(nil)
	end
})

define_model('boanosewheel', {
	info = {
		lod_pixels={5,10,20,0},
		--[[
		lod_pixels = {5,50,0},
		this will be lod 3 = max. - max factor = 3 when used for div i.e.
		--]]
		bounding_radius = 10,
		materials={'leg','tyre'}
	},
	static = function(lod)
		set_material('leg', .5, .5, .5, 1, .5, .5, .5, 2.0, 0, 0, 0)
		set_material('tyre', .3, .3, .3, 1, 0,0,0, 1, 0, 0, 0)
		use_material('leg')
	end,

	dynamic = function(lod)
		local v6 = v(0, 0, 0)
		local v7 = v(0, 0.1 + (4*(get_animation_position('WHEEL_STATE')^5)), 0)
		local v8 = v(0, 0.1 + (8*(get_animation_position('WHEEL_STATE')^5)), 0)
		local divs = lod*4
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		use_material('tyre')
		xref_cylinder(divs, v(.5,0.1 + (8*(get_animation_position('WHEEL_STATE')^5)),0), v(1,0.1 + (8*(get_animation_position('WHEEL_STATE')^5)),0), v(0,0,1), 1.0)
	end
})

define_model('boamainwheel', {
	info = {
		lod_pixels = {5,10,20,0},
		bounding_radius = 7,
		materials = {'leg', 'tyre'}
	},
	static = function(lod)

		set_material('leg', .5,.5,.5,1, 1,1,1, 2, 0,0,0)
		use_material('leg')

	end,

	dynamic = function(lod)
		local v6 = v(0,0,0)
		local v7 = v(0,0.1 + (3*(get_animation_position('WHEEL_STATE')^7)),0)
		local v8 = v(0,0.1 + (5*(get_animation_position('WHEEL_STATE')^7)),0)
		-- crossbar
		local v13 = v(0, 0.1 + (5*(get_animation_position('WHEEL_STATE')^7)), 1.4)
		local v14 = v(0, 0.1 + (5*(get_animation_position('WHEEL_STATE')^7)), -1.4)
		local divs = 4*lod

		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		cylinder(4, v13, v14, v(1,0,0), .5)
		set_material('tyre', .3,.3,.3,1, 0,0,0, 1, 0,0,0)
		use_material('tyre')
		xref_cylinder(divs, v(.5, 0.1 + (5*(get_animation_position('WHEEL_STATE')^8)), 1.1), v(1, 0.1 + (5*(get_animation_position('WHEEL_STATE')^8)), 1.1), v(0,0,1), 1)
		xref_cylinder(divs, v(.5, 0.1 + (5*(get_animation_position('WHEEL_STATE')^8)), -1.1), v(1, 0.1 + (5*(get_animation_position('WHEEL_STATE')^8)), -1.1), v(0,0,1), 1)
	end
})

define_model('boa_recess', {
	info = {
		bounding_radius = 30,
	},
	static = function(lod)
		texture('boa.png')
		load_obj('boa2_recess.obj')
	end
})

define_model('boa_bkflap', {
	info = {
		bounding_radius = 18,
	},
	static = function(lod)
		texture('boa.png')
		load_obj('boa2_bkflap.obj')
	end
})

define_model('boa_frflap', {
	info = {
		bounding_radius = 28,
	},
	static = function(lod)
		texture('boa.png')
		load_obj('boa2_frflap.obj')
	end
})

define_model('boa', {
	info = {
		scale = 1.34,
		lod_pixels={.1,50,200,0},
		bounding_radius = 72, -- "scale" for lod calculation, bounding box in preview and closest zoom limit
		materials={'steel', 'body', 'glow', 'wing', 'darksteel', 'glass', 'inside', 'text1'},
		tags = { 'ship' },
		ship_defs = {
			{
				name='Boa Freighter',
				forward_thrust = -12e7,
				reverse_thrust = 4e7,
				up_thrust = 4e7,
				down_thrust = -2e7,
				left_thrust = -2e7,
				right_thrust = 2e7,
				angular_thrust = 50e7,
				gun_mounts =
				{
					{ v(0,-2,-46), v(0,0,-1) },
					{ v(0,0,0), v(0,0,1) },
				},
				max_cargo = 600,
				max_laser = 2,
				max_missile = 6,
				max_cargoscoop = 0,
				capacity = 600,
				hull_mass = 300,
				fuel_tank_mass = 280,
				thruster_fuel_use = 0.00025,
				price = 2474000,
				hyperdrive_class = 7,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('steel', .44,.47,.48,1,.55,.58,.6,30)
			set_material('darksteel', .24,.27,.28,1,.5,.5,.6,80)
			set_material('glass', .5,.5,.5,.6,.8,.8,.8,80)
			set_material('inside', .5,.5,.5,1,.55,.58,.6,30)

			texture('boa.png')

			use_material('body')
			load_obj('boa2_body.obj')

			use_material('wing')
			load_obj('boa2_frw.obj')
			load_obj('boa2_bkw.obj')

			use_material('steel')
			load_obj('boa2_bot.obj')

			use_material('glow')
			load_obj('boa_glow.obj')
			set_local_lighting(true)
			use_material('inside')
			set_light(1, 0.0005, v(0,4.369,-2.288), v(1,1,1))
			use_light(1)
			load_obj('boa_inside.obj')
			call_model('pilot2', v(0,4.1,-1.8), v(1,0,0), v(0,1,0), .12)
			set_local_lighting(false)
			use_material('glass')
			load_obj('boa2_cpit.obj')

		else
			load_obj('boa2_coll.obj')
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('body', get_arg_material(0))
			set_material('wing', get_arg_material(1))
			set_material('glow', lerp_materials(get_time('HOURS')*.1,	{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1.5 },
			{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1 }))

			use_material('darksteel')

			if lod > 2 then

				set_material('text1', .45,.45,.45,1,.1,.1,.1,10)
				local reg = get_label()
				texture('models/ships/boa/fade.png')
				use_material('wing')
				--zbias(1, v(0, -1.127, 18.783), v(0,0.25,1))
				text(reg, v(0, 4, 25.205), v(0,0,1), v(1,0,0), 1.2, {center=true})
				--zbias(0)

				use_material('darksteel')

				if get_equipment('ECM') then
					call_model('ecm_1', v(-7.7,1,-10.5), v(0,1,0), v(-1,0,0), 1.4)
					call_model('ecm_1', v(7.7,1,-10.5), v(0,-1,0), v(1,0,0), 1.4)
				end

				if get_equipment('SCANNER') then
					call_model('scanner', v(0,-1.22,-31.124), v(1,0,0), v(0,-1,0), 1)
					call_model('scanner', v(0,5.95,8.026), v(1,0,0), v(0,1,0), 1.4)
					call_model('antenna_1', v(0,-1.882,-44.7), v(1,0,0), v(0,1,0), 1)
				end


				if get_equipment('LASER', 1) then
					use_material('darksteel')
					call_model('largegun2',v(0,-.2,-39),v(-1,0,0),v(0,1,0),.2)
				end

				if get_equipment('LASER', 2) then
					use_material('darksteel')
					call_model('largegun2',v(0,6.97,22.44),v(1,0,0),v(0,1,0),.4)
					call_model('largegun2',v(0,-2.945,23),v(-1,0,0),v(0,-1,0),.4)
				end

				local M_1 = v(-4.8,-1.827,-27.5)
				local M_2 = v(4.8,-1.827,-27.5)
				local M_3 = v(-5.96,-1.84,-26.6)
				local M_4 = v(5.96,-1.84,-26.6)
				local M_5 = v(-7,-1.89,-24.9)
				local M_6 = v(7,-1.89,-24.9)


				if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_1,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 1) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_1,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 1) == 'MISSILE_SMART'  then
					call_model('d_smart',M_1,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 1) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_1,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_2,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 2) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_2,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 2) == 'MISSILE_SMART'  then
					call_model('d_smart',M_2,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 2) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_2,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_3,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 3) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_3,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 3) == 'MISSILE_SMART'  then
					call_model('d_smart',M_3,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 3) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_3,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_4,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 4) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_4,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 4) == 'MISSILE_SMART'  then
					call_model('d_smart',M_4,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 4) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_4,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 5) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_5,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 5) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_5,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 5) == 'MISSILE_SMART'  then
					call_model('d_smart',M_5,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 5) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_5,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 6) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_6,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 6) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_6,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 6) == 'MISSILE_SMART'  then
					call_model('d_smart',M_6,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 6) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_6,v(1,0,0), v(0,.95,.05),1)
				end




			end

		end

		use_material('steel')
		call_model('boa_bkflap',v(0,0,8 * get_animation_position('WHEEL_STATE')),v(1,0,0),v(0,1,0),1)
		call_model('boa_frflap',v(0,0,3 * get_animation_position('WHEEL_STATE')),v(1,0,0),v(0,1,0),1)
		call_model('boa_plate',v(0,0,0),v(1,0,0),v(0,1,0),1)

		if get_animation_position('WHEEL_STATE') ~= 0 then

			use_material('steel')
			call_model('boa_recess',v(0,0,0),v(1,0,0),v(0,1,0),1)

			local v1 = v(0,0,-25.5) --TEMPORARY
			local v2 = v(-8,-2,9)  --TEMPORARY
			local v3 = v(8,-2,9)  --TEMPORARY
			call_model('boanosewheel', v1, v(1,0,0), v(0,-1,0), 0.9)
			call_model('boamainwheel', v2, v(1,0,0), v(0,-1,0), 1.3)
			call_model('boamainwheel', v3, v(1,0,0), v(0,-1,0), 1.3)

			if lod > 1 then
				call_model('posl_green', v(13.5,12,27), v(0,0,1), v(1,-0.27,0),1)
				call_model('posl_red', v(-13.5,12,27), v(0,0,1), v(-1,-0.27,0),1)
				call_model('posl_green', v(9.9,-2.2,-23.3), v(0,0,1), v(1,-0.27,0),1)
				call_model('posl_red', v(-9.9,-2.2,-23.3), v(0,0,1), v(-1,-0.27,0),1)
			end
		end

		if lod > 1 then -- i set thrusters dynamic because else they interfere with the posl.
			local M_T = v(13.5,-0.3,16)
			local R_T = v(12,0,8)
			xref_thruster(M_T, v(0,0,1), 65, true)
			xref_thruster(R_T, v(0,0,-1), 25, true)
		end
	end
})
