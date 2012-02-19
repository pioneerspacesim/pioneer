-- Currently called "turtle" spacecraft, low relative dv (to something a lot bigger), high 3d impulse manned maneuvering vehicle, for uncooperative intercept in a high number threat environment.

-- +++large lateral thrusters for high lateral g, heavy armoured top hull, 360 active sensors, 360 passive sensors, large missile/drone bays, countermeasures, rapid pulsed impulse g handling active gel seating, high automation with man in the loop "vetoe", highly redundant triplex main engines, numerous rcs outlets, jettisonable power module, manual features inc eva friendly design and shielded windows for manual astrogation.

-- ++armoured bottom hull, misleading attitude ambiguous shape, variety of camoflage options for long term planetary storage, small size

-- -small size small storage, single up/down thrusters, non aero friendly shape - designed for space, top hull +++ kinetic/radiative armor, bottom hull ++ thermal, man in the loop fighter

-- strutanim for landing struts/gear, based on sidewinder.lua
define_model('strutanim', {
	info = {
		lod_pixels = { 5, 10, 20, 40},
		bounding_radius = 5,
		materials={'strut'}
	},
	static = function(lod)
		set_material('strut',0.15,0.15,0.15,1,0,0,0,0,0,0,0)
	end,
	dynamic = function(lod)
		use_material('strut')
		local strut_pos = math.clamp(get_animation_position('WHEEL_STATE'),0,1)
		cylinder(4*lod,v(0,0,0), v(0+1*strut_pos,0+3*strut_pos,0), v(0,0,1), 1,1)
	end
})

