--[[ position lights including bulbs for a four times flashing position lights setting
period; 1st green to set left, 2nd red to set right, 3rd light blue to set at top in back of ship
when LG engaged, light blue flashing lights at period 4 to set at bottom in back, left and right sides as collision warners
and a white headlight to set at front (optional)
]]--

define_model('posl_green', {
	info = 	{
		lod_pixels = {1, 3, 6, 0},
		bounding_radius = 1,
		materials = {'green'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('green', 0, .85, 0, .5, 1, 1, 1, 100, 0, 0, 0)
			use_material('green')
			sphere_slice(3*lod, 1*lod, 0, 0.5*math.pi, Matrix.scale(v(0.1,0.1,0.1)))
		end
	end,
	dynamic = function(lod)
		if ((get_flight_state() == 'DOCKING') or (get_flight_state() == 'FLYING' and get_animation_position('WHEEL_STATE') ~= 0)) then
			if lod > 1 then
				set_material('green', 0, .85, 0, .5, 1, 1, 1, 100, 0, 0, 0)
			end
			local lightphase = math.fmod((get_time('SECONDS')*0.75),1)
			if lightphase > .1 then
				if lightphase  < .3 then
					if lod > 1 then
						set_material('green', 0, .85, 0, 1, 0, 0, 0, 0, 0, .85, 0)
					end
					billboard('smoke.png', 2,  v(0,.85,0), { v(0, 0.12, 0) })
				end
			end
		end
	end
})

define_model('posl_red', {
	info = 	{
		lod_pixels = {1, 3, 6, 0},
		bounding_radius = 1,
		materials = {'red'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('red', .9, 0, 0, .6, 1, 1, 1, 100, 0, 0, 0)
			use_material('red')
			sphere_slice(3*lod, 1*lod, 0, 0.5*math.pi, Matrix.scale(v(0.1,0.1,0.1)))
		end
	end,
	dynamic = function(lod)
		if ((get_flight_state() == 'DOCKING') or (get_flight_state() == 'FLYING' and get_animation_position('WHEEL_STATE') ~= 0)) then
			if lod > 1 then
				set_material('red', .9, 0, 0, .6, 1, 1, 1, 100, 0, 0, 0)
			end
			local lightphase = math.fmod((get_time('SECONDS')*0.75),1)
			if lightphase  > .3 then
				if lightphase < .5 then
					if lod > 1 then
						set_material('red', .9, 0, 0, 1, 0, 0, 0, 0, .9, 0, 0)
					end
					billboard('smoke.png', 2,  v(1,0,0), { v(0, 0.12, 0) })
				end
			end
		end
	end
})

define_model('posl_white', {
	info = 	{
		lod_pixels = {1, 3, 6, 0},
		bounding_radius = 1,
		materials = {'blue_white'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('blue_white', .8, .85, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			use_material('blue_white')
			sphere_slice(3*lod, 1*lod, 0, 0.5*math.pi, Matrix.scale(v(0.1,0.1,0.1)))
		end
	end,
	dynamic = function(lod)
		if ((get_flight_state() == 'DOCKING') or (get_flight_state() == 'FLYING' and get_animation_position('WHEEL_STATE') ~= 0)) then
			if lod > 1 then
				set_material('blue_white', .8, .85, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			end
			local lightphase = math.fmod((get_time('SECONDS')*0.75),1)
			if lightphase  > .5 then
				if lightphase < .7 then
					if lod > 1 then
						set_material('blue_white', .8, .85, 1, 1, 0, 0, 0, 0, .7, .75, 1)
					end
					billboard('smoke.png', 2,  v(.7,.75,1), { v(0, 0.12, 0) })
				end
			end
		end
	end
})

define_model('coll_warn', {
	info = 	{
		lod_pixels = {1, 3, 6, 0},
		bounding_radius = 1,
		materials = {'blue_white'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('blue_white', .8, .85, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			use_material('blue_white')
			sphere_slice(3*lod, 1*lod, 0, 0.5*math.pi, Matrix.scale(v(0.1,0.1,0.1)))
		end
	end,
	dynamic = function(lod)
		if ((get_flight_state() == 'DOCKING') or (get_flight_state() == 'FLYING' and get_animation_position('WHEEL_STATE') ~= 0)) then
			if lod > 1 then
				set_material('blue_white', .8, .85, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			end
			if get_animation_position('WHEEL_STATE') ~= 0 then
				local lightphase = math.fmod((get_time('SECONDS')*0.75),1)
				if lightphase  > .7 then
					if lightphase < .9 then
						if lod > 1 then
							set_material('blue_white', .8, .85, 1, 1, 0, 0, 0, 0, .7, .75, 1)
						end
						billboard('smoke.png', 2,  v(.7,.75,1), { v(0, 0.12, 0) })
					end
				end
			end
		end
	end
})

define_model('headlight', {
	info = 	{
		lod_pixels = {1, 3, 6, 0},
		bounding_radius = 1,
		materials = {'white'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('white', .9, .95, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			use_material('white')
			sphere_slice(3*lod, 1*lod, 0, 0.5*math.pi, Matrix.scale(v(0.1,0.1,0.1)))
		end
	end,
	dynamic = function(lod)
		if ((get_flight_state() == 'DOCKING') or (get_flight_state() == 'FLYING' and get_animation_position('WHEEL_STATE') ~= 0)) then
			if lod > 1 then
				set_material('white', .9, .95, 1, .5, 1, 1, 1, 100, 0, 0, 0)
			end
			if get_animation_position('WHEEL_STATE') ~= 0 then
				if lod > 1 then
					set_material('white', .9, .95, 1, 1, 0, 0, 0, 0, .8, .85, 1)
				end
				billboard('smoke.png', 2,  v(.8,.85,1), { v(0, 0.12, 0) })
			end
		end
	end
})
