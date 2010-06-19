
define_model('cobra_mk3', {
	info = {
			lod_pixels = { 50, 100, 200, 0 },
			bounding_radius = 100,
			materials = {'thrusters', 'text'},
			tags = {'ship'},
			ship_defs = {
				{
					'Cobra Mk III', 
					{ 2*10^7,-2*10^7,1*10^7,-1*10^7,-1*10^7,1*10^7 },
					4*10^7,
					{
					{ v(0,-0.5,0), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
					},
					{ 90, 1, 2, 8, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
					90, 100, 16000000,
					1
				}
			}
		},
	static = function(lod)
		load_obj('cobra3_redux.obj', Matrix.new(v(45,0,0),v(0,45,0),v(0,0,45)) * Matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1))) 
		
		set_material('thrusters', .30, .30, .30,1, .30, .30, .30, 20)
		use_material('thrusters')

		local vBackThruster = v(8, 0.7, 14.5) -- last number is how far back down the ship it is
		local vFrontThruster = v(3.5, -1.0, -5.0)

		xref_thruster(vBackThruster, v(0,0,1), 30, true)
		xref_thruster(vFrontThruster, v(0,0,-1), 20, true)

		local TopThrust1 = v(-9.0, 1.5, 11.0) 
		local TopThrust2 = v(9.0, 1.5, 11.0)
		local TopThrustForward = v(0.0, 1.5, -11.0)
		local BottomThrust1 = v(-9.0, -0.5, 10.0)
		local BottomThrust2 = v(9.0, -0.5, 10.0)
		local BottomThrustForward = v(0.0, -0.5, -10.0)

		thruster(TopThrust1, v(0,1,0), 15)
		thruster(TopThrust2, v(0,1,0), 15)
		thruster(TopThrustForward, v(0,1,0), 15)
		thruster(BottomThrust1, v(0,-1,0), 15)
		thruster(BottomThrust2, v(0,-1,0), 15)
		thruster(BottomThrustForward, v(0,-1,0), 15)

		local LeftBackThruster = v(-18.0, 0.0, 10.0)
		local LeftForwardThruster = v(-3.5, 0.0, -10.0)
		local RightBackThruster = v(18.0, 0.0, 10.0)
		local RightForwardThruster = v(3.5, 0.0, -10.0)

		thruster(LeftBackThruster, v(-1,0,0), 15)
		thruster(LeftForwardThruster, v(-1,0,0), 15)
		thruster(RightBackThruster, v(1,0,0), 15)
		thruster(RightForwardThruster, v(1,0,0), 15)
	end,
	dynamic = function(lod)
		if get_arg(0) ~= 0 then

			-- lights on wingtips
			local lightphase = math.fmod(get_arg(1), 1)
			if lightphase > .9 then
				billboard('smoke.png', 10, v(1,1,1), { v(-29, 0, 14) })
			elseif lightphase > .8 then
				billboard('smoke.png', 10, v(1,1,1), { v(29, 0, 14) }) -- middle number is how high vertically on the ship 
			end

			-- wheels
			local v73 = v(0.0, -1.0, -5.5)
			local v74 = v(-4.5, -4.5, 10.5)
			local v75 = v(4.5, -4.5, 10.5)
			zbias(1, v73, v(0,-1,0))
			-- nose wheel
			call_model('nosewheelunit', v73, v(-1,0,0), v(0,-1,0), 1.2)
			zbias(1, v74, v(0,-1,0))
			-- rear wheels
			call_model('nosewheelunit', v74, v(-1,0,0), v(0,-1,0), .64)
			call_model('nosewheelunit', v75, v(-1,0,0), v(0,-1,0), .64)
			zbias(0)
		end
	end

})
