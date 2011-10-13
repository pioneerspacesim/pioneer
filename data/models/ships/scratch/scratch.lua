define_model('scratch', {
	info = {
		scale = 1.0,
		lod_pixels = { 10, 50, 70 },
		bounding_radius = 1,
		materials = { 'default', 'chrome' },
		tags = {'ship'},
		ship_defs = {
			{
				name='Scratch',
				forward_thrust = -30e5,
				reverse_thrust = 20e5,
				up_thrust = 8e5,
				down_thrust = -8e5,
				left_thrust = -8e5,
				right_thrust = 8e5,
				angular_thrust = 30e5,
				gun_mounts =  { },
				max_cargo = 10,
				max_missile = 2,
				max_fuelscoop = 0,
				capacity = 10,
				hull_mass = 10,
				price = 38000,
				hyperdrive_class = 1,
			}
		}
	},
	static = function(lod)
		set_material('default', .6,.65,.65,1,.5,.5,.5,50)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)

		use_material('default')
		texture(nil)
		--load_obj('cube.obj')
		--circle(5, v(0,0,0), v(0,1,0), v(0,0,-1), 1)
		load_obj('scratch.obj')
		thruster(v(0,-0.186,4.475), v(0,0,1), 5, true)
		--[[
		thruster(v(0,0,1), v(0,0,1), 5, true)
		thruster(v(0,0,-1), v(0,0,-1), 5, true)
		thruster(v(0,1,0), v(0,1,0), 5, true)
		thruster(v(0,-1,0), v(0,-1,0), 5, true)
		thruster(v(1,0,0), v(1,0,0), 5, true)
		thruster(v(-1,0,0), v(-1,0,0), 5, true)
		--]]
	end
})
