define_model('gear', {
	info = {
		scale = 1.0,
		bounding_radius = 5,
		lod_pixels = {100},
		materials = {'default'},
	},
	static = function(lod)
		set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		use_material('default')
		texture('gear_diff.png')
		load_obj('gear.obj')
	end
})

define_model('natrix', {
	info = {
		scale = 3.5,
		bounding_radius = 40,
		lod_pixels = {50, 100},
		materials = {'default'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Natrix',
				forward_thrust = -6e6,
				reverse_thrust = 1e6,
				up_thrust = 1e6,
				down_thrust = -1e6,
				left_thrust = -1e6,
				right_thrust = 1e6,
				angular_thrust = 15e6,
				gun_mounts = {
					{ v(0.000, 0.000, -9.342), v(0.000, 0.000, -1.000) },
				},
				max_cargo = 50,
				max_laser = 1,
				max_missile = 0,
				max_cargoscoop = 0,
				max_fuelscoop = 0,
				capacity = 40,
				hull_mass = 15,
				fuel_tank_mass = 15,
				thruster_fuel_use = 0.00015,
				price = 50000,
				hyperdrive_class = 2,
			}
		}
	},
	static = function(lod)
		--name, diffuse rgba, spec rgb+intensity, emit rgb
		set_material('default', 0.8,0.8,0.8,1, .3,.3,.3,5, 0,0,0)
		use_material('default')
		texture('hull.png')
		texture_glow('emit.png')
		if lod == 1 then
			load_obj('lod1.obj')
		else
			load_obj('lod2.obj')
		end

		--thrusters from blender
		thruster(v(1.300, 0.000, 9.769), v(0.000, 0.000, 1.000), 4.50, true)
		thruster(v(-1.300, 0.000, 9.769), v(0.000, 0.000, 1.000), 4.50, true)
		thruster(v(-3.000, 0.427, 9.214), v(0.000, 1.000, -0.000), 1.24, false)
		thruster(v(-3.458, 0.000, 9.214), v(-1.000, -0.000, 0.000), 1.24, false)
		thruster(v(-3.000, -0.472, 9.214), v(0.000, -1.000, 0.000), 1.24, false)
		thruster(v(3.447, 0.000, 9.214), v(1.000, -0.000, -0.000), 1.24, false)
		thruster(v(2.989, -0.472, 9.214), v(-0.000, -1.000, -0.000), 1.24, false)
		thruster(v(2.989, 0.427, 9.214), v(0.000, 1.000, 0.000), 1.24, false)
		thruster(v(-3.896, -0.619, -0.006), v(0.000, -1.000, 0.000), 1.24, false)
		thruster(v(-3.896, 0.607, -0.006), v(0.000, 1.000, -0.000), 1.24, false)
		thruster(v(3.880, 0.607, -0.006), v(0.000, 1.000, -0.000), 1.24, false)
		thruster(v(3.880, -0.619, -0.006), v(0.000, -1.000, 0.000), 1.24, false)
		thruster(v(-4.117, 0.000, -2.199), v(-1.000, -0.000, 0.000), 1.24, false)
		thruster(v(4.117, 0.000, -2.199), v(1.000, -0.000, -0.000), 1.24, false)
		thruster(v(-0.201, 0.327, -8.694), v(0.000, 1.000, -0.000), 1.24, false)
		thruster(v(-0.201, -0.384, -8.694), v(-0.000, -1.000, -0.000), 1.24, false)
		thruster(v(0.201, -0.384, -8.694), v(-0.000, -1.000, -0.000), 1.24, false)
		thruster(v(0.201, 0.327, -8.694), v(0.000, 1.000, -0.000), 1.24, false)

	end,
	dynamic = function(lod)
		--no point in visible gear on lowest lod
		if lod > 1 then
			--gear animation. In Blender we have determined that the gear
			--should be translated 0.5 units downwards.
			local gearpos = get_animation_position('WHEEL_STATE')
			call_model('gear', v(0,-0.5 * gearpos,0), v(1,0,0), v(0,1,0), 1.0)
		end

		--lights visible when the craft is docking, landing
		--or landing gear is down
		navigation_lights(
			{v(4.355, 0.763, -0.018), v(-4.467, 0.763, -0.018), },
			{v(4.030, 0.028, 8.898), v(3.300, 0.028, -8.869), },
			{v(-4.030, 0.028, 8.898), v(-3.300, 0.028, -8.869), }
		)
	end
})
