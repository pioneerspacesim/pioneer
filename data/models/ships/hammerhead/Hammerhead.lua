define_model('hhbox', {
	info = {
		bounding_radius = 5,
		materials={'steel2', 'rand2'},
	},

	static = function(lod)

		set_material('steel2', .45,.48,.56,1,.65,.68,.72,130)
		use_material('rand2')
		texture('tex1.png')
		load_obj('hh_gear.obj')

	end,

	dynamic = function(lod)

		set_material('rand2', get_arg_material(1))

	end

})

define_model('hhwheel', {
	info = {
		bounding_radius = 5,
		materials={'black1'},
	},

	static = function(lod)

		set_material('black1', .06,.06,.06,1,.05,.08,.06,20)
		use_material('black1')
		texture(nil)
		load_obj('hh_wheels.obj')

	end

})

define_model('hhgear', {
	info = {
		bounding_radius = 5,
		materials={'black2'},
	},

	static = function(lod)

		set_material('black2', .1,.11,.12,1,.05,.08,.06,20)
		use_material('black2')


	end,

	dynamic = function(lod)

		local rot = get_animation_position('WHEEL_STATE')
		call_model('hhwheel', v(0,-10*rot,0),v(1,0,0),v(0,1,0),1)
		call_model('hhbox', v(0,-10*rot,0), v(1,0,0), v(0,1,0), 1)

	end

})

define_model('hhrrd', {
	info = {
		bounding_radius = 5,
		materials={'steel2'},
	},

	static = function(lod)

		set_material('steel2', .45,.48,.56,1,.65,.68,.72,130)
		use_material('steel2')
		texture('body_botH.png')
		load_obj('hh_rrd.obj')

	end

})

define_model('hhrld', {
	info = {
		bounding_radius = 5,
		materials={'steel2'},
	},

	static = function(lod)

		set_material('steel2', .45,.48,.56,1,.65,.68,.72,130)
		use_material('steel2')
		texture('body_botH.png')
		load_obj('hh_rld.obj')

	end

})

define_model('hhlrd', {
	info = {
		bounding_radius = 5,
		materials={'steel2'},
	},

	static = function(lod)

		set_material('steel2', .45,.48,.56,1,.65,.68,.72,130)
		use_material('steel2')
		texture('body_botH.png')
		load_obj('hh_lrd.obj')

	end

})

define_model('hhlld', {
	info = {
		bounding_radius = 5,
		materials={'steel2'},
	},

	static = function(lod)

		set_material('steel2', .45,.48,.56,1,.65,.68,.72,130)
		use_material('steel2')
		texture('body_botH.png')
		load_obj('hh_lld.obj')

	end

})

define_model('hhmaingear', {
	info = {
		bounding_radius = 5,
		materials={'steel2', 'black2', 'rand1', 'rand2'},
	},

	static = function(lod)

		set_material('black2', .12,.14,.16,1,.2,.2,.24,30)
		use_material('black2')
		load_obj('hh_recess.obj')



	end,

	dynamic = function(lod)

		local rot = get_animation_position('WHEEL_STATE')
		call_model('hhrrd', v(5*rot,.1,0),v(1,0,0),v(0,1,0),1)
		call_model('hhrld', v(-5*rot,.1,0), v(1,0,0), v(0,1,0), 1)
		call_model('hhlrd', v(5*rot,.1,0),v(1,0,0),v(0,1,0),1)
		call_model('hhlld', v(-5*rot,.1,0), v(1,0,0), v(0,1,0), 1)

		if get_animation_position('WHEEL_STATE') > 0 then

			call_model('hhgear',v(-16,-11-(3*rot),10.6),v(1,0,0),v(0,1,0),1)
			call_model('hhgear',v(-21,-11-(3*rot),10.6),v(1,0,0),v(0,1,0),1)

			call_model('hhgear',v(16,-11-(3*rot),10.6),v(1,0,0),v(0,1,0),1)
			call_model('hhgear',v(21,-11-(3*rot),10.6),v(1,0,0),v(0,1,0),1)
		end


	end

})

