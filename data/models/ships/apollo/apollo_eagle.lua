--
-- apolo eagle
--
define_model('apollo', {
	info = {
		scale = 8,
		lod_pixels = { 1, 10, 50, 0 },
		bounding_radius = 75,
		materials = {'black', 'gold', 'darkgold', 'foil', 'gray', 'darkgray', 'metal', 'silver'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Apollo Eagle LM',
				reverse_thrust=2000,
				forward_thrust=-2000,
				up_thrust=48000,
				down_thrust=-2000,
				left_thrust=-2000,
				right_thrust=2000,
				angular_thrust=2000,
				max_cargo = 0,
				max_fuelscoop = 0,
				max_laser = 0,
				max_missile = 0,
				capacity = 50,
				hull_mass = 14.696,
				price = 250000,
				hyperdrive_class = 0,
			}
		},
	},
	static = function(lod)
		set_material('black', 0.1, 0.1, 0.1, 1, 0, 0, 0, 0)
		set_material('gold', 0.615, 0.424, 0.04706, 1, 0, 0, 0, 0)
		set_material('darkgold', 0.509, 0.254, 0.016, 1, 0, 0, 0, 0)
		set_material('foil', 0.398, 0.398, 0.398, 1, 0, 0, 0, 0)
		set_material('gray', 0.8, 0.8, 0.8, 1, 0, 0, 0, 0)
		set_material('darkgray', 0.213, 0.213, 0.213, 1, 0, 0, 0, 0)
		set_material('metal', 0.543, 0.543, 0.543, 1, 0, 0, 0, 0)
		set_material('silver', 0.599, 0.599, 0.599, 1, 0, 0, 0, 0)
		if (lod == 1) then
			load_obj('apollo-LOD1.obj')
		else
			use_material('black')
			load_obj('engine.obj')
			use_material('gold')
			load_obj('gold.obj')
			use_material('foil')
			load_obj('foil.obj')
			use_material('gray')
			load_obj('gray-pipes.obj')
			use_material('black')
			load_obj('black-pipes.obj')
			use_material('darkgray')
			load_obj('dark-gray.obj')
			use_material('metal')
			load_obj('metal.obj')
			use_material('darkgold')
			load_obj('gold-pipes.obj')
			use_material('silver')
			load_obj('silver.obj')
		end
	end,
	dynamic = function(lod)
		thruster(v(1.28,3.28,1.42), v(0,0,1), .5) -- back right
		thruster(v(-1.2,3.28,1.42), v(0,0,1), .5) -- back left
		
		thruster(v(1.08,3.22,-1.45), v(0,0,-1), .5) -- front right
		thruster(v(-1.05,3.22,-1.45), v(0,0,-1), .5) -- front left

		thruster(v(1.48,3.25,1.25), v(1,0,0), .5) -- right back
		thruster(v(-1.45,3.25,1.25), v(-1,0,0), .5) -- left back
		
		thruster(v(1.4,3.25,-1.34), v(1,0,0), .5) -- right front
		thruster(v(-1.4,3.25,-1.34), v(-1,0,0), .5) -- left front

		thruster(v(1.28,3.5,1.25), v(0,1,0), .5) -- top right back
		thruster(v(-1.2,3.5,1.25), v(0,1,0), .5) -- top left back
		
		thruster(v(1.22,3.55,-1.34), v(0,1,0), .5) -- top right front
		thruster(v(-1.2,3.55,-1.34), v(0,1,0), .5) -- top left front

		thruster(v(0,.5,0), v(0,-1,0), 8, 1) -- thruster down (linear only)
	end,
})