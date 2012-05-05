define_model('Asp_gear', {
	info = {
		bounding_radius = 5,
		lod_pixels = {100},
		materials = {'default'},
	},
	static = function(lod)

		load_obj('gear.obj')
	end
})

define_model('Asp_scout', {
	info = {
		bounding_radius = 20,
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
		local models = { 'asp_lod1.obj', 'asp_lod2.obj', 'asp_lod3.obj' }
		load_obj(models[lod])
		

		--forward
		xref_thruster(v(4.8, 0.000, 18.2), v(0.000, 0.000, 1.000), 4.50, true)
		xref_thruster(v(6.9, 0.000, 16.8), v(0.000, 0.000, 1.000), 4.50, true)
		--backward top
		thruster(v(5.77, 3.78, 3.15), v(0.3, 0.3, -1.000), 2.25, true)
		thruster(v(4.30, 4.13, 3.15), v(0.3, 0.3, -1.000), 2.25, true)
		thruster(v(-5.77, 3.78, 3.15), v(-0.3, 0.3, -1.000), 2.25, true)
		thruster(v(-4.30, 4.13, 3.15), v(-0.3, 0.3, -1.000), 2.25, true)
		--backward bottom
		thruster(v(5.77, -3.78, 3.15), v(0.3, -0.3, -1.000), 2.25, true)
		thruster(v(4.30, -4.13, 3.15), v(0.3, -0.3, -1.000), 2.25, true)
		thruster(v(-5.77, -3.78, 3.15), v(-0.3, -0.3, -1.000), 2.25, true)
		thruster(v(-4.30, -4.13, 3.15), v(-0.3, -0.3, -1.000), 2.25, true)

		--pitch up
		thruster(v(0, -0.7, -13.3), v(0, -1, 0), 2.25, false)
		thruster(v(-7.91, 1.26, 13.3), v(0, 1, 0), 2.25, false)
		thruster(v(7.91, 1.26, 13.3), v(0, 1, 0), 2.25, false)
		--pitch down
		thruster(v(0, 0.7, -13.3), v(0, 1, 0), 2.25, false)
		thruster(v(-7.91, -1.26, 13.3), v(0, -1, 0), 2.25, false)
		thruster(v(7.91, -1.26, 13.3), v(0, -1, 0), 2.25, false)
		--roll left
		thruster(v(-15.05, 1.75, 0), v(0, 1, 0), 2.25, false)
		thruster(v(15.05, -1.75, 0), v(0, -1, 0), 2.25, false)
		--roll right
		thruster(v(-15.05, -1.75, 0), v(0, -1, 0), 2.25, false)
		thruster(v(15.05, 1.75, 0), v(0, 1, 0), 2.25, false)
		--yaw left
		thruster(v(1.05, 0, -14.7), v(1, 0, 0), 2.25, false)
		thruster(v(-9.8, 0, 13.65), v(-1, 0, 0), 2.25, false)
		--yaw right
		thruster(v(-1.05, 0, -14.7), v(-1, 0, 0), 2.25, false)
		thruster(v(9.8, 0, 13.65), v(1, 0, 0), 2.25, false)

	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		if lod > 1 then
			--gear animation. 
			--should be translated some units downwards.
			local gearpos = get_animation_position('WHEEL_STATE')
			call_model('Asp_gear', v(0,-3.675 * gearpos,0), v(1,0,0), v(0,1,0), 1.0)
		end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		-- (white, red, green)
		navigation_lights(
			{v(16.65, 0, 0), v(-16.65, 0, 0), v(0, 0, -14.5), v(0, -1.45, 14.15), },
			{v(-5.5, 0, -14.5), v(-9, 0, 12.9), },
			{v(5.5, 0, -14.5), v(9, 0, 12.9), }
		)
	end,
})
