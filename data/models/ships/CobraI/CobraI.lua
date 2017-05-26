define_model('CobraI_front_gear', {
	info = {
		bounding_radius = 5,
		lod_pixels = {100},
		materials = {'default'},
	},
	static = function(lod)

		load_obj('front_gear.obj')
	end
})

define_model('CobraI_rear_gear', {
	info = {
		bounding_radius = 5,
		lod_pixels = {100},
		materials = {'default'},
	},
	static = function(lod)

		load_obj('rear_gear.obj')
	end
})

define_model('CobraI', {
	info = {
		bounding_radius = 15,
		lod_pixels = {20, 60, 100},
		materials = {'default'},
		tags = {'ship'},
	},
	static = function(lod)
		--name, diffuse rgba, spec rgb+intensity, emit rgb
		--set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		--use_material('default')
		--texture('asp_diff.png')
		--texture_spec('asp_spec.png')
		--texture_glow('asp_emit.png')
		local models = { 'CobraI_lod1.obj', 'CobraI_lod2.obj', 'CobraI_lod3.obj' }
		load_obj(models[lod])
		

		--forward
		xref_thruster(v(7.32, 1.32, 8.46), v(0.000, 0.000, 1.000), 4, false)
		xref_thruster(v(5.47, 1.32, 8.46), v(0.000, 0.000, 1.000), 4, false)
		xref_thruster(v(7.32, -1.32, 8.46), v(0.000, 0.000, 1.000), 4, false)
		xref_thruster(v(5.47, -1.32, 8.46), v(0.000, 0.000, 1.000), 4, false)
		
		--backward 
		xref_thruster(v(2.88, 0, -9.3), v(0, 0, -1.000), 3, false)

		--sideslip left
		thruster(v(16.2, 0, 0), v(1, 0, 0), 2, true)
		--side right
		thruster(v(-16.2, 0, 0), v(-1, 0, 0), 2, true)
		--up
		xref_thruster(v(3.96, -4.32, 6.72), v(0, -1, 0), 2, true)
		--down
		xref_thruster(v(3.96, 4.32, 6.72), v(0, 1, 0), 2, true)
		
		--pitch up
		xref_thruster(v(2.88, 0, -9.3), v(0, -1, 0), 1, false)
		xref_thruster(v(3.96, 4.32, 6.72), v(0, 1, 0), 1, false)
		
		--pitch down
		xref_thruster(v(2.88, 0, -9.3), v(0, 1, 0), 1, false)
		xref_thruster(v(3.96, -4.32, 6.72), v(0, -1, 0), 1, false)
		


	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		if lod > 1 then
			--gear animation. 
			local gearpos = get_animation_position('WHEEL_STATE')
			call_model('CobraI_front_gear', v(0,-3.5 * gearpos,-3.5 * gearpos), v(1,0,0), v(0,1,0), 1.0)
			call_model('CobraI_rear_gear', v(0,-1.8 * gearpos,0), v(1,0,0), v(0,1,0), 1.0)
		end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		-- (white, red, green)
		navigation_lights(
			{v(15.5, 0, 0), v(-15.5, 0, 0), v(0, 0, -9.6), v(0, 0, 7.5), },
			{v(-11, 0, -6.5), v(-1.8, 0, 7.5), },
			{v(11, 0, -6.5), v(1.8, 0, 7.5), }
		)
	end,
})
