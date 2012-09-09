define_model('caribou_eng_l', {
	info = {
		lod_pixels = { .1, 10, 100, 0 },
		bounding_radius = 20,
	},

	static = function(lod)
		if lod > 1 then
			texture('caribou_leg.png')
		end
		load_obj('caribou_eng.obj', matrix.rotate(math.pi,v(0,0,1)))
	end
})

define_model('caribou_eng_r', {
	info = {
		lod_pixels = { .1, 10, 100, 0 },
		bounding_radius = 20,
	},

	static = function(lod)
		if lod > 1 then
			texture('caribou_leg.png')
		end
		load_obj('caribou_eng.obj')
	end
})

define_model('caribou_sub', {
	info = {
		lod_pixels = { .1, 30, 400, 0 },
		bounding_radius = 70,
		materials={'top', 'bot', 'steel', 'glass', 'inside'},
	},

	static = function(lod)
		if lod > 1 then

			texture('caribou_old.png')
			set_material('top', .5,.5,.5,1,.35,.38,.4,30)
			set_material('bot', .5,.5,.5,1,.35,.38,.4,30)
			set_material('glass', .6,.6,.6,.7,.7,.8,.9,100)
			set_material('inside', .6,.65,.7,1,.7,.8,.9,10)

			if lod > 2 then
				use_material('bot')
				load_obj('caribou_bot.obj')
				use_material('top')
				load_obj('caribou_top.obj')

				texture(nil)
				use_material('inside')
				set_light(1, 0.9, v(0,1.4,-13.1), v(1,1,1))
				set_light(2, 0.9, v(-.8,1.4,-12.9), v(1,.2,.2))
				set_light(3, 0.9, v(.8,1.4,-12.9), v(.2,1,.2))
				set_local_lighting(true)
				use_light(1)
				use_light(2)
				use_light(3)
				texture('caribou_inside.png')
				load_obj('caribou_inside.obj')

				if lod > 3 then
					call_model('pilot1', v(-.8,1.57,-12.5), v(1,0,0), v(0,1,0), .06)
					call_model('pilot1', v(.8,1.57,-12.5), v(1,0,0), v(0,1,0), .06)
					call_model('pilot1', v(0,1.56,-12.7), v(1,0,0), v(0,1,0), .06)
					call_model('console1', v(-.6,1.4,-12.9), v(1,0,0), v(0,1,0), .3)
					call_model('console1', v(1,1.4,-12.9), v(1,0,0), v(0,1,0), .3)
					call_model('console1', v(.2,1.4,-13.1), v(1,0,0), v(0,1,0), .3)
				end

				set_local_lighting(false)
				use_material('glass')
				load_obj('caribou_glass.obj')
				use_material('top')
			else
				use_material('bot')
				load_obj('caribou_coll_bot.obj')
				use_material('top')
				load_obj('caribou_coll_top.obj')
			end
		else
			load_obj('caribou_coll.obj')
		end
	end,
	dynamic = function(lod)
		local rot = get_animation_position('WHEEL_STATE')
		local v1 = v(-6,-3.627+3*rot,-7.017+2*rot)
		local v2 = v(6,-3.627+3*rot,-7.017+2*rot)
		local v3 = v(-6,-3.627+3*rot,10.983-2*rot)
		local v4 = v(6,-3.627+3*rot,10.983-2*rot)
		call_model('caribou_eng_l', v1, v(-1,0,0), v(0,1-1.2*rot,.1+rot), 1)
		call_model('caribou_eng_r', v2, v(-1,0,0), v(0,1-1.2*rot,.1+rot), 1)
		call_model('caribou_eng_l', v3, v(-1,0,0), v(0,-1+1.2*rot,.1+rot), 1)
		call_model('caribou_eng_r', v4, v(-1,0,0), v(0,-1+1.2*rot,.1+rot), 1)
	end
})

