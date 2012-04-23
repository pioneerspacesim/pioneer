define_model('gear', {
	info = {
		--scale = 1.0,
		bounding_radius = 5,
		lod_pixels = {50,140},
		materials = {'default'},
	},
	static = function(lod)
		set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		use_material('default')
		texture('gear_diff.png')
		load_obj('gear.obj')
	end
})

define_model('meteor', {
	info = {
		scale = 1,
		bounding_radius = 40,
		lod_pixels = {50 , 200, 0},
		materials = {'default'},
		tags = {'ship'},
	},
	static = function(lod)
		--name, diffuse rgba, spec rgb+intensity, emit rgb
		set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		use_material('default')
		texture('ramjetcraft.png')
		texture_glow('glowmap.png')
		if lod == 1 then
			load_obj('ramjetcraft_lod1.obj')
		end
		if lod == 2 then
			load_obj('ramjetcraft_lod2.obj')
		end
		if lod == 3 then
			load_obj('ramjetcraft_lod3.obj')
		end

		--thrusters from blender
		--mains (rear)
		thruster(v(0.600, 0.000, 5.000), v(0.000, 0.000, 1.000), 5, true)
		thruster(v(-0.600, 0.000, 5.000), v(0.000, 0.000, 1.000), 5, true)
		thruster(v(0.200, 0.000, 5.000), v(0.000, 0.000, 1.000), 5, true)
		thruster(v(-0.200, 0.000, 5.000), v(0.000, 0.000, 1.000), 5, true)
		--dorsal fore
		thruster(v(1.200, 0.060, -7.000), v(0.000, 1.000, 0.000), 2, false)
		thruster(v(-1.200, 0.060, -7.000), v(0.000, 1.000, 0.000), 2, false)
		--ventral fore
		thruster(v(1.200, -0.200, -7.200), v(0.000, -1.000, 0.000), 2, false)
		thruster(v(-1.200,-0.200, -7.200), v(0.000, -1.000, 0.000), 2, false)
		--dorsal aft (upper wings)
		thruster(v(2.8, 1.500, 4.000), v(-0.500, 1.000, 0.000), 3, false)
		thruster(v(-2.8, 1.500, 4.000), v(0.500, 1.000, 0.000), 3, false)
		--ventral aft (lower fins)
		thruster(v(2.8, -1.500, 4.000), v(-0.500, -1.000, 0.000), 3, false)
		thruster(v(-2.8, -1.500, 4.000), v(0.500, -1.000, 0.000), 3, false)
		--flank fore
		thruster(v( 2.000, 0.000, -6.400), v(1.000, 0.000, 0.000), 2, false)
		thruster(v(-2.000, 0.000, -6.400), v(-1.000, 0.000, 0.000), 2, false)
		--flank aft
		thruster(v( 2.000, 0.000, 4.400), v(1.000, 0.000, 0.000), 3, false)
		thruster(v(-2.000, 0.000, 4.400), v(-1.000, 0.000, 0.000), 3, false)

	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		--if lod > 1 then
			--gear animation. In Blender we have determined that the gear
			--should be translated 0.5 units downwards.
		--	local gearpos = get_animation_position('WHEEL_STATE')
		--	call_model('gear', v(0,-0.5 * gearpos,0), v(1,0,0), v(0,1,0), 1.0)
		--end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		navigation_lights(
			{v(4.355, 0.763, -0.018), v(-4.467, 0.763, -0.018), },
			{v(4.030, 0.028, 8.898), v(3.300, 0.028, -8.869), },
			{v(-4.030, 0.028, 8.898), v(-3.300, 0.028, -8.869), }
		)
	end
})