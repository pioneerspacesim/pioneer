-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('meteor_gear', {
	info = {
		scale = 1.0,
		bounding_radius = 5,
		lod_pixels = {100},
		materials = {'default'},
	},
	static = function(lod)
		set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		use_material('default')
		texture('ramjetcraft.png')
		load_obj('meteor_gear.obj')
	end
})

define_model('meteor', {
	info = {
		scale = 1.2,
		bounding_radius = 13,
		lod_pixels = {10, 30, 500},
		materials = {'default'},
		tags = {'ship'},
	},
	static = function(lod)
		--name, diffuse rgba, spec rgb+intensity, emit rgb
		set_material('default', 0.8,0.8,0.8,1, .8,.8,.8,100, 0,0,0)
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
		thruster(v(0.600, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		thruster(v(-0.600, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		thruster(v(0.200, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		thruster(v(-0.200, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		thruster(v(1.000, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		thruster(v(-1.000, 0.000, 5.000), v(0.000, 0.000, 1.000), 6, true)
		--dorsal fore
		thruster(v(1.300,-0.100, -7.500), v(0.000, 1.000, 0.000), 1, false)
		thruster(v(-1.300,-0.100, -7.500), v(0.000, 1.000, 0.000), 1, false)
		--ventral fore
		thruster(v(1.400, -0.400, -7.500), v(0.000, -1.000, 0.000), 1, false)
		thruster(v(-1.400,-0.400, -7.500), v(0.000, -1.000, 0.000), 1, false)
		--dorsal aft (upper wings)
		thruster(v(2.900, 1.300, 4.200), v(-0.500, 1.000, 0.000), 2, false)
		thruster(v(-2.900, 1.300, 4.200), v(0.500, 1.000, 0.000), 2, false)
		--ventral aft (lower fins)
		thruster(v(3.000, -1.600, 4.200), v(-0.500, -1.000, 0.000), 2, false)
		thruster(v(-3.000, -1.600, 4.200), v(0.500, -1.000, 0.000), 2, false)
		--flank fore
		thruster(v( 2.000,-0.200, -6.800), v(1.000, 0.000, 0.000), 1, false)
		thruster(v(-2.000,-0.200, -6.800), v(-1.000, 0.000, 0.000), 1, false)
		--flank aft
		thruster(v( 1.800, 0.000, 4.600), v(1.000, 0.000,-0.500), 2, false)
		thruster(v(-1.800, 0.000, 4.600), v(-1.000, 0.000,-0.500), 2, false)

	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		if lod > 1 then
			local gearpos = get_animation_position('WHEEL_STATE')
			call_model('meteor_gear', v(0,.5 * (1-gearpos),2.56 * (1-gearpos)), v(1,0,0), v(0,1-.4*gearpos,.2-.2*gearpos), 1.0)
		end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		navigation_lights(
			{v(1.500,-1.000,-0.018), v(-1.500, -1.000, 0.018), },
			{v(-3.100, 1.500, 4.000), v(-3.100,-2.000, 4.000), },
			{v(3.100, 1.500, 4.000), v(3.100,-2.000, 4.000), }
		)
	end
})