define_model('caribou', {
	info = {
		scale = 2.8,
		lod_pixels = { .1, 50, 500, 0 },
		bounding_radius = 70,
		materials={'top', 'bot', 'steel', 'text1', 'text2', 'glow'},
		tags = { 'ship' },
	},
	static = function(lod)
		call_model('caribou_sub',v(0,0,0),v(1,0,0),v(0,1,0),1)

		if lod > 1 then
			if lod > 2 then
				set_material('text1', .45,.45,.45,1,.1,.1,.1,10)
				set_material('text2', .55,.55,.1,1,.1,.1,.1,10)
				set_material('steel', .2,.23,.25,1,.35,.38,.4,30)

				use_material('steel')
				load_obj('caribou_steel.obj')
				texture('caribou_new.png')
				use_material('glow')
				load_obj('caribou_glow.obj')
			end
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			if lod > 2 then
				set_material('glow', lerp_materials(get_time('SECONDS')*0.5,	{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1.5 },
				{0, 0, 0, 1, 0, 0, 0, 0, .7, 1.2, 1 }))
				local reg = get_label()
				use_material('text1')
				zbias(1, v(0, -1.127, 18.783), v(0,0.25,1))
				text(reg, v(0, -1.127, 18.783), v(0,0.25,1), v(1,0,0), 1.2, {center=true})
				zbias(0)
				use_material('text2')
				zbias(1, v(0, -1.527, -18.867), v(0,.2,-1))
				text(reg, v(0, -1.527, -18.867), v(0,.2,-1), v(-1,0,0), 1.0, {center=true})
				zbias(0)

				if get_equipment('LASER', 1) then
					use_material('steel')
					call_model('largegun1',v(0,-0.1,-16.5),v(1,0,0),v(0,1,0),.34)
				end

				if get_equipment('LASER', 2) then
					use_material('steel')
					call_model('largegun2',v(0,3.55,15.6),v(1,0,0),v(0,1,0),.34)
				end

				if get_equipment('SCANNER') then
					call_model('scanner', v(0,-4.2,-14.546), v(1,0,0), v(0,-1,0), 1)
					call_model('scanner', v(0,3.45,4.6), v(1,0,0), v(0,1,0), 1)
					call_model('antenna_1', v(3,-2.56,-19), v(1,0,0), v(0,1,0), 1)
				end

				if get_equipment('ECM') then
					call_model('ecm_1', v(-9.5,-1.789,-7.958), v(0,1,0), v(-1,0,0), 1)
					call_model('ecm_1', v(9.5,-1.789,-7.958), v(0,-1,0), v(1,0,0), 1)
				end

				local M_1 = v(-5,-4.75,-11.604)
				local M_2 = v(5,-4.75,-11.604)
				local M_3 = v(-4,-4.75,-12)
				local M_4 = v(4,-4.75,-12)
				local M_5 = v(-3,-4.75,-12.4)
				local M_6 = v(3,-4.75,-12.4)
				local M_7 = v(-2,-4.75,-12.8)
				local M_8 = v(2,-4.75,-12.8)

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

				if get_equipment('MISSILE', 7) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_7,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 7) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_7,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 7) == 'MISSILE_SMART'  then
					call_model('d_smart',M_7,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 7) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_7,v(1,0,0), v(0,.95,.05),1)
				end

				if get_equipment('MISSILE', 8) == 'MISSILE_UNGUIDED'  then
					call_model('d_unguided',M_8,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 8) == 'MISSILE_GUIDED'  then
					call_model('d_guided',M_8,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 8) == 'MISSILE_SMART'  then
					call_model('d_smart',M_8,v(1,0,0), v(0,.95,.05),1)
				elseif get_equipment('MISSILE', 8) == 'MISSILE_NAVAL'  then
					call_model('d_naval',M_8,v(1,0,0), v(0,.95,.05),1)
				end
			end

			if get_animation_position('WHEEL_STATE') ~= 0 then
				call_model('headlight',v(0,-2.78,-18.7),v(1,0,0),v(0,0,-1),3)

				call_model('posl_green', v(9.88,-2.6,-5), v(0,0,1), v(1,-0.27,0),1)
				call_model('posl_red', v(-9.88,-2.6,-5), v(0,0,1), v(-1,-0.27,0),1)
			end

			local M_T = v(-7,-1.127,16.017)
			xref_thruster(M_T, v(0,0,1), 20, true) -- i set thrusters dynamic, else the billboard function (posl.) messes with them

			local rot = get_animation_position('WHEEL_STATE')
			local LFB_T = v(-6,-4.7-.8*rot,-6+5*(1-rot))
			local RFB_T = v(6,-4.7-.8*rot,-6+5*(1-rot))
			local LRB_T = v(-6,-4.7-.8*rot,10-5*(1-rot))
			local RRB_T = v(6,-4.7-.8*rot,10-5*(1-rot))
			thruster(LFB_T, v(0,-rot,-rot+.8), 8)
			thruster(RFB_T, v(0,-rot,-rot+.8), 8)
			thruster(LRB_T, v(0,-rot,rot-.8), 8)
			thruster(RRB_T, v(0,-rot,rot-.8), 8)
		end
	end
})