define_model('hhbridge', {
	info = {
		lod_pixels={.1,50,150,0},
		bounding_radius = 60,
		materials={'steel', 'rand1', 'rand2', 'glass', 'inside'},
	},

	static = function(lod)

		if lod > 1 then

			set_material('steel', .65,.65,.65,1,.85,.85,.85,80)

			set_material('inside', .6,.6,.6,1,.6,.6,.6,0)
			texture('bridge1H.png')
			use_material('steel')
			load_obj('hh_Bridge_main.obj')
			load_obj('hh_Bridge_front.obj')
			use_material('rand1')
			load_obj('hh_Bridge_colour1.obj')
			use_material('rand2')
			load_obj('hh_Bridge_colour2.obj')

			set_light(1, 0.08, v(0,1.5,-95), v(1,1,1))
			set_light(2, 0.05, v(-5,1.5,-91), v(.2,1,.2))
			set_light(3, 0.05, v(5,1.5,-91), v(.2,.2,1))

			if lod > 3 then



				set_local_lighting(true)
				use_light(1)
				use_light(2)
				use_light(3)

				texture(nil)
				use_material('inside')
				load_obj('hh_cockpit.obj')
				call_model('pilot1',v(0,1.5,-93),v(1,0,0),v(0,1,0),.25)
				call_model('console1',v(1.4,1.5,-94),v(1,0,0),v(0,1,0),2)
				call_model('pilot1',v(-5,1.5,-89),v(1,0,0),v(0,1,0),.25)
				call_model('console1',v(-3.6,1.5,-90),v(1,0,0),v(0,1,0),2)
				call_model('pilot1',v(5,1.5,-89),v(1,0,0),v(0,1,0),.25)
				call_model('console1',v(6.4,1.5,-90),v(1,0,0),v(0,1,0),2)
				set_local_lighting(false)
				set_material('glass', .45,.48,.56,.7,.65,.68,.8,90)
				texture('bridge1H.png')
				use_material('glass')
				load_obj('hh_Bridge_glass.obj')



			elseif lod > 2 then


				set_local_lighting(true)
				use_light(1)
				use_light(2)
				use_light(3)

				texture(nil)
				use_material('inside')
				load_obj('hh_cockpit.obj')
				set_local_lighting(false)
				set_material('glass', .45,.48,.56,.7,.65,.68,.8,90)
				texture('bridge1H.png')
				use_material('glass')
				load_obj('hh_Bridge_glass.obj')

			else

				--set_material('glass', .45,.48,.56,1,.65,.68,.8,90)
				texture('bridge1H.png')
				use_material('steel')
				load_obj('hh_Bridge_glass.obj')
			end

			texture('bridge2H.png')

			use_material('steel')
			load_obj('hh_pylon.obj')


		else
			load_obj('hh_col1.obj')
		end

	end,

	dynamic = function(lod)

		set_material('rand1', get_arg_material(0))
		set_material('rand2', get_arg_material(1))

	end
})

define_model('hhfgun', {
	info = {
		bounding_radius = 5,
	},

	static = function(lod)

		texture(nil)
		load_obj('hh_fgun.obj')

	end
})

define_model('hhrgun', {
	info = {
		bounding_radius = 15,
	},

	static = function(lod)

		texture(nil)
		load_obj('hh_rgun.obj')

	end
})

