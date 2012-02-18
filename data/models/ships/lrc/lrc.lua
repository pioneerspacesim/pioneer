define_model('lrc', {
	info = {
		scale = 40.0,
		lod_pixels = { .1, 100, 200, 0 },
		bounding_radius = 1600,
		materials = {'chrome', 'darksteel', 'medsteel', 'glow'},
		tags = {'static_ship'},
		ship_defs = {
			{
				name='Long Range Cruiser',
				forward_thrust = -2e8,
				reverse_thrust = 5e7,
				up_thrust = 5e7,
				down_thrust = -5e7,
				left_thrust = -5e7,
				right_thrust = 5e7,
				angular_thrust = 5e7,
				gun_mounts =
				{
					{ v(0,0,-150), v(0,0,-1) },
					{ v(0,0,-150), v(0,0,-1) }
				},
				max_cargo = 15000,
				max_laser = 0,
				max_missile = 0,
				max_cargoscoop = 0,
				capacity = 15000,
				hull_mass = 4000,
				fuel_tank_mass = 1000,
				thruster_fuel_use = 0.0, -- These can be parked, engines running
				price = 3.1e8,
				hyperdrive_class = 10,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('chrome', .63,.7,.83,.99,1.26,1.4,1.66,30)
			set_material('darksteel', .08,.08,.1,1,.50,.60,.72,90)
			set_material('medsteel', .65,.65,.65,1,.85,.85,.85,80)
			use_material('darksteel')
			texture('lrc_out.png')
			load_obj('lrc_out.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			use_material('medsteel')
			texture(nil)
			load_obj('lrc_city.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			use_material('glow')
			load_obj('lrc_engine.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			use_material('chrome')
			load_obj('lrc_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			thruster(v(-3,1,16), v(0,0,1), 40, true)
			thruster(v(0.5,1,16), v(0,0,1), 40, true)
			thruster(v(4.1,1,16), v(0,0,1), 40, true)
		else
			load_obj('lrc_collision.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
		end
	end,
	dynamic = function(lod)
		if lod > 2 then
			set_material('glow', lerp_materials(get_time('SECONDS')*0.4,	{0, 0, 0, 1, 0, 0, 0, 0, 0, 0, .5 }, {0, 0, 0, 1, 0, 0, 0, 0, 0, 0, .1 }))
		end
	end
})
