define_model('ips_r1_flap', {
	info =	{
		bounding_radius =11,
	},

	static = function(lod)
		texture('ips_bottom.png', v(0.305,0.5,0), v(0.162,0,0), v(0,0,0.455))
		extrusion(v(0,0,5.2), v(0,0,-5.2), v(0,1,0), 1, v(0,0,0), v(0,0.05,0), v(-0.55,0.05,0), v(-0.55,0,0))
	end
})

define_model('ips_r2_flap', {
	info =	{
		bounding_radius =11,
	},

	static = function(lod)
		texture('ips_bottom.png', v(0.128,0.5,0), v(0.162,0,0), v(0,0,0.455))
		extrusion(v(0,0,5.2), v(0,0,-5.2), v(0,1,0), 1, v(0,0,0), v(0.55,0,0), v(0.55,0.05,0), v(0,0.05,0))
	end
})

define_model('ips_l1_flap', {
	info =	{
		bounding_radius =11,
	},

	static = function(lod)
		texture('ips_bottom.png', v(0.872,0.5,0), v(0.162,0,0), v(0,0,0.455))
		extrusion(v(0,0,5.2), v(0,0,-5.2), v(0,1,0), 1, v(0,0,0), v(0,0.05,0), v(-0.55,0.05,0), v(-0.55,0,0))
	end
})

define_model('ips_l2_flap', {
	info =	{
		bounding_radius =11,
	},

	static = function(lod)
		texture('ips_bottom.png', v(0.695,0.5,0), v(0.162,0,0), v(0,0,0.455))
		extrusion(v(0,0,5.2), v(0,0,-5.2), v(0,1,0), 1, v(0,0,0), v(0.55,0,0), v(0.55,0.05,0), v(0,0.05,0))
	end
})

define_model('socket', {
	info =	{
		bounding_radius =1,
	},

	static = function(lod)
		local v1 = v(-0.5,-0.5,0.5)
		local v2 = v(0.5,-0.5,0.5)
		local v3 = v(0.5,0.5,0.5)
		local v4 = v(-0.5,0.5,0.5)
		local v5 = v(-0.7,-0.7,-0.5)
		local v6 = v(0.7,-0.7,-0.5)
		local v7 = v(0.7,0.5,-0.5)
		local v8 = v(-0.7,0.5,-0.5)

		texture('shut02.png', v(0,0,0), v(1,0,0), v(0,1,0))
		quad(v1, v2, v3, v4)
		texture('shut02.png', v(0,0,0), v(0,1,0), v(0,0,1))
		quad(v1, v5, v6, v2)
		texture('shut02.png', v(0,0,0), v(0,0,1), v(0,1,0))
		xref_quad(v2, v6, v7, v3)

	end
})