define_model('turtle_c0', {
	info = {
		lod_pixels={0.1,82,510,1000},
		bounding_radius = 30,
		materials={'turtle'}
	},
	static = function(lod)
		set_material('turtle', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
		use_material('turtle')
		if lod == 2 then
			texture('turtlelod2.png')
			load_obj('turtlelod2.obj')
		elseif lod > 2 then
			texture('bottom.png')
			load_obj('bottom.obj')
			texture('top.png')
			load_obj('top.obj')
		end
		texture(nil)
	end
})

define_model('turtle_c1', {
	info = {
		lod_pixels={0.1,82,510,1000},
		bounding_radius = 30,
		materials={'turtle'}
	},
	static = function(lod)
		set_material('turtle', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
		use_material('turtle')
		if lod == 2 then
			texture('turtlelod2camo1.png')
			load_obj('turtlelod2.obj')
		elseif lod > 2 then
			texture('bottom1.png')
			load_obj('bottom.obj')
			texture('top1.png')
			load_obj('top.obj')
		end
		texture(nil)
	end
})

define_model('turtle_c2', {
	info = {
		lod_pixels={0.1,82,510,1000},
		bounding_radius = 30,
		materials={'turtle'}
	},
	static = function(lod)
		set_material('turtle', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
		use_material('turtle')
		if lod == 2 then
			texture('turtlelod2camo2.png')
			load_obj('turtlelod2.obj')
			texture(nil)
		elseif lod > 2 then
			texture('bottom2.png')
			load_obj('bottom.obj')
			texture('top2.png')
			load_obj('top.obj')
		end
		texture(nil)
	end
})

-- Turtle high 3d impulse fighter
define_model('turtle', {
	info = {
		scale = 0.9,
		lod_pixels={0.1,82,510,1000},
		bounding_radius = 30,
		materials = {'turtle', 'text', 'glow'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Turtle',
				forward_thrust = -10e6,
				reverse_thrust = 10e6,
				up_thrust = 10e6,
				down_thrust = -10e6,
				left_thrust = -10e6,
				right_thrust = 10e6,
				angular_thrust = 30e6,
				gun_mounts =
				{
					{ v(0,-4,-10.2), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 90,
				max_laser = 2,
				max_missile = 4,
				max_cargoscoop = 0,
				capacity = 90,
				hull_mass = 50,
				fuel_tank_mass = 5,
				thruster_fuel_use = 0.0004,
				price = 250000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		set_material('turtle', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)

		if lod == 1 then
			load_obj('turtlelod1.obj')
		elseif lod > 2 then
			use_material('turtle')
			texture('access.png')
			load_obj('access.obj')
			texture('componen.png')
			load_obj('componen.obj')
			texture('lateralt.png')
			load_obj('lateralt.obj')
			texture('missileb.png')
			load_obj('missileb.obj')
			texture('module.png')
			load_obj('module.obj')
			texture('sensors.png')
			load_obj('sensors.obj')
			texture('sides.png')
			load_obj('sides.obj')
			texture('vertical.png')
			load_obj('vertical.obj')
			texture('weapon.png')
			load_obj('weapon.obj')
			texture('glowing.png')
			load_obj('glowingb.obj')
			use_material('glow')
			load_obj('glowing.obj')
			texture(nil)
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			selector2()
			if select2 < 34 then
				call_model('turtle_c0', v(0,0,0), v(1,0,0), v(0,1,0),1)
			elseif select2 < 67 then
				call_model('turtle_c1', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				call_model('turtle_c2', v(0,0,0), v(1,0,0), v(0,1,0),1)
			end
		end

		if lod > 2 then
			-- glowing parts thanks to s2odan, found had to add directory directions to texture and load_obj otherwise wouldnt work, dont know why
			set_material('glow',lerp_materials(get_time('SECONDS'),{0,0,0,.8,0,0,0,0,10,10,10},{0,0,0,.4,0,0,0,0,9,9,9}))
			-- text on ship L first
			-- first number id, then center, then normal, then vector direction

			set_material('text', .6,.6,.6,1,.3,.3,.3,5)
			use_material('text')
			text(get_label(), v(-13.2,-5.7,0), v(-0.55,-1,0), v(0,0,1), 1.1, {center = true})
			text(get_label(), v(13.2,-5.71,0), v(0.54,-1,0), v(0,0,-1), 1.1, {center = true})
		end

		-- landing gear
		if get_animation_position('WHEEL_STATE') ~= 0 then
			local FL = v(-8.9, -6.6, -7.3)
			local FR = v(8.9, -6.6, -7.3)
			local RL = v(-10.2, -6.6, 5.4)
			local RR = v(10.2, -6.6, 5.4)

			call_model('strutanim', FL, v(-0.5,0,-0.5), v(0,-1,0), 0.7)
			call_model('strutanim', FR, v(0.5,0,-0.5), v(0,-1,0), 0.7)
			call_model('strutanim', RL, v(-0.5,0,0.5), v(0,-1,0), 0.7)
			call_model('strutanim', RR, v(0.5,0,0.5), v(0,-1,0), 0.7)
		end
		if lod > 1 then
			call_model('posl_red', v(-12.163,-2.189,-12.187), v(0,1,0), v(-12.1,-2.2,-12.1),2)
			call_model('posl_red', v(-12.163,-2.189,12.187), v(0,1,0), v(-12.1,-2.2,12.1),2)
			call_model('posl_green', v(12.163,-2.189,-12.187), v(0,1,0), v(12.1,-2.2,-12.1),2)
			call_model('posl_green', v(12.163,-2.189,12.187), v(0,1,0), v(12.1,-2.2,12.1),2)

			call_model('posl_red', v(-12.163,2.504,-12.187), v(0,1,0), v(-12.1,2.5,-12.1),2)
			call_model('posl_red', v(-12.163,2.504,12.187), v(0,1,0), v(-12.1,-2.5,12.1),2)
			call_model('posl_green', v(12.163,2.504,-12.187), v(0,1,0), v(12.1,2.5,-12.1),2)
			call_model('posl_green', v(12.163,2.504,12.187), v(0,1,0), v(12.1,-2.5,12.1),2)

			call_model('coll_warn',  v(-4.797,-6.846,9.772), v(0,1,0), v(-4.7,-6.8,9.7),2)
			call_model('coll_warn',  v(4.795,-6.848,9.772), v(0,1,0), v(4.7,-6.8,9.7),2)
			call_model('coll_warn',  v(-3.443,-6.664,-10.073), v(0,1,0), v(-3,-6.6,-10),2)
			call_model('coll_warn',  v(3.441,-6.664,-10.073), v(0,1,0), v(3,-6.6,-10),2)

			call_model('headlight',  v(0,-6.936,-9.595), v(0,1,0), v(0,-7,-9.5),2)

			if lod > 2 then
				-- missiles
				-- missile bays L2 L1 R1 L2 1 inside 2 outside
				local L1 = v(-8.8,-4,0)
				local L2 = v(-12.4,-4,0)
				local R1 = v(8.8,-4,0)
				local R2 = v(12.4,-4,0)

				-- unguided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_unguided',L1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_unguided',R1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_unguided',L2,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_unguided',R2,v(1,0,0),v(0,1,0),2.5)
				end

				-- guided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_guided',L1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_guided',R1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_guided',L2,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_guided',R2,v(1,0,0),v(0,1,0),2.5)
				end

				-- smart missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_smart',L1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_smart',R1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_smart',L2,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_smart',R2,v(1,0,0),v(0,1,0),2.5)
				end

				-- naval missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_naval',L1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_naval',R1,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_naval',L2,v(1,0,0),v(0,1,0),2.5)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),2.5)
					call_model('d_naval',R2,v(1,0,0),v(0,1,0),2.5)
				end
			end

			-- back thrusters
			local backtop1 = v(-1.9, 0.2, 12.9)
			local backtop2 = v(0, 0.2, 12.9)
			xref_thruster(backtop1, v(0,0,1), 11, true)
			thruster(backtop2, v(0,0,1), 11, true)
			local backleft1 = v(-5.5, -3.6, 11.4)
			local backleft2 = v(-3.5, -3.6, 11.4)
			local backleft3 = v(-1.6, -3.6, 11.4)
			xref_thruster(backleft1, v(0,0,1), 11, true)
			xref_thruster(backleft2, v(0,0,1), 11, true)
			xref_thruster(backleft3, v(0,0,1), 11, true)

			-- front thrusters
			local front1 = v(2, 0.2, -12.9)
			local front2 = v(0, 0.2, -12.9)
			xref_thruster(front1, v(0,0,-1), 11, true)
			thruster(front2, v(0,0,-1), 11, true)

			-- side thrusters 3 left and 3 right, left is -x, right is +x
			local left1 = v(-12.8, 0.2, -2.1)
			local left2 = v(-12.8, 0.2, 0)
			local left3 = v(-12.8, 0.2, 2.1)
			thruster(left1, v(-1,0,0), 11, true)
			thruster(left2, v(-1,0,0), 11, true)
			thruster(left3, v(-1,0,0), 11, true)
			local right1 = v(12.8, 0.2, 2.1)
			local right2 = v(12.8, 0.2, 0)
			local right3 = v(12.8, 0.2, -2.1)
			thruster(right1, v(1,0,0), 11, true)
			thruster(right2, v(1,0,0), 11, true)
			thruster(right3, v(1,0,0), 11, true)

			-- lateral top and bottom thrusters
			local top1 = v(0, 3.7, 0)
			thruster(top1, v(0,1,0), 25, true)
			local bottom1 = v(0, -3.6, 0.1)
			thruster(bottom1, v(0,-1,0), 25, true)

			-- RCS TOP YAW CW (1 starts at front left)
			local topyawcw1 = v(-11.9, 3.1, -11.8)
			thruster(topyawcw1, v(-1,0,1), 1)
			local topyawcw2 = v(11.8, 3.1, -11.9)
			thruster(topyawcw2, v(-1,0,-1), 1)
			local topyawcw3 = v(11.9, 3.1, 11.8)
			thruster(topyawcw3, v(1,0,-1), 1)
			local topyawcw4 = v(-11.8, 3.1, 11.9)
			thruster(topyawcw4, v(1,0,1), 1)

			-- RCS BOTTOM YAW CW
			local bottomyawcw1 = v(-11.9, -2.8, -11.8)
			thruster(bottomyawcw1, v(-1,0,1), 1)
			local bottomyawcw2 = v(11.8, -2.8, -11.9)
			thruster(bottomyawcw2, v(-1,0,-1), 1)
			local bottomyawcw3 = v(11.9, -2.8, 11.8)
			thruster(bottomyawcw3, v(1,0,-1), 1)
			local bottomyawcw4 = v(-11.8, -2.8, 11.9)
			thruster(bottomyawcw4, v(1,0,1), 1)

			-- RCS TOP YAW CCW
			local topyawccw1 = v(-11.8, 3.1, -11.9)
			thruster(topyawccw1, v(1,0,-1), 1)
			local topyawccw2 = v(11.9, 3.1, -11.8)
			thruster(topyawccw2, v(1,0,1), 1)
			local topyawccw3 = v(11.8, 3.1, 11.9)
			thruster(topyawccw3, v(-1,0,1), 1)
			local topyawccw4 = v(-11.9, 3.1, 11.8)
			thruster(topyawccw4, v(-1,0,-1), 1)

			-- RCS BOTTOM YAW CCW
			local bottomyawccw1 = v(-11.8, -2.8, -11.9)
			thruster(bottomyawccw1, v(1,0,-1), 1)
			local bottomyawccw2 = v(11.9, -2.8, -11.8)
			thruster(bottomyawccw2, v(1,0,1), 1)
			local bottomyawccw3 = v(11.8, -2.8, 11.9)
			thruster(bottomyawccw3, v(-1,0,1), 1)
			local bottomyawccw4 = v(-11.9, -2.8, 11.8)
			thruster(bottomyawccw4, v(-1,0,-1), 1)

			-- RCS TOP UP (facing) THRUSTERS
			local topup1 = v(-11.8, 3.1, -11.9)
			thruster(topup1, v(0.25,1,0.25), 1)
			local topup2 = v(11.8, 3.1, -11.9)
			thruster(topup2, v(-0.25,1,0.25), 1)
			local topup3 = v(11.8, 3.1, 11.9)
			thruster(topup3, v(-0.25,1,-0.25), 1)
			local topup4 = v(-11.8, 3.1, 11.9)
			thruster(topup4, v(0.25,1,-0.25), 1)

			-- RCS TOP DOWN (facing) THRUSTERS
			local topdown1 = v(-11.8, 3.1, -11.9)
			thruster(topdown1, v(-1,-0.9,-1), 1)
			local topdown2 = v(11.8, 3.1, -11.9)
			thruster(topdown2, v(1,-1,-1), 1)
			local topdown3 = v(11.8, 3.1, 11.9)
			thruster(topdown3, v(1,-1,1), 1)
			local topdown4 = v(-11.8, 3.1, 11.9)
			thruster(topdown4, v(-1,-1,1), 1)

			-- RCS BOTTOM UP (facing) THRUSTERS
			local bottomup1 = v(-11.8, -2.8, -11.9)
			thruster(bottomup1, v(-1,1,-1), 1)
			local bottomup2 = v(11.8, -2.8, -11.9)
			thruster(bottomup2, v(1,1,-1), 1)
			local bottomup3 = v(11.8, -2.8, 11.9)
			thruster(bottomup3, v(1,0.9,1), 1)
			local bottomup4 = v(-11.8, -2.8, 11.9)
			thruster(bottomup4, v(-1,1,1), 1)

			-- RCS BOTTOM DOWN (facing) THRUSTERS
			local bottomdown1 = v(-11.8, -2.8, -11.9)
			thruster(bottomdown1, v(0.25,-1,0.25), 1)
			local bottomdown2 = v(11.8, -2.8, -11.9)
			thruster(bottomdown2, v(-0.25,-1,0.25), 1)
			local bottomdown3 = v(11.8, -2.8, 11.9)
			thruster(bottomdown3, v(-0.25,-1,-0.25), 1)
			local bottomdown4 = v(-11.8, -2.8, 11.9)
			thruster(bottomdown4, v(0.25,-1,-0.25), 1)
		end
	end
})
