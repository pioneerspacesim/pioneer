-- Hammer class corvette
-- Armoured atm capable warship for strike and hit and run operations, can endure a pounding while offering high foreward offensive fire potential
-- ++ Heavy top and bottom armor, ++ Good fore acceleration for a large spaceship, + Useful lateral g ability, ++ Large lateral missile bays, +++ Large forward weapon bay
-- Some long range passive scan ability, Medium active scan ability,
-- - Exposed rear, -- single forward weapons bay, --slow maneuvering versus nimble targets (maybe needs tweaking up to stop being annoying but its a big ship and cant make everything uber :)
-- 160m long and about 200m wide
-- made in wings3d (z front, y up, x left) textured in gimp (textures flipped vertically in game), exported to blender via 3ds then to game via obj due to wings 3d obj texture coords export mess up,
-- 10.6k quads, 21.5k tris for hi res model
-- sliding gear wells and larger gear

define_model('hammergear', {
	info = {
		lod_pixels = { 5, 10, 30, 0 },
		bounding_radius = 8,
		materials = {'leg', 'tyre'}
	},
	static = function(lod)
		set_material('leg', .02,.02,.02,1, 0,0,0, 1, 0,0,0)
		set_material('tyre', .01,.01,.01,1, 0,0,0, 1, 0,0,0)

		local v6 = v(0,0,0)
		local v7 = v(0,3,0)
		local v8 = v(0,5,0)
		-- crossbar
		local v13 = v(0, 5, 1.5)
		local v14 = v(0, 5, -2.0)
		local divs = 4*lod

		use_material('leg')
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		cylinder(4, v13, v14, v(1,0,0), .5)

		use_material('tyre')
		xref_cylinder(divs, v(.5, 5, 1.7), v(1, 5, 1.7), v(0,0,1), 0.9)
		xref_cylinder(divs, v(.5, 5, -0.3), v(1, 5, -0.3), v(0,0,1), 0.9)
		xref_cylinder(divs, v(.5, 5, -2.5), v(1, 5, -2.5), v(0,0,1), 0.9)
	end
})

