define_model('Asp_gear', {
	info = {
		scale = 1.0,
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
		scale = 3.5,
		bounding_radius = 20,
		lod_pixels = {100, 250, 500},
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
		xref_thruster(v(1.37, 0.000, 5.2), v(0.000, 0.000, 1.000), 4.50, true)
		xref_thruster(v(1.97, 0.000, 4.8), v(0.000, 0.000, 1.000), 4.50, true)
		--backward top
		thruster(v(1.65, 1.08, 0.9), v(0.3, 0.3, -1.000), 2.25, true)
		thruster(v(1.23, 1.18, 0.9), v(0.3, 0.3, -1.000), 2.25, false)
		thruster(v(-1.65, 1.08, 0.9), v(-0.3, 0.3, -1.000), 2.25, true)
		thruster(v(-1.23, 1.18, 0.9), v(-0.3, 0.3, -1.000), 2.25, false)
		--backward bottom
		thruster(v(1.65, -1.08, 0.9), v(0.3, -0.3, -1.000), 2.25, true)
		thruster(v(1.23, -1.18, 0.9), v(0.3, -0.3, -1.000), 2.25, false)
		thruster(v(-1.65, -1.08, 0.9), v(-0.3, -0.3, -1.000), 2.25, true)
		thruster(v(-1.23, -1.18, 0.9), v(-0.3, -0.3, -1.000), 2.25, false)
		--sideslip left
		thruster(v(5, 0, 0), v(1, 0, 0), 2.25, true)
		--side right
		thruster(v(-5, 0, 0), v(-1, 0, 0), 2.25, true)
		--up
		thruster(v(0, -1.7, 0), v(0, -1, 0), 2.25, true)
		--down
		thruster(v(0, 1.7, 0), v(0, 1, 0), 2.25, true)
		--pitch up
		thruster(v(0, -0.2, -3.8), v(0, -1, 0), 2.25, false)
		thruster(v(-2.26, 0.36, 3.86), v(0, 1, 0), 2.25, false)
		thruster(v(2.26, 0.36, 3.86), v(0, 1, 0), 2.25, false)
		--pitch down
		thruster(v(0, 0.2, -3.8), v(0, 1, 0), 2.25, false)
		thruster(v(-2.26, -0.36, 3.86), v(0, -1, 0), 2.25, false)
		thruster(v(2.26, -0.36, 3.86), v(0, -1, 0), 2.25, false)
		--roll left
		thruster(v(-4.3, 0.5, 0), v(0, 1, 0), 2.25, false)
		thruster(v(4.3, -0.5, 0), v(0, -1, 0), 2.25, false)
		--roll right
		thruster(v(-4.3, -0.5, 0), v(0, -1, 0), 2.25, false)
		thruster(v(4.3, 0.5, 0), v(0, 1, 0), 2.25, false)
		--yaw left
		thruster(v(0.3, 0, -4.2), v(1, 0, 0), 2.25, false)
		thruster(v(-2.8, 0, 3.9), v(-1, 0, 0), 2.25, false)
		--yaw right
		thruster(v(-0.3, 0, -4.2), v(-1, 0, 0), 2.25, false)
		thruster(v(2.8, 0, 3.9), v(1, 0, 0), 2.25, false)

	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		if lod > 1 then
			--gear animation. In Blender we have determined that the gear
			--should be translated 0.5 units downwards.
			local gearpos = get_animation_position('WHEEL_STATE')
			call_model('Asp_gear', v(0,-1.05 * gearpos,0), v(1,0,0), v(0,1,0), 1.0)
		end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		-- (white, red, green)
		navigation_lights(
			{v(4.8, 0, 0), v(-4.8, 0, 0), v(0, 0, -4.2), v(0, -0.45, 4.1), },
			{v(1.6, 0, -4.2), v(2.8, 0, 3.8), },
			{v(-1.6, 0, -4.2), v(-2.8, 0, 3.8), }
		)
	end,
})