define_model('ips_dash', {
	info =	{
		bounding_radius =5,
		materials = {'dash_lit'},
	},

	static = function(lod)
		set_material('dash_lit', 1,1,1,.99,0,0,0,1,1,1,1)
	end,
	dynamic = function(lod)
		local v14 = v(3.2,0.8,-6.7)
		local v15 = v(-3.2,0.8,-6.7)
		local timer = math.fmod ((get_time('SECONDS')*0.2),1)
		use_material('dash_lit')
		if timer < .17 then
			texture('models/ships/ip_shuttle/dash_lit_01.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
		else
			if timer < .34 then
				texture('models/ships/ip_shuttle/dash_lit_02.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
			else
				if timer < .51 then
					texture('models/ships/ip_shuttle/dash_lit_03.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
				else
					if timer < .68 then
						texture('models/ships/ip_shuttle/dash_lit_04.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
					else
						if timer < .85 then
							texture('models/ships/ip_shuttle/dash_lit_05.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
						else
							if timer > .84 then
								texture('models/ships/ip_shuttle/dash_lit_06.png', v(0.5,0.4,0), v(-.14,0,0), v(0,1.8,0))
							end
						end
					end
				end
			end
		end
		quad(v14+v(0,0.001,0.001), v15+v(0,0.001,0.001), v(-3.2,0.201,-6.201), v(3.2,0.201,-6.201)) -- dash lit
	end
})

define_model('ip_shuttle', {
	info = 	{
		scale = 1.8,
		lod_pixels = {5, 10, 50, 0},
		bounding_radius = 25,
		materials = {'grey', 'win', 'alu', 'anth', 'matvar0', 'text', 'hole', 'dash_lit', 'projector'},
		tags = {'ship'},
	},

	static = function(lod)

		local v00 = v(3.1,2.5,-5.0)
		local v01= v(-3.1,2.5,-5.0)
		local v02 = v(3.1,2.5,6.8)
		local v03 = v(-3.1,2.5,6.8)
		local v04 =	v(3.7,0,-7.5)
		local v05 = v(-3.7,0,-7.5)
		local v06 = v(3.7,0,7.5)
		local v07 = v(-3.7,0,7.5)
		local v08 = v(3.1,-1.2,-6.8)
		local v09 = v(-3.1,-1.2,-6.8)
		local v10 = v(3.1,-1.2,6.8)
		local v11 = v(-3.1,-1.2,6.8)
		local v12 = v(2.8,2.3,-5.2)
		local v13 = v(-2.8,2.3,-5.2)
		local v14 = v(3.2,0.8,-6.7)
		local v15 = v(-3.2,0.8,-6.7)

		set_material('alu',  .5, .51, .65,1, .5, .5, .7, 50)

		use_material('grey')
		if lod > 2 then
			texture('ips_side.png', v(.5,.0,0),  v(0,0,-0.081), v(0,-0.83,0))  -- lower sides
		end
		xref_quad(v10, v08, v04, v06)

		if lod > 2 then
			texture('ips_front.png', v(.5,0,0),  v(0.136,0,0), v(0,0.84,0))  -- lower front
		end
		quad(v08,v09,v05,v04)

		if lod > 2 then
			texture('ips_back.png', v(.5,0,0),  v(0.136,0,0), v(0,0.84,0)) -- lower back
		end
		quad(v11,v10,v06,v07)

		if lod > 1 then
			texture('ips_doors.png', v(.5,.5,0), v(.38,0,0), v(0,.38,0))
		end
		if lod > 2 then
			zbias(1,v(0,0.54,-4.001), v(0,0,-1))
			circle(5, v(0,0.54,-4.001), v(0,0,-1), v(-0.324,1,0), 1.3) -- door cockpit
			zbias(0)
		end
		if lod > 1 then
			use_material('anth')
			zbias(1,v(0,1.2,7.165), v(0,0,1))
			circle(8, v(0,1.2,7.165), v(0,0,1), v(0,1,-0.28), 1.2) -- door outside
			zbias(0)
			texture('shut02.png')
			use_material('grey')
			tube(8, v(0,1.2,7.1), v(0,1.2,7.25), v(0,1,-0.28), 1.1, 1.2)  -- door frame outside
		end
		if lod > 2 then
			use_material('anth')
			tube(5, v(0,0.54,-4.05), v(0,0.54,-4), v(0.324,1,0), 1.15, 1.3)  -- door frame cockpit
			ring(2*lod, v(0,-0.6,-5.5), v(0,0,-5.5), v(0,0,1), 0.2) -- pilot seat base
			use_material('grey')
			texture('dash.png', v(0.5,0.4,0), v(.14,0,0), v(0,1.8,0))
			quad(v14, v15, v(-3.2,0.2,-6.2), v(3.2,0.2,-6.2)) -- dash
			texture('shut02.png', v08, v(0,0.4,0), v(0,0,0.8))
			quad(v13, v12, v(2.8,2.3,-4), v(-2.8,2.3,-4))            -- cockpit top
			quad(v(3.1,-0.6,-7.15), v(-3.1,-0.6,-7.15), v(-3.1,-0.6,-4), v(3.1,-0.6,-4)) -- cockpit floor
			texture('shut02.png', v10, v(0,0.8,0), v(0,0,0.2) )
			xref_quad(v12, v(3.4,0,-7.5), v(3.4,0,-4), v(2.8,2.3,-4))  -- cockpit side
			xref_quad(v(3.4,0,-7.5), v(3.1,-0.6,-7.15), v(3.1,-0.6,-4), v(3.4,0,-4))
			texture('shut02.png', v08, v(0.4,0,0), v(0,0.8,0) )
			quad(v(3.1,-0.6,-4), v(-3.1,-0.6,-4), v(-3.4,0,-4), v(3.4,0,-4)) -- cockpit back
			quad(v(3.4,0,-4), v(-3.4,0,-4), v(-2.8,2.3,-4), v(2.8,2.3,-4))
		end
		use_material('matvar0')
		texture('shut08.png', v(0.5,0.4,0), v(0.16,0,0), v(0,0,0.5)) -- top
		quad(v02,v00,v01,v03)

		texture('shut08.png', v(0.89,0.6,0.0), v(0,0,0.5), v(0,0.2,0)) -- upper sides
		xref_quad(v06,v04,v00,v02)

		texture('shut08.png', v(0.4,0.25,0), v(0.12,0,0), v(0,0.2,0)) -- front
		if lod > 2 then
			xref_quad(v00,v04,v14,v12)  -- upper front
			quad(v00,v12,v13,v01) -- upper front
			quad(v04,v05,v15,v14) -- upper front
		else
			quad(v00,v04,v05,v01) -- upper front when far
		end
		texture('shut08.png', v(0.5,0.5,0), v(0,0.12,0), v(0.2,0.0))
		quad(v07,v06,v02,v03) -- upper back
		texture(nil)
		if lod > 1 then
			use_material('anth')
			call_model('socket', v(2.5,-0.6,6.97), v(1,0,0), v(0,1,0), .85) -- socket main
			call_model('socket', v(-2.5,-0.6,6.97), v(1,0,0), v(0,1,0), .85)
		end
		if lod > 2 then
			call_model('socket', v(2.5,-0.6,-7.05), v(-1,0,0), v(0,1,0), 0.5)  -- socket retro
			call_model('socket', v(-2.5,-0.6,-7.05), v(-1,0,0), v(0,1,0), 0.5)
			call_model('socket', v(3.66,-0.25,-3), v(0,0,-1), v(0,1,0), 0.5)  -- sockets right
			call_model('socket', v(3.66,0.25,-3), v(0,0,1), v(0,-1,0), 0.5)
			call_model('socket', v(3.66,-0.25,5), v(0,0,-1), v(0,1,0), 0.5)
			call_model('socket', v(3.66,0.25,5), v(0,0,1), v(0,-1,0), 0.5)
			call_model('socket', v(-3.66,-0.25,-3), v(0,0,1), v(0,1,0), 0.5)  -- sockets left
			call_model('socket', v(-3.66,0.25,-3), v(0,0,-1), v(0,-1,0), 0.5)
			call_model('socket', v(-3.66,-0.25,5), v(0,0,1), v(0,1,0), 0.5)
			call_model('socket', v(-3.66,0.25,5), v(0,0,-1), v(0,-1,0), 0.5)
		end

		texture(nil)

		if lod >= 2 then
			zbias(1,v(3.56,0.6,-5), v(1,0,0))
			call_model('decal', v(3.56,0.6,-5), v(1,0,0), v(-0.235,1,0), 1)  -- decals
			zbias(1,v(-3.56,0.6,-4), v(-1,0,0))
			call_model('decal', v(-3.56,0.6,-4), v(-1,0,0), v(0.235,1,0), 1)
			zbias(1,v(3.176,-1.05,-5), v(1,0,0))
			call_model('squadsign_1', v(3.176,-1.05,-5), v(1,0,0), v(0.5,1,0),1)  -- squadron label
			zbias(1,v(-3.176,-1.05,-4), v(-1,0,0))
			call_model('squadsign_1', v(-3.176,-1.05,-4), v(-1,0,0), v(-0.5,1,0),1)
			zbias(0)

			call_model('nazzle_n', v(2.5,-0.6,-7.55), v(1,0,0), v(0,0,-1),0.2)  -- reverse nazzels
			call_model('nazzle_n', v(-2.5,-0.6,-7.55), v(1,0,0), v(0,0,-1),0.2)

			call_model('nazzle_n', v(-3.7,.8,-3), v(1,0,0), v(0,1,0),0.2) -- left front top nazzel
			call_model('nazzle_n', v(-3.7,-.8,-3), v(1,0,0), v(0,-1,0),0.2) -- left front bottom nazzel
			call_model('nazzle_n', v(3.7,.8,-3), v(1,0,0), v(0,1,0),0.2) -- right front top nazzel
			call_model('nazzle_n', v(3.7,-.8,-3), v(1,0,0), v(0,-1,0),0.2) -- right front bottom nazzel

			call_model('nazzle_n', v(-3.7,.8,5), v(1,0,0), v(0,1,0),0.2) -- left back top nazzel
			call_model('nazzle_n', v(-3.7,-.8,5), v(1,0,0), v(0,-1,0),0.2) -- left back bottom nazzel
			call_model('nazzle_n', v(3.7,.8,5), v(1,0,0), v(0,1,0),0.2) -- right back top nazzel
			call_model('nazzle_n', v(3.7,-.8,5), v(1,0,0), v(0,-1,0),0.2) -- right back bottom nazzel

			call_model('nazzle_n', v(-4.2,0,-3), v(0,1,0), v(-1,0,0),0.2) -- left front nazzel
			call_model('nazzle_n', v(-4.2,0,5), v(0,1,0), v(-1,0,0),0.2) -- left back nazzel
			call_model('nazzle_n', v(4.2,0,-3), v(0,1,0), v(1,0,0),0.2) -- right front nazzel
			call_model('nazzle_n', v(4.2,0,5), v(0,1,0), v(1,0,0),0.2) -- right back nazzel
		end

		if lod > 1 then

			if lod < 4 then
				call_model('nazzle_s', v(2.5,-0.6,7.9), v(1,0,0), v(0,0,1),.4)  -- main nazzels
				call_model('nazzle_s', v(-2.5,-0.6,7.9), v(1,0,0), v(0,0,1),.4)
			else
				call_model('nazzle_l', v(2.5,-0.6,7.9), v(1,0,0), v(0,0,1),.4)
				call_model('nazzle_l', v(-2.5,-0.6,7.9), v(1,0,0), v(0,0,1),.4)
			end
		end

		if lod > 2 then
			call_model('pilot1', v(0,0.9,-5.5), v(1,0,0), v(0,1,0), 0.25)
			call_model('ips_dash',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end

		if lod > 2 then
			texture(nil)
			set_material('win', 0,0,0.05, .7, 1.7, 1.75, 2, 100,0.1,0.1,0.1)
			use_material('win')
			quad(v14,v15,v13,v12)
		else
			texture(nil)
			set_material('win', 0,0,0.05, 1, 1.7, 1.75, 2, 100,0.1,0.1,0.1)
			use_material('win')
			quad(v14-v(0,0,.01),v15-v(0,0,.01),v13-v(0,0,.01),v12-v(0,0,.01))
		end

		if lod > 1 then
			use_material('projector')
			sphere_slice(4*lod,lod, 0, 0.5*math.pi, Matrix.translate(v(0,-1.2,0)) * Matrix.rotate(math.pi,v(0,0,1)) * Matrix.scale(v(0.7,0.3,0.7)))
			sphere_slice(4*lod,lod, 0, 0.5*math.pi, Matrix.translate(v(0,2.5,0)) * Matrix.scale(v(0.7,0.3,0.7)))

			call_model('headlight', v(0,-0.9,-6.95), v(1,0,0), v(0,-0.5,-1),1.2)
			call_model('posl_red', v(-3.64,0.2,1), v(0,0,1), v(-1,0.27,0),1)
			call_model('posl_green', v(3.64,0.2,1), v(0,0,1), v(1,0.27,0),1)
			call_model('posl_white', v(0,2.5,6.6), v(1,0,0), v(0,1,0),1)
			call_model('coll_warn', v(-3.6,-0.2,1), v(0,0,1), v(-1,-0.5,0),1)
			call_model('coll_warn', v(3.6,-0.2,1), v(0,0,1), v(1,-0.5,0),1)
			call_model('coll_warn', v(0,-1.2,6.6), v(1,0,0), v(0,-1,0),1)
		end

		call_model('blank',v(0,0,0),v(1,0,0),v(0,1,0),0)

		local BackThrust = v(2.5, -0.6, 8)
		local FrontThrust = v(2.5, -0.6, -7.6)

		xref_thruster(BackThrust, v(0,0,1), 6, true)
		xref_thruster(FrontThrust, v(0,0,-1), 3, true)

		local LeftFrontTopThrust = v(-3.7,.85,-3)
		local RightFrontTopThrust = v(3.7, .85, -3)
		local LeftFrontBottomThrust = v(-3.7, -.85, -3)
		local RightFrontBottomThrust = v(3.7, -.85, -3)

		thruster(LeftFrontTopThrust, v(0,1,0), 2)
		thruster(RightFrontTopThrust, v(0,1,0), 2)
		thruster(LeftFrontBottomThrust, v(0,-1,0), 2)
		thruster(RightFrontBottomThrust, v(0,-1,0), 2)

		local LeftBackTopThrust = v(-3.7, .85, 5)
		local RightBackTopThrust = v(3.7, .85, 5)
		local LeftBackBottomThrust = v(-3.7, -.85, 5)
		local RightBackBottomThrust = v(3.7, -.85, 5)

		thruster(LeftBackTopThrust, v(0,1,0), 2)
		thruster(RightBackTopThrust, v(0,1,0), 2)
		thruster(LeftBackBottomThrust, v(0,-1,0), 2)
		thruster(RightBackBottomThrust, v(0,-1,0), 2)

		local BackLeftThrust = v(-4.25, 0, 5)
		local BackRightThrust = v(4.25, 0, 5)

		thruster(BackLeftThrust, v(-1,0,0), 2)
		thruster(BackRightThrust, v(1,0,0), 2)

		local FrontLeftThrust = v(-4.25, 0, -3)
		local FrontRightThrust = v(4.25, 0, -3)

		thruster(FrontLeftThrust, v(-1,0,0), 2)
		thruster(FrontRightThrust, v(1,0,0), 2)
	end,

	dynamic = function(lod)

		local v08 = v(3.1,-1.2,-6.8)
		local v09 = v(-3.1,-1.2,-6.8)
		local v10 = v(3.1,-1.2,6.8)
		local v11 = v(-3.1,-1.2,6.8)
		local v12 = v(2.8,2.3,-5.2)
		local v13 = v(-2.8,2.3,-5.2)
		local v14 = v(3.2,0.8,-6.7)
		local v15 = v(-3.2,0.8,-6.7)
		local v16 = v(-3.37,1.4,1)
		local v17 = v(3.37,1.4,1)
		local v32 = v(2.3,-1.2,5.2)
		local v33 = v(1.2,-1.2,5.2)
		local v34 = v(1.2,-1.2,-5.2)
		local v35 = v(2.3,-1.2,-5.2)
		local v36 = v(-1.2,-1.2,5.2)
		local v37 = v(-2.3,-1.2,5.2)
		local v38 = v(-2.3,-1.2,-5.2)
		local v39 = v(-1.2,-1.2,-5.2)
		local v40 = v(2.3,-0.7,5.2)
		local v41 = v(1.2,-0.7,5.2)
		local v42 = v(1.2,-0.7,-5.2)
		local v43 = v(2.3,-0.7,-5.2)
		local v99 = v(0,2.49,3)


		set_material('matvar0', get_arg_material(0))
		set_material('projector', lerp_materials(get_time('SECONDS')*0.5,	{.5,.51,.65,.7,1.7,1.75,2,100,.08,.081,.1},       -- shield projectors
		{.5,.51,.65,.7,1.7,1.75,2,100,.3,.31,.5}))
		if lod > 2 then
			set_material('grey', 0.5, 0.5, 0.5, 1, 0.5, 0.5, 0.5, 10)
			set_material('anth', .2,.2,.2,1,.3,.3,.3,10)
		else
			set_material('grey', 0.3, 0.3, 0.3, 1, 0.35, 0.35, 0.35, 10)
			set_material('anth', .1,.1,.1,1,.15,.15,.15,10)
		end
		use_material('grey')
		if lod > 1 then
			if lod > 2 then
				texture('models/ships/ip_shuttle/ips_bottom.png', v(0.5,0.5,0), v(0.162,0,0), v(0,0,0.455))
			end
			if get_animation_position('WHEEL_STATE') ~= 0 then

				quad(v10,v32,v35,v08) -- bottom uc engaged
				quad(v10,v11,v33,v32)
				quad(v11,v37,v36,v33)
				quad(v37,v11,v09,v38)
				quad(v39,v38,v09,v08)
				quad(v08,v35,v34,v39)
				quad(v33,v36,v39,v34)

				local flap = 1.2*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, 0.6)  -- flap factor

				call_model('ips_r1_flap', v(1.2,-1.2,0), v(0,-1,0), v(math.sin(flap),math.cos(flap),0), 1)  -- right uc flaps
				call_model('ips_r2_flap', v(2.3,-1.2,0), v(0,1,0), v(-math.sin(flap),math.cos(flap),0), 1)
				call_model('ips_l1_flap', v(-2.3,-1.2,0), v(0,-1,0), v(math.sin(flap),math.cos(flap),0), 1)  -- left uc flaps
				call_model('ips_l2_flap', v(-1.2,-1.2,0), v(0,1,0), v(-math.sin(flap),math.cos(flap),0), 1)

				texture('models/ships/ip_shuttle/shut02.png', v(0.5,0.5,0), v(0,0.4,0), v(0,0,0.8))
				use_material('anth')
				xref_quad(v40,v41,v42,v43) -- uc cage top
				texture('models/ships/ip_shuttle/shut02.png', v(0,0,0), v(0,0.8,0), v(0,0,0.2) )
				xref_quad(v32,v40,v43,v35) -- uc cage sides
				xref_quad(v33,v34,v42,v41)
				texture('models/ships/ip_shuttle/shut02.png', v(0,0,0), v(0.4,0,0), v(0,0.8,0) )
				xref_quad(v32,v33,v41,v40) -- uc cage back
				xref_quad(v34,v35,v43,v42) -- uc cage front

				texture(nil)
				use_material('alu')

				local uc_rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0.14, 1)  -- uc factor
				local uc_trans = 1.9*math.clamp(get_animation_position('WHEEL_STATE'), 0.14, 1)

				xref_ring(3*lod, v(1.4,-0.6,-2.5), v(1.4+uc_rot,-0.6-uc_trans,-2.5), v(1,1,0),0.08)  -- under carriage
				xref_ring(3*lod, v(1.4,-0.6,2.5), v(1.4+uc_rot,-0.6-uc_trans,2.5), v(1,1,0),0.08)
				xref_cylinder(3*lod, v(1.4+uc_rot,-0.6-uc_trans,-5.0), v(1.4+uc_rot,-0.6-uc_trans,5.0), v(0,1,0),0.1)

			else
				quad(v08, v10, v11, v09) -- bottom uc not engagaged
			end
		else
			quad(v08, v10, v11, v09) -- bottom uc engaged when far away
		end

		if lod > 1 then
			set_material('text', .6,.6,.6, 1)
			use_material('text')
			reg = get_label()
			texture(nil)
			zbias(1,v(-1,0.235,0), v(0,0,1))
			text(reg, v16, v(-1,0.235,0), v(0,0,1), 1.7, {center = true})  -- text label
			zbias(1,v(1,0.235,0), v(0,0,-1))
			text(reg, v17, v(1,0.235,0), v(0,0,-1), 1.7, {center = true})
			zbias(0)
			if get_equipment('SCANNER') == 'SCANNER' then
				use_material('grey')
				call_model('scanner_+', v(0,2.5,4), v(1,0,0), v(0,1,0),0.75)
			end
			if lod > 2 then
				if get_equipment('LASER', 1) then
					use_material('alu')
					cylinder(2*lod, v(0,-0.3,-7), v(0,-0.3,-7.9), v(0,1,0),0.08) -- laser
					sphere_slice(2*lod,lod, 0, 0.5*math.pi, Matrix.translate(v(0,-0.3,-7.2)) * Matrix.rotate(0.5*math.pi,v(-1,0,0)) * Matrix.scale(v(0.2,0.3,0.2)))
					set_material('hole', .1,.1,.1,1)
					use_material('hole')
					circle(2*lod, v(0,-0.3,-7.901), v(0,0,-1), v(0,1,0), 0.06)
				end
			end
		end
	end
})