define_model('hammerleftgear', {
	info = {
		scale = 1,
		lod_pixels = { 10, 100, 200, 0 },
		bounding_radius = 131,
		materials={'hammer'}
	},
	static = function(lod)
		if lod > 1 then
			set_material('hammer', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
			use_material('hammer')
			texture('hammer.png')
			load_obj('hammerleftgear.obj')
			texture(nil)
		end
	end
})

define_model('hammerrightgear', {
	info = {
		scale = 1,
		lod_pixels = { 10, 100, 200, 0 },
		bounding_radius = 131,
		materials={'hammer'}
	},
	static = function(lod)
		if lod > 1 then
			set_material('hammer', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
			use_material('hammer')
			texture('hammer.png')
			load_obj('hammerrightgear.obj')
			texture(nil)
		end
	end
})

define_model('hammer', {
	info = {
		scale = 1,
		lod_pixels={ 10, 100, 200, 0},
		bounding_radius = 131,
		materials = {'hammer', 'distant', 'text', 'glow'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Hammer',
				forward_thrust = -15e7,
				reverse_thrust = 3e7,
				up_thrust = 3e7,
				down_thrust = -3e7,
				left_thrust = -3e7,
				right_thrust = 3e7,
				angular_thrust = 2e9,
				gun_mounts =
				{
					{ v(0,-2.6,-76.6), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 950,
				max_laser = 1,
				max_missile = 64,
				capacity = 950,
				hull_mass = 800,
				price = 7e6,
				hyperdrive_class = 9,
			}
		}
	},
	static = function(lod)
		set_material('distant',0.15,0.15,0.15,1,1,1,1,0,0.15,0.15,0.15)
		set_material('hammer', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
		set_material('text', .6,.6,.6,1,.3,.3,.3,5)
		if lod == 1 then
			use_material('distant')
			load_obj('hammerlod1.obj')
		else
			use_material('hammer')
			texture('hammer.png')
			if lod == 2 then
				load_obj('hammerlod2.obj')
			else
				load_obj('hammer.obj')
				use_material('glow')
				texture('glow.png')
				load_obj('hammerglow.obj')
			end
			texture(nil)

			--lights
			-- xwings
			call_model('posl_red', v(-93.919,-7.338, 70.732), v(0,1,0), v(-1,0,0), 2)
			call_model('posl_red', v(-93.919, 6.629, 70.785), v(0,1,0), v(-1,0,0), 2)
			call_model('posl_white', v(-93.919,-7.338, 76.732), v(0,1,0), v(-1,0,0), 2)
			call_model('posl_white', v(-93.919, 6.629, 76.785), v(0,1,0), v(-1,0,0), 2)

			call_model('posl_green', v(93.919,-7.763, 70.732), v(0,1,0), v(1,0,0), 2)
			call_model('posl_green', v(93.919, 6.166, 70.785), v(0,1,0), v(1,0,0), 2)
			call_model('posl_white', v(93.919,-7.763, 76.732), v(0,1,0), v(1,0,0), 2)
			call_model('posl_white', v(93.919, 6.166, 76.785), v(0,1,0), v(1,0,0), 2)

			-- front probes
			call_model('headlight', v(1.308, -4.910, -79.570), v(0,1,0), v(0,0,-1), 2)
			call_model('headlight', v(2.561, -4.910, -79.570), v(0,1,0), v(0,0,-1), 2)
			call_model('headlight', v(-2.375, -4.910, -79.570), v(0,1,0), v(0,0,-1), 2)
			call_model('headlight', v(-1.172, -4.910, -79.573), v(0,1,0), v(0,0,-1), 2)

			-- side doors
			call_model('posl_red', v(-20.7, -3.8, -28.4), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_white', v(-25.4, -3.8, 9.0), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_red', v(-29.2, -3.8, 69.8), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_green', v(20.7, -3.8, -28.4), v(0,1,0), v(1,0,0), 1)
			call_model('posl_white', v(25.4, -3.8, 9.0), v(0,1,0), v(1,0,0), 1)
			call_model('posl_green', v(29.2, -3.8, 69.8), v(0,1,0), v(1,0,0), 1)

			-- landing lights
			call_model('coll_warn', v(-18.268,-11.000, 39.767), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(-18.415,-11.000, 37.595), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(-18.545,-10.600, 3.282), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(-18.461,-10.600, 0.855), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(-18.592,-9.820,-30.367), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(-18.535,-9.760,-33.106), v(-1,0,0), v(0,-1,0), 2)

			call_model('coll_warn', v(18.482,-11.000, 39.783), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(18.435,-11.000, 37.421), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(18.330,-10.600, 3.031), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(18.310,-10.600, 0.773), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(18.269,-9.820,-30.798), v(-1,0,0), v(0,-1,0), 2)
			call_model('coll_warn', v(18.183,-9.760,-33.106), v(-1,0,0), v(0,-1,0), 2)
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			-- 4 back facing thrusters
			local back1 = v(-26.4, 2.5, 55.1)
			local back2 = v(-8.8, 2.8, 55.1)
			local back3 = v(8.8, 2.5, 55.1)
			local back4 = v(26.4, 2.8, 55.1)
			thruster(back1, v(0,0,1), 50, true)
			thruster(back2, v(0,0,1), 50, true)
			thruster(back3, v(0,0,1), 50, true)
			thruster(back4, v(0,0,1), 50, true)

			-- 4 front facing thrusters
			local front1 = v(9.3, -2.9, -72.2)
			local front2 = v(6.2, -2.6, -75.8)
			xref_thruster(front1, v(0,0,-1), 10, true)
			xref_thruster(front2, v(0,0,-1), 10, true)

			-- 8 down facing thrusters
			local downl1 = v(9.2, -9.9, -56.0)
			xref_thruster(downl1, v(0,-1,0), 10)
			local downl2 = v(9.8, -10.0, -43.1)
			xref_thruster(downl2, v(0,-1,0), 10)

			local downl3 = v(12.7, -10.0, 19.7)
			xref_thruster(downl3, v(0,-1,0), 10)
			local downl4 = v(13.1, -10.0, 27.9)
			xref_thruster(downl4, v(0,-1,0), 10)

			-- 8 up facing thrusters
			local upl1 = v(-6.3, 3.3, -56.1)
			xref_thruster(upl1, v(0,1,0), 10)
			local upl2 = v(-7.0, 3.4, -43.1)
			xref_thruster(upl2, v(0,1,0), 10)

			local upl3 = v(-10.7, 3.4, 19.7)
			xref_thruster(upl3, v(0,1,0), 10)
			local upl4 = v(-11.1, 3.4, 27.8)
			xref_thruster(upl4, v(0,1,0), 10)

			--  8 side facing thrusters
			local sidel1 = v(-15.0, -3.8, -56.1)
			local sidel2 = v(-16.8, -3.8, -43.1)
			local sidel3 = v(-24.7, -3.8, 19.7)
			local sidel4 = v(-25.7, -3.8, 27.8)
			thruster(sidel1, v(-1,0,0), 10)
			thruster(sidel2, v(-1,0,0), 10)
			thruster(sidel3, v(-1,0,0), 10)
			thruster(sidel4, v(-1,0,0), 10)

			local sider1 = v(15.0, -3.8, -56.1)
			local sider2 = v(16.8, -3.8, -43.1)
			local sider3 = v(24.7, -3.8, 19.7)
			local sider4 = v(25.7, -3.8, 27.8)
			thruster(sider1, v(1,0,0), 10)
			thruster(sider2, v(1,0,0), 10)
			thruster(sider3, v(1,0,0), 10)
			thruster(sider4, v(1,0,0), 10)

			if lod > 2 then
				-- glowing parts thanks to s2odan
				set_material('glow',lerp_materials(get_time('SECONDS'),{0,0,0,.8,0,0,0,0,10,10,10},{0,0,0,.4,0,0,0,0,9,9,9}))

				-- text on ship L first
				-- first number id, then center, then normal, then vector direction
				set_material('text', .6,.6,.6,1,.3,.3,.3,5)
				use_material('text')
				text(get_label(), v(-22.5,-10.0,23.8), v(-0.3,-1,0), v(-0.1,0,1), 3, {center = true})
				text(get_label(), v(22.5,-10.0,23.8), v(0.3,-1,0), v(-0.1,0,-1.1), 3, {center = true})

				-- missiles
				-- missile bays LeftTop/LeftBottom and RightTop/RightBottom
				-- 64 missile launch rails from textures
				local LT1 = v(-20.1, 3.0, 18.5)
				local LT2 = v(-20.1, 3.0, 19.2)
				local LT3 = v(-20.1, 3.0, 19.9)
				local LT4 = v(-20.1, 3.0, 20.6)
				local LT5 = v(-20.1, 2.7, 21.3)
				local LT6 = v(-20.2, 2.7, 22.0)
				local LT7 = v(-20.3, 2.7, 22.7)
				local LT8 = v(-20.4, 2.7, 23.4)
				--local LT9 = v(-20.5, 2.7, 24.1)
				--local LT10 = v(-20.6, 2.7, 24.8)
				--local LT11 = v(-20.7, 2.7, 25.5)
				--local LT12 = v(-20.8, 2.7, 26.2)
				--local LT13 = v(-20.9, 2.7, 26.9)
				--local LT14 = v(-21.1, 2.7, 27.6)
				--local LT15 = v(-21.2, 2.7, 28.3)
				--local LT16 = v(-21.3, 2.7, 29.0)

				local LB1 = v(-24.0, 1.0, 18.5)
				local LB2 = v(-24.0, 1.0, 19.2)
				local LB3 = v(-24.0, 1.0, 19.9)
				local LB4 = v(-24.0, 1.0, 20.6)
				local LB5 = v(-24.0, 0.8, 21.3)
				local LB6 = v(-24.1, 0.8, 22.0)
				local LB7 = v(-24.2, 0.8, 22.7)
				local LB8 = v(-24.3, 0.8, 23.4)
				--local LB9 = v(-24.4, 0.8, 24.1)
				--local LB10 = v(-24.5, 0.8, 24.8)
				--local LB11 = v(-24.6, 0.8, 25.5)
				--local LB12 = v(-24.7, 0.8, 26.2)
				--local LB13 = v(-24.8, 0.8, 26.9)
				--local LB14 = v(-24.9, 0.8, 27.6)
				--local LB15 = v(-25.1, 0.8, 28.3)
				--local LB16 = v(-25.2, 0.8, 29.0)

				local RT1 = v(20.1, 3.0, 18.5)
				local RT2 = v(20.1, 3.0, 19.2)
				local RT3 = v(20.1, 3.0, 19.9)
				local RT4 = v(20.1, 3.0, 20.6)
				local RT5 = v(20.1, 2.7, 21.3)
				local RT6 = v(20.2, 2.7, 22.0)
				local RT7 = v(20.3, 2.7, 22.7)
				local RT8 = v(20.4, 2.7, 23.4)
				--local RT9 = v(20.5, 2.7, 24.1)
				--local RT10 = v(20.6, 2.7, 24.8)
				--local RT11 = v(20.7, 2.7, 25.5)
				--local RT12 = v(20.8, 2.7, 26.2)
				--local RT13 = v(20.9, 2.7, 26.9)
				--local RT14 = v(21.1, 2.7, 27.6)
				--local RT15 = v(21.2, 2.7, 28.3)
				--local RT16 = v(21.3, 2.7, 29.0)

				local RB1 = v(24.0, 1.0, 18.5)
				local RB2 = v(24.0, 1.0, 19.2)
				local RB3 = v(24.0, 1.0, 19.9)
				local RB4 = v(24.0, 1.0, 20.6)
				local RB5 = v(24.0, 0.8, 21.3)
				local RB6 = v(24.1, 0.8, 22.0)
				local RB7 = v(24.2, 0.8, 22.7)
				local RB8 = v(24.3, 0.8, 23.4)
				--local RB9 = v(24.4, 0.8, 24.1)
				--local RB10 = v(24.5, 0.8, 24.8)
				--local RB11 = v(24.6, 0.8, 25.5)
				--local RB12 = v(24.7, 0.8, 26.2)
				--local RB13 = v(24.8, 0.8, 26.9)
				--local RB14 = v(24.9, 0.8, 27.6)
				--local RB15 = v(25.1, 0.8, 28.3)
				--local RB16 = v(25.2, 0.8, 29.0)

				-- unguided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',LT1,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',RT1,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',LT2,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',RT2,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 5) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',LT3,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 6) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',RT3,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 7) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',LT4,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 8) == 'MISSILE_UNGUIDED' then
					call_model('d_unguided',RT4,v(0,0,1),v(0,1,0),1)
				end

				-- guided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('d_guided',LT1,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
					call_model('d_guided',RT1,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
					call_model('d_guided',LT2,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
					call_model('d_guided',RT2,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 5) == 'MISSILE_GUIDED' then
					call_model('d_guided',LT3,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 6) == 'MISSILE_GUIDED' then
					call_model('d_guided',RT3,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 7) == 'MISSILE_GUIDED' then
					call_model('d_guided',LT4,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 8) == 'MISSILE_GUIDED' then
					call_model('d_guided',RT4,v(0,0,1),v(0,1,0),1)
				end

				-- smart missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
					call_model('d_smart',LT1,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
					call_model('d_smart',RT1,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
					call_model('d_smart',LT2,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
					call_model('d_smart',RT2,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 5) == 'MISSILE_SMART' then
					call_model('d_smart',LT3,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 6) == 'MISSILE_SMART' then
					call_model('d_smart',RT3,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 7) == 'MISSILE_SMART' then
					call_model('d_smart',LT4,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 8) == 'MISSILE_SMART' then
					call_model('d_smart',RT4,v(0,0,1),v(0,1,0),1)
				end
				-- naval missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
					call_model('d_naval',LT1,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
					call_model('d_naval',RT1,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
					call_model('d_naval',LT2,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
					call_model('d_naval',RT2,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 5) == 'MISSILE_NAVAL' then
					call_model('d_naval',LT3,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 6) == 'MISSILE_NAVAL' then
					call_model('d_naval',RT3,v(0,0,1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 7) == 'MISSILE_NAVAL' then
					call_model('d_naval',LT4,v(0,0,-1),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 8) == 'MISSILE_NAVAL' then
					call_model('d_naval',RT4,v(0,0,1),v(0,1,0),1)
				end
			end
		end

		-- landing gear & navlights
		if get_animation_position('WHEEL_STATE') == 0 then
			call_model('hammerleftgear', v(0,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('hammerrightgear', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		else
			-- sliding front well covers and front gear
			-- sliders (seasons 1-2) rules!
			local sliders1 = 4.2 * get_animation_position('WHEEL_STATE')
			local sliders2 = 1 * math.pi*math.clamp(1.5*(get_animation_position('WHEEL_STATE')-0.4), 0, 1)

			call_model('hammerleftgear', v(sliders1,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('hammerrightgear', v(-sliders1,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('hammergear', v(-11.6,-7.5+(1.2*math.cos(sliders2)),-31.1), v(-1,0,0), v(0,-1,0), 0.8)
			call_model('hammergear', v(11.6,-7.5+(1.2*math.cos(sliders2)),-31.1), v(-1,0,0), v(0,-1,0), 0.8)
			call_model('hammergear', v(-13.3,-7.5+(1.2*math.cos(sliders2)),0.6), v(-1,0,0), v(0,-1,0), 0.8)
			call_model('hammergear', v(13.3,-7.5+(1.2*math.cos(sliders2)),0.6), v(-1,0,0), v(0,-1,0), 0.8)
			call_model('hammergear', v(-13.6,-7.5+(1.2*math.cos(sliders2)),40.5), v(-1,0,0), v(0,-1,0), 0.8)
			call_model('hammergear', v(13.6,-7.5+(1.2*math.cos(sliders2)),40.5), v(-1,0,0), v(0,-1,0), 0.8)
		end
	end
})
