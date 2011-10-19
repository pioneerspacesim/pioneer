define_model('wyvern_gear16', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear15.obj')
	end,
})
define_model('wyvern_gear15', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear14.obj')
	end,
})
define_model('wyvern_gear14', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear13.obj')
	end,
})
define_model('wyvern_gear13', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear12.obj')
	end,
})
define_model('wyvern_gear12', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear11.obj')
	end,
})
define_model('wyvern_gear11', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear10.obj')
	end,
})
define_model('wyvern_gear10', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear09.obj')
	end,
})
define_model('wyvern_gear09', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear08.obj')
	end,
})
define_model('wyvern_gear08', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear07.obj')
	end,
})
define_model('wyvern_gear07', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear06.obj')
	end,
})
define_model('wyvern_gear06', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear05.obj')
	end,
})
define_model('wyvern_gear05', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear04.obj')
	end,
})
define_model('wyvern_gear04', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear03.obj')
	end,
})
define_model('wyvern_gear03', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear02.obj')
	end,
})
define_model('wyvern_gear02', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear01.obj')
	end,
})
define_model('wyvern_gear01', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear00.obj')
	end,
})
define_model('wyvern_gear00', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('tex.png')
		load_obj('wyvern_gear.obj')
	end,
})

define_model('wyvern_gear', {
	info = {
		scale = 1,
		lod_pixels={.1,80,160,0},
		bounding_radius = 5,
		materials={'darksteel'},
	},
	static = function(lod)
		set_material('darksteel', .08,.08,.1,1,.50,.60,.72,90)
	end,
	dynamic = function(lod)
		use_material('darksteel')
		if get_animation_position('WHEEL_STATE') == 0 then
			call_model('wyvern_gear00', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.75 then
			call_model('wyvern_gear16', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.7 then
			call_model('wyvern_gear15', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.65 then
			call_model('wyvern_gear14', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.6 then
			call_model('wyvern_gear13', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.55 then
			call_model('wyvern_gear12', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.5 then
			call_model('wyvern_gear11', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.45 then
			call_model('wyvern_gear10', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.4 then
			call_model('wyvern_gear09', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.35 then
			call_model('wyvern_gear08', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.3 then
			call_model('wyvern_gear07', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.25 then
			call_model('wyvern_gear06', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.2 then
			call_model('wyvern_gear05', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.15 then
			call_model('wyvern_gear04', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.1 then
			call_model('wyvern_gear03', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.05 then
			call_model('wyvern_gear02', v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			call_model('wyvern_gear01', v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('wyvern', {
	info = {
		scale = 2.0,
		lod_pixels = { .1, 20, 50, 0 },
		bounding_radius = 48,
		materials = {'darksteel', 'e_glow', 'glass'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Wyvern Explorer',
				forward_thrust = -280e5,
				reverse_thrust = 12e5,
				up_thrust = 90e5,
				down_thrust = -90e5,
				left_thrust = -60e5,
				right_thrust = 60e5,
				angular_thrust = 900e5,
				gun_mounts =
				{
					{ v(0,0,-150), v(0,0,-1) },
					{ v(0,0,-150), v(0,0,-1) }
				},
				max_cargo = 175,
				max_laser = 2,
				max_missile = 2,
				capacity = 175,
				hull_mass = 130,
				price = 249000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod == 1 then
			load_obj('wyvern_collision.obj')
		else
			set_material('darksteel', .08,.08,.1,1,.50,.60,.72,90)
			use_material('darksteel')
			texture('tex.png')
			load_obj('wyvern_out.obj')
			texture(nil)
			if lod > 2 then
				set_material('glass', .1,.1,.1,1,.95,.98,1,80)
				load_obj('wyvern_scoop.obj')
				use_material('glass')
				load_obj('wyvern_window.obj')
				use_material('e_glow')
				texture('wyvern_engine.png')
				load_obj('wyvern_engine.obj')

				thruster(v(0, -1.71, 7.44), v(0,0,1), 30, true)
				xref_thruster(v(0.832, -2.561, -7.718), v(0,0,-1), 1, true)
				xref_thruster(v(0.962, -4.809, 6.208), v(0,-1,0), 5)
				xref_thruster(v(0.877, -4.308, -1.198), v(0,-1,0), 5)
				xref_thruster(v(0.895, 1.434, 6.970), v(0,1,0), 5)
				xref_thruster(v(0.875, 0.492, 0.450), v(0,1,0), 5)
				thruster(v(3.364, -0.736, 3.316), v(1,0,0), 4)
				thruster(v(1.647, -1.489, -6.198), v(1,0,0), 4)
				thruster(v(-3.364, -0.736, 3.316), v(-1,0,0), 4)
				thruster(v(-1.647, -1.489, -6.198), v(-1,0,0), 4)
				call_model('coll_warn',v(0,-3.544,-5.918),v(1,0,0),v(0,-1,-.3),1)
				call_model('coll_warn',v(1.194,-4.291,7.529),v(1,0,0),v(0,-1,-.3),1)
				call_model('coll_warn',v(-1.194,-4.291,7.529),v(1,0,0),v(0,-1,-.3),1)

				call_model('posl_white',v(0,1.265,7.777),v(1,0,0),v(0,0,1),1)
				call_model('posl_green',v(16.428,-3.589,8.185),v(1,0,0),v(.3,1,-.11),1)
				call_model('posl_red',v(-16.428,-3.589,8.185),v(1,0,0),v(-.3,1,-.11),1)
			end
		end
	end,
	dynamic = function(lod)
		if lod > 2 then
			set_material('e_glow', lerp_materials(get_time('SECONDS')*.4,{0, 0, 0, 1, 0, 0, 0, 1, .5, 2, 2.5}, {0, 0, 0, 1, 0, 0, 0, 1, 1, 2.5, 2.5 }))
		end
		call_model('wyvern_gear', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})
