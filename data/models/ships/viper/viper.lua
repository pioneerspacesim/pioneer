define_model('viper_pol_body', {
	info = {
		scale = 1,
		lod_pixels={.1,80,600,0},
		bounding_radius = 15,
		materials={'col1', 'dark', 'glass', 'inside'},
	},
	static = function(lod)

		if lod > 1 then
			set_material('col1', .5,.5,.5,1,.95,.98,1,90)
			set_material('dark', .1,.1,.1,1,.1,.1,.1,10)
			set_material('inside', .5,.5,.5,1,.95,.98,1,20)
			set_material('glass', .5,.5,.5,.4,.95,.98,1,50)

			if lod > 3 then
				use_material('col1')
				texture('viper_pol1.png')
				load_obj('viper.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_cockpit.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				use_material('dark')
				--texture(nil)
				load_obj('viper_intakepart1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

				set_light(1, 0.00000000008, v(0,0.19,-1.43), v(1,1,1))
				set_local_lighting(true)
				use_material('inside')
				texture('viper_pol1.png')
				load_obj('viper_inside.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				call_model('pilot1', v(0,0.18,-1.45),v(1,0,0),v(0,1,0),0.012)
				set_local_lighting(false)

				--texture(nil)
				use_material('glass')
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

			elseif lod > 2 then
				use_material('col1')
				texture('viper_pol1.png')
				load_obj('viper.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_cockpit.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				use_material('dark')
				--texture(nil)
				load_obj('viper_intakepart1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

				--set_light(1, 0.00000000008, v(0,1.533,0.23), v(1,1,1))
				--set_local_lighting(true)
				--use_material('inside')
				--texture('viper.png')
				--load_obj('viper_inside.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				--call_model('pilot1', v(0,0.18,-1.46,v(1,0,0),v(0,1,0),0.012))
				--set_local_lighting(false)

				--texture(nil)
				use_material('col1')
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			else
				use_material('col1')
				texture('viper_pol1.png')
				load_obj('viper_lod.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			end
		else
			load_obj('viper_col.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
		end
	end
})

define_model('viper_body', {
	info = {
		scale = 1,
		lod_pixels={.1,80,600,0},
		bounding_radius = 15,
		materials={'col1', 'dark', 'glass', 'inside'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('col1', .5,.5,.5,1,.95,.98,1,90)
			set_material('dark', .1,.1,.1,1,.1,.1,.1,10)
			set_material('inside', .5,.5,.5,1,.95,.98,1,20)
			set_material('glass', .5,.5,.5,.4,.95,.98,1,50)
			--[[
			selector2()
			select2 = 10
			if select2 < 56 then
			texture('viper_a1.png')
			else
			texture('viper_pol1.png')
			end
			--]]
			if lod > 3 then

				use_material('col1')
				texture('viper_a1.png')
				load_obj('viper.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_cockpit.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				use_material('dark')
				--texture(nil)
				load_obj('viper_intakepart1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

				set_light(1, 0.00000000008, v(0,0.19,-1.44), v(1,1,1))
				set_local_lighting(true)
				use_material('inside')
				--texture('viper_pol1.png')
				load_obj('viper_inside.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				call_model('pilot1', v(0,0.18,-1.45),v(1,0,0),v(0,1,0),0.012)
				set_local_lighting(false)

				--texture(nil)
				use_material('glass')
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

			elseif lod > 2 then

				use_material('col1')
				texture('viper_a1.png')
				load_obj('viper.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_cockpit.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				use_material('dark')
				--texture(nil)
				load_obj('viper_intakepart1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))

				--set_light(1, 0.00000000008, v(0,1.533,0.23), v(1,1,1))
				--set_local_lighting(true)
				--use_material('inside')
				--texture('viper.png')
				--load_obj('viper_inside.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				--call_model('pilot1', v(0,0.18,-1.46,v(1,0,0),v(0,1,0),0.012))
				--set_local_lighting(false)

				--texture(nil)
				use_material('col1')
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			else
				use_material('col1')
				texture('viper_a1.png')
				load_obj('viper_lod.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
				load_obj('viper_window.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
			end
		else
			load_obj('viper_col.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
		end
	end
})

define_model('viperpol_gear20', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear20.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear19', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear19.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear18', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear18.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear17', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear17.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear16', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear16.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear15', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear15.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear14', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear14.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear13', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear13.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear12', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear12.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear11', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear11.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear10', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear10.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear09', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear9.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear08', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear8.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear07', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear7.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear06', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear6.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear05', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear5.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear04', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear4.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear03', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear3.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear02', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear2.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear01', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viperpol_gear00', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_pol1.png')
		load_obj('gear.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})

define_model('viper_pol_gear', {
	info = {
		scale = 1,
		lod_pixels={.1,80,160,0},
		bounding_radius = 5,
		materials={'col1'},
	},
	static = function(lod)
		--if lod > 1 then
		set_material('col1', .5,.5,.5,1,.95,.98,1,90)
	end,

	dynamic = function(lod)
		use_material('col1')
		--		texture('models/ships/viper/viper_pol1.png')
		if get_animation_position('WHEEL_STATE') > 0.95 then
			call_model('viperpol_gear20', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.9 then
			call_model('viperpol_gear19', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.85 then
			call_model('viperpol_gear18', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.8 then
			call_model('viperpol_gear17', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.75 then
			call_model('viperpol_gear16', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.7 then
			call_model('viperpol_gear15', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.65 then
			call_model('viperpol_gear14', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.6 then
			call_model('viperpol_gear13', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.55 then
			call_model('viperpol_gear12', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.5 then
			call_model('viperpol_gear11', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.45 then
			call_model('viperpol_gear10', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.4 then
			call_model('viperpol_gear09', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.35 then
			call_model('viperpol_gear08', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.3 then
			call_model('viperpol_gear07', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.25 then
			call_model('viperpol_gear06', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.2 then
			call_model('viperpol_gear05', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.15 then
			call_model('viperpol_gear04', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.1 then
			call_model('viperpol_gear03', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.05 then
			call_model('viperpol_gear02', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0 then
			call_model('viperpol_gear01', v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			call_model('viperpol_gear00', v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('viper_gear20', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear20.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear19', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear19.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear18', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear18.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear17', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear17.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear16', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear16.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear15', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear15.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear14', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear14.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear13', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear13.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear12', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear12.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear11', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear11.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear10', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear10.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear09', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear9.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear08', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear8.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear07', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear7.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear06', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear6.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear05', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear5.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear04', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear4.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear03', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear3.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear02', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear2.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear01', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear1.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})
define_model('viper_gear00', {
	info = { bounding_radius = 5, },
	static = function(lod)
		texture('viper_a1.png')
		load_obj('gear.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
	end,
})


define_model('viper_gear', {
	info = {
		scale = 1,
		lod_pixels={.1,80,160,0},
		bounding_radius = 5,
		materials={'col1'},
	},
	static = function(lod)
		--if lod > 1 then
		set_material('col1', .5,.5,.5,1,.95,.98,1,90)
	end,

	dynamic = function(lod)
		use_material('col1')
		--		texture('models/ships/viper/viper_a1.png')
		if get_animation_position('WHEEL_STATE') > 0.95 then
			call_model('viper_gear20', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.9 then
			call_model('viper_gear19', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.85 then
			call_model('viper_gear18', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.8 then
			call_model('viper_gear17', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.75 then
			call_model('viper_gear16', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.7 then
			call_model('viper_gear15', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.65 then
			call_model('viper_gear14', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.6 then
			call_model('viper_gear13', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.55 then
			call_model('viper_gear12', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.5 then
			call_model('viper_gear11', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.45 then
			call_model('viper_gear10', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.4 then
			call_model('viper_gear09', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.35 then
			call_model('viper_gear08', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.3 then
			call_model('viper_gear07', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.25 then
			call_model('viper_gear06', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.2 then
			call_model('viper_gear05', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.15 then
			call_model('viper_gear04', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.1 then
			call_model('viper_gear03', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0.05 then
			call_model('viper_gear02', v(0,0,0),v(1,0,0),v(0,1,0),1)
		elseif get_animation_position('WHEEL_STATE') > 0 then
			call_model('viper_gear01', v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			call_model('viper_gear00', v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('viperpol', {
	info = {
		scale = 10,
		lod_pixels={.1,80,600,0},
		bounding_radius = 35,
		materials={'col1'},
		tags = { 'ship' },
	},
	static = function(lod)

		if lod > 1 then
			set_material('col1', .5,.5,.5,1,.95,.98,1,90)

			use_material('col1')
			--texture('viper_pol1.png')
			call_model('viper_pol_body', v(0,0.22,0.38),v(1,0,0),v(0,1,0),1)
			call_model('viper_pol_gear', v(0,0.22,0.38),v(1,0,0),v(0,1,0),1)
			local vMainThruster = v(0.4,0.221,1.97)
			local vRetroThruster = v(0.71,0.231,-0.1)
			local vLeftThruster = v(-1.02,0.231,0.79)
			local vRightThruster = v(1.02,0.231,0.79)
			local vTopThruster = v(0,0.711,0.8)
			local vBottomThruster = v(0,-0.199,0.8)

			xref_thruster(vMainThruster, v(0,0,1), 3, true)
			xref_thruster(vRetroThruster, v(0,0,-1), 1.4, true)
			thruster(vLeftThruster, v(-1,0,0), 1)
			thruster(vRightThruster, v(1,0,0), 1)
			thruster(vTopThruster, v(0,1,0), 1)
			thruster(vBottomThruster, v(0,-1,0), 1)
		else
			--load_obj('viper_col.obj')
			load_obj('viper_col.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
		end
	end,
	dynamic = function(lod)
		if lod > 2 then
			local M_1 = v(0.202,0.807,1.294)
			local M_2 = v(-0.202,0.807,1.294)
			local M_3 = v(0.411,0.806,1.294)
			local M_4 = v(-0.411,0.806,1.294)

			if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
				call_model('d_smart',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_1,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
				call_model('d_smart',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_2,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
				call_model('d_smart',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_3,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
				call_model('d_smart',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_4,v(1,0,0), v(0,.95,.05),0.16)
			end

		end
	end
})

define_model('viper', {
	info = {
		scale = 10,
		lod_pixels={.1,80,600,0},
		bounding_radius = 35,
		materials={'col1'},
		tags = { 'ship' },
	},
	static = function(lod)
		if lod > 1 then
			set_material('col1', .5,.5,.5,1,.95,.98,1,90)

			use_material('col1')
			--texture('viper_pol1.png')
			call_model('viper_body', v(0,0.22,0.38),v(1,0,0),v(0,1,0),1)
			call_model('viper_gear', v(0,0.22,0.38),v(1,0,0),v(0,1,0),1)

			local vMainThruster = v(0.4,0.221,1.97)
			local vRetroThruster = v(0.71,0.231,-0.1)
			local vLeftThruster = v(-1.02,0.231,0.79)
			local vRightThruster = v(1.02,0.231,0.79)
			local vTopThruster = v(0,0.711,0.8)
			local vBottomThruster = v(0,-0.199,0.8)

			xref_thruster(vMainThruster, v(0,0,1), 3, true)
			xref_thruster(vRetroThruster, v(0,0,-1), 1.4, true)
			thruster(vLeftThruster, v(-1,0,0), 1)
			thruster(vRightThruster, v(1,0,0), 1)
			thruster(vTopThruster, v(0,1,0), 1)
			thruster(vBottomThruster, v(0,-1,0), 1)
		else
			--load_obj('viper_col.obj')
			load_obj('viper_col.obj', Matrix.rotate(0.5*math.pi,v(-1,0,0)))
		end
	end,

	dynamic = function(lod)
		if lod > 2 then
			local M_1 = v(0.202,0.807,1.294)
			local M_2 = v(-0.202,0.807,1.294)
			local M_3 = v(0.411,0.806,1.294)
			local M_4 = v(-0.411,0.806,1.294)

			if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
				call_model('d_smart',M_1,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_1,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
				call_model('d_smart',M_2,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_2,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
				call_model('d_smart',M_3,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_3,v(1,0,0), v(0,.95,.05),0.16)
			end

			if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
				call_model('d_unguided',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
				call_model('d_guided',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
				call_model('d_smart',M_4,v(1,0,0), v(0,.95,.05),0.16)
			elseif get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
				call_model('d_naval',M_4,v(1,0,0), v(0,.95,.05),0.16)
			end
		end
	end
})