define_model('hh', {
	info = {
		scale = 0.65,
		lod_pixels={.1,50,150,0},
		bounding_radius = 77,
		materials={'steel', 'darksteel', 'medsteel', 'glow', 'rand1', 'rand2', 'glass', 'inside','text1', 'text2'},
		tags = { 'ship' },
	},

	static = function(lod)

		if lod > 1 then
			set_material('medsteel', .65,.65,.65,1,.85,.85,.85,80)
			set_material('steel', .49,.5,.5,1,.95,.98,.99,60)
			set_material('darksteel', .08,.08,.1,1,.50,.60,.72,90)
			--use_material('rand1')
			use_material('medsteel')
			--load_obj('hh_Body.obj')

			texture('body_topH.png')

			load_obj('hh_Body_top.obj')
			load_obj('hh_Body_front.obj')
			use_material('rand1')
			load_obj('hh_Body_colour.obj')

			use_material('medsteel')

			texture('body_botH.png')

			load_obj('hh_Body_bot2.obj')
			load_obj('hh_Body_back.obj')
			texture(nil)
			use_material('darksteel')
			load_obj('hh_steel.obj')
			use_material('darksteel')
			load_obj('hh_Body_rest.obj')
			use_material('glow')

			-- Main Engines
			xref_thruster(v(37,0,56.3), v(0,0,1), 80, true)
			xref_thruster(v(47.07,0,56.3), v(0,0,1), 45, true)
			xref_thruster(v(17,0,56.3), v(0,0,1), 45, true)
			xref_thruster(v(11,0,56.3), v(0,0,1), 45, true)
			xref_thruster(v(5,0,56.3), v(0,0,1), 45, true)
			xref_thruster(v(0,0,56.3), v(0,0,1), 45, true)
			load_obj('glow_main.obj')

			-- Retro Engines
			xref_thruster(v(41,0,-6), v(0,0,-1), 35, true)
			xref_thruster(v(45,0,-6), v(0,0,-1), 45, true)
			xref_thruster(v(49,0,-6), v(0,0,-1), 35, true)
			load_obj('glow_retro.obj')

			-- Top Thrusters
			xref_thruster(v(23.611,8.427,9), v(0,1,0), 35, true)
			xref_thruster(v(23.611,8.427,13), v(0,1,0), 45, true)
			xref_thruster(v(23.611,8.427,17), v(0,1,0), 35, true)
			load_obj('glow_up.obj')

			-- Hover Thrusters
			xref_thruster(v(18.636,-13.531,-16), v(0,-1,0), 35, true)
			xref_thruster(v(18.636,-13.531,-12), v(0,-1,0), 45, true)
			xref_thruster(v(18.636,-13.531,-8), v(0,-1,0), 35, true)

			xref_thruster(v(18.636,-13.531,37), v(0,-1,0), 35, true)
			xref_thruster(v(18.636,-13.531,33), v(0,-1,0), 45, true)
			xref_thruster(v(18.636,-13.531,29), v(0,-1,0), 35, true)
			load_obj('glow_down.obj')
		else
			load_obj('hh_col2.obj')
		end
	end,

	dynamic = function(lod)

		use_material('medsteel')
		call_model('hhbridge',v(0,0,36*get_animation_position('WHEEL_STATE')),v(1,0,0),v(0,1,0),1)

		if lod > 2  then

			use_material('medsteel')
			call_model('hhmaingear',v(0,0,0),v(1,0,0),v(0,1,0),1)

			set_material('glow', lerp_materials(get_time('SECONDS')*0.4,	{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, .5 },
			{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, .1 }))

			if get_equipment('SCANNER') then
				call_model('scanner', v(5.467,14.299,(-74.73 + (36*get_animation_position('WHEEL_STATE')))), v(-1,0,0), v(0,1,0), 3)
				call_model('scanner', v(-5.467,14.299,(-74.73 + (36*get_animation_position('WHEEL_STATE')))), v(1,0,0), v(0,1,0), 3)
				call_model('antenna_1', v(32.212,1.994,(-91 + (36*get_animation_position('WHEEL_STATE')))), v(1,0,0), v(0,1,0), 4)
			end

			if get_equipment('ECM') then
				call_model('ecm_1', v(-38.044,0.565,(-78.316 + (36*get_animation_position('WHEEL_STATE')))), v(0,1,0), v(-1,0,0), 3.5)
				call_model('ecm_1', v(38.044,0.565,(-78.316 + (36*get_animation_position('WHEEL_STATE')))), v(0,-1,0), v(1,0,0), 3.5)
			end

			if get_equipment('LASER', 1) then
				use_material('darksteel')
				texture(nil)
				call_model('hhfgun',v(0,0,36*get_animation_position('WHEEL_STATE')),v(1,0,0),v(0,1,0),1)
			end

			if get_equipment('LASER', 2) then
				use_material('darksteel')
				texture(nil)
				call_model('hhrgun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end

			set_material('rand1', get_arg_material(0))
			set_material('rand2', get_arg_material(1))

		end
	end
})
