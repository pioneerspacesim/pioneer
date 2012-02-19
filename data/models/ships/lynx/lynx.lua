define_model('lx_bulk', {
	info = {
		lod_pixels = { .1, 100, 200, 0 },
		bounding_radius = 100,
		materials = {'n_cv', 'bulk'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('n_cv', .5,.55,.6,1,.5,.5,.6,50)
			set_material('bulk', .83,.9,.92,1,.4,.4,.5,30)

			texture('body1.png')
			use_material('n_cv')
		end
		load_obj('lx_bulk0.obj',Matrix.rotate(.15*math.pi,v(0,0,-1)))

		if lod > 1 then
			use_material('bulk')
		end
		load_obj('lx_bulk1.obj',Matrix.rotate(.15*math.pi,v(0,0,-1)))
	end
})

define_model('lynx', {
	info = {
		scale = 4.0,
		lod_pixels = { .1, 100, 200, 0 },
		bounding_radius = 840,
		materials = {'chrome', 'matte', 'n_cv', 'cv0', 'cv1', 'win', 'text', 'e_glow', 'cutout'},
		tags = {'static_ship'},
		ship_defs = {
			{
				name='Lynx Bulk Carrier',
				forward_thrust = -3e7,
				reverse_thrust = 2e7,
				up_thrust = 2e7,
				down_thrust = -2e7,
				left_thrust = -2e7,
				right_thrust = 2e7,
				angular_thrust = 2e7,
				gun_mounts =
				{
					{ v(0,0,-150), v(0,0,-1) },
					{ v(0,0,-150), v(0,0,-1) }
				},
				max_cargo = 3500,
				max_laser = 0,
				max_missile = 0,
				max_cargoscoop = 0,
				capacity = 3500,
				hull_mass = 800,
				fuel_tank_mass = 200,
				thruster_fuel_use = 0.0, -- These can be parked, engines running
				price = 6.5e6,
				hyperdrive_class = 8,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('chrome', .63,.7,.83,.99,1.26,1.4,1.66,30)
			set_material('matte', .25,.23,.2,1,.3,.32,.35,5)
			set_material('win', 0,0,0,.99,1,1,2,100)
			set_material('text', .7,.7,.7,1,.3,.3,.3,5)
			set_material('n_cv', .5,.55,.6,1,.5,.5,.6,50)
			set_material('cutout', .5,.55,.6,.99,.5,.5,.6,50)

			texture('body1.png')
			use_material('n_cv')
		end
		load_obj('lx_body0.obj')
		load_obj('lx_body1.obj')
		load_obj('lx_body2.obj')

		if lod > 1 then
			use_material('cv0')
		end
		load_obj('lx_body1_cv0.obj')

		if lod > 1 then
			use_material('cv1')
		end
		load_obj('lx_body2_cv1.obj')

		if lod > 1 then
			use_material('win')
			zbias(1,v(0,0,0),v(0,1,0))
			load_obj('lx_win.obj')

			use_material('chrome')
			zbias(1,v(0,0,0),v(0,1,0))
			load_obj('lx_top.obj')
			zbias(1,v(0,0,0),v(0,-1,0))
			load_obj('lx_bot.obj')
			zbias(1,v(0,0,0),v(1,0,0))
			load_obj('lx_right.obj')
			zbias(1,v(0,0,0),v(-1,0,0))
			load_obj('lx_left.obj')
			zbias(1,v(0,0,0),v(0,0,1))
			load_obj('lx_back.obj')
			zbias(1,v(0,0,0),v(0,0,-1))
			load_obj('lx_front.obj')
			zbias(1,v(0,0,0),v(0,-1,0))
			load_obj('lx_cheat0.obj')
			zbias(0)

			texture('chrome.png')
		end
		load_obj('lx_chrome.obj')

		if lod > 1 then
			texture('e_glow.png')
			use_material('e_glow')
		end
		load_obj('lx_eglow.obj')

		call_model('lx_bulk',v(0,0,-50),v(0,0,1),v(1,0,0),1)
		call_model('lx_bulk',v(0,0,-50),v(0,0,1),v(-1,0,0),1)
		call_model('lx_bulk',v(0,0,50),v(0,0,1),v(1,0,0),1)
		call_model('lx_bulk',v(0,0,50),v(0,0,1),v(-1,0,0),1)
	end,

	dynamic = function(lod)
		if lod > 1 then
			set_material('cv0', get_arg_material(0))
			set_material('cv1', get_arg_material(1))
			set_material('e_glow',lerp_materials(get_time('SECONDS')*0.25, {0,0,0,1,0,0,0,0,1,1.5,1.5},
			{0,0,0,1,0,0,0,0,1,1,2}))
		end

		local rot = .5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,1)
		call_model('lx_bulk',v(0,0,0),v(0,0,1),v(math.sin(rot),math.cos(rot),0),1)
		call_model('lx_bulk',v(0,0,0),v(0,0,1),v(-math.sin(rot),-math.cos(rot),0),1)
	end
})
