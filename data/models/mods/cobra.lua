
define_model('cobra_mk3', {
	info = {
			lod_pixels = { 50, 100, 200, 0 },
			bounding_radius = 40,
			materials = {'default', 'text'},
			tags = {'ship'},
			ship_defs = {
				{
					name='Cobra Mk III',
					forward_thrust = -16e6,
					reverse_thrust = 8e6,
					up_thrust = 4e6,
					down_thrust = -4e6,
					left_thrust = -4e6,
					right_thrust = 4e6,
					angular_thrust = 22e6,
					gun_mounts = 
					{
					{ v(0,-0.5,0), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
					},
					max_cargo = 80,
					max_laser = 2,
					max_missile = 4,
					max_cargoscoop = 0,
					capacity = 80,
					hull_mass = 40,
					fuel_tank_mass = 20,
					thruster_fuel_use = 0.0002,
					price = 124000,
					hyperdrive_class = 3,
				}
			}
		},
	static = function(lod)
	    set_material('default', .6,.6,.6,1,.3,.3,.3,50)
	    set_material('text', .6,.6,.6,1,.3,.3,.3,5)
		use_material('default')
	    texture('cobra3_redux.png')
		load_obj('cobra3_redux.obj', Matrix.new(v(-39,0,0),v(0,33,0),v(0,0,-35)))
	end,
	dynamic = function(lod)
		
		if lod > 1 then
			use_material('text')
			local reg = get_label()
			zbias(1,v(16,0.42,1), v(0,1,-.63))
			text(reg,v(16,0.42,1), v(0,1,-.63), v(-1,.041,-.95), 3, {center = true})
			zbias(1,v(-16,0.42,1), v(0,1,-.63))
			text(reg,v(-16,0.42,1), v(0,1,-.63), v(-1,-.041,.95), 3, {center = true})
			zbias(0)	
		end
		
		if get_animation_position('WHEEL_STATE') ~= 0 then
      		-- wheels
			local v73 = v(0.0, -1.4, -5.5)
			local v74 = v(-4.5, -4.05, 7)
			local v75 = v(4.5, -4.05, 7)
			zbias(1, v73, v(0,-1,0))
			-- nose wheel
			call_model('nosewheelunit', v73, v(-1,0,0), v(0,-1,-.22), 1.2)
			zbias(1, v74, v(0,-1,0))
			-- rear wheels
			call_model('nosewheelunit', v74, v(-1,0,0), v(0,-1,-.22), .64)
			call_model('nosewheelunit', v75, v(-1,0,0), v(0,-1,-.22), .64)
			zbias(0)

            -- lights on wingtips
			local lightphase = math.fmod(get_time('SECONDS'), 1)
			if lightphase > .9 then
				billboard('smoke.png', 10, v(1,1,1), { v(-25.35,-.95,11.375) })
			elseif lightphase > .8 then
				billboard('smoke.png', 10, v(1,1,1), { v(25.35,-.95,11.375) }) -- middle number is how high vertically on the ship
			elseif lightphase > .7 then
				billboard('smoke.png', 10, v(1,1,1), { v(0,-4.95,11.375) })
			end
		end
		
		if get_animation_position('WHEEL_STATE') == 0 then
			local lightphase = math.fmod(get_time('SECONDS'), 1)
			if lightphase > .9 then
				billboard('smoke.png', 10, v(0,1,0), { v(-25.35,-.95,11.375) })
			elseif lightphase > .8 then
				billboard('smoke.png', 10, v(1,0,0), { v(25.35,-.95,11.375) })
			elseif lightphase > .7 then
				billboard('smoke.png', 10, v(1,1,1), { v(0,4.95,11.375) })
			end
		end
				
		local vBackThruster = v(7, 0, 12) -- last number is how far back down the ship it is
		local vFrontThruster = v(4, -.2, -12)

		xref_thruster(vBackThruster, v(0,0,1), 30, true)
		xref_thruster(vFrontThruster, v(0,0,-1), 15, true)

		local TopThrust1 = v(-9, 4.5, 9)
		local TopThrust2 = v(9, 4.5, 9)
		local TopThrustForward = v(0, 1.5, -9)
		local BottomThrust1 = v(-9, -4.5, 9)
		local BottomThrust2 = v(9, -4.5, 9)
		local BottomThrustForward = v(0, -1, -9)

		thruster(TopThrust1, v(0,1,0), 10)
		thruster(TopThrust2, v(0,1,0), 10)
		thruster(TopThrustForward, v(0,1,0), 10)
		thruster(BottomThrust1, v(0,-1,0), 10)
		thruster(BottomThrust2, v(0,-1,0), 10)
		thruster(BottomThrustForward, v(0,-1,0), 10)

		local LeftBackThruster = v(-25, -1, 8.5)
		local LeftForwardThruster = v(-8.5, -.25, -10.0)
		local RightBackThruster = v(25, -1, 8.5)
		local RightForwardThruster = v(8.5, .25, -10.0)

		thruster(LeftBackThruster, v(-1,0,0), 10)
		thruster(LeftForwardThruster, v(-1,0,0), 10)
		thruster(RightBackThruster, v(1,0,0), 10)
		thruster(RightForwardThruster, v(1,0,0), 10)

	end

})
