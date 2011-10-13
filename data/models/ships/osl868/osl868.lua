-- Olympic Stellar Liner 868 Atmospheric Mixed freighter, designed for broad atmospheric density operations and extended in atmosphere runs at subsonic speeds
-- Limited Space Combat potential partially offset by large strong wings with numerous missile hardpoints
-- Designed by Gareth Allnutt in 2011, please mention my name in a readme (it doesnt have to be prominent) if using the model
-- Designed with wings 3d 1.4.1 and the Gimp 2.6
-- Numerous design ideas from a 767 forward fuselage, c-17 style rear ramp and overhanging wings, easyglider wingtips, walrus planform and various other bits and bobs, mostly civilian and subsonic.

define_model('1nosewheel', {
	info = {
		lod_pixels={5,15,30,0},
		bounding_radius = 7,
		materials={'leg','tyre'}
	},
	static = function(lod)
		set_material('leg', .02,.02,.02,1, 0,0,0, 1, 0,0,0)
		set_material('tyre', .01,.01,.01,1, 0,0,0, 1, 0,0,0)
		use_material('leg')
		local v6 = v(0, 0, 0)
		local v7 = v(0, 3, 0)
		local v8 = v(0, 5, 0)
		local divs = lod*4
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		use_material('tyre')
		xref_cylinder(divs, v(.5,5,0), v(1,5,0), v(0,0,1), 1.0)
	end
})

define_model('frontgear', {
	info = {
		scale = 1,
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 7,
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_arg(ARG_SHIP_WHEEL_STATE)-0.34), 0, 1)
		call_model('1nosewheel', v(0,0,0), v(1,0,0), v(0,math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
	end
})

define_model('3mainwheels', {
	info = {
		lod_pixels = {5,50,0},
		bounding_radius = 8,
		materials = {'leg', 'tyre'}
	},
	static = function(lod)
		local v6 = v(0,0,0)
		local v7 = v(0,3,0)
		local v8 = v(0,5,0)
		-- crossbar
		local v13 = v(0, 5, 1.5)
		local v14 = v(0, 5, -2.0)
		local divs = 4*lod
		set_material('leg', .02,.02,.02,1, 0,0,0, 1, 0,0,0)
		set_material('tyre', .01,.01,.01,1, 0,0,0, 1, 0,0,0)
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

define_model('reargear', {
	info = {
		lod_pixels={.1,80,600,0},
		bounding_radius = 7,
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		local wheel_ang = 0.5*math.pi*math.clamp(1.5*(get_arg(ARG_SHIP_WHEEL_STATE)-0.34), 0, 1)
		call_model('3mainwheels', v(0,0,0), v(1,0,0), v(0,-.3+math.sin(wheel_ang),math.cos(wheel_ang)), 1.0)
	end
})

define_model('osl868', {
	info = {
		scale = 1,
		lod_pixels={3,12,50,0},

		-- 46 metres from centre to wingtip, 767 can take 100t in similar hull but is limited by aerodynamic lift, 200t is available in osl with wing lift+thruster lift but care must then be taken not to exceed v thrust limit on high g planets with large loads
		bounding_radius = 70,
		materials = {'oslo', 'text', 'glow', 'glass', 'distant'},
	},
	static = function(lod)
		-- material specifications; diffuse r,g,b, alpha trans, specular r,g,b, shinyness, environmental r,g,b
		set_material('distant',0.15,0.15,0.15,1,1,1,1,0,0.15,0.15,0.15)
		--set_material('osl', .8,.8,.8,1,20,20,20,30,0.2,0.2,0.2)
		set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
		set_material('glass', 1,1,1,0.2,0.2,0.2,0.2,1,0,0,0)

		if lod == 1 then
			use_material('distant')
			load_obj('osl868lod1.obj')
		else
			use_material('oslo')
			texture('vthrust.png')
			load_obj('osl868_vthrust.obj')
			if lod > 2 then
				texture('weapon.png')
				load_obj('osl868_weapon.obj')
				texture('smalls.png')
				load_obj('osl868_smalls.obj')
				if lod > 3 then
					texture('ports.png')
					load_obj('osl868_ports.obj')
					texture('chairs.png')
					load_obj('osl868_chairs.obj')
					texture('dash.png')
					load_obj('osl868_dash.obj')
					texture('floor.png')
					load_obj('osl868_floor.obj')
					texture('ins.png')
					load_obj('osl868_ins.obj')
					texture('interior.png')
					load_obj('osl868_interior.obj')
					texture('panels.png')
					load_obj('osl868_panels.obj')
					texture('pitinter.png')
					load_obj('osl868_pitinter.obj')
				end
			end
			texture('exb.png')
			load_obj('osl868_exb.obj')
			texture('wells.png')
			load_obj('osl868_wells.obj')

			use_material('glass')
			texture('glass.png')
			load_obj('glass.obj')
			texture(nil)
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('glow',lerp_materials(get_arg(ARG_ALL_TIME_SECONDS),{0,0,0,.8,0,0,0,0,10,10,10},{0,0,0,.4,0,0,0,0,9,9,9}))
			use_material('glow')
			texture('models/ships/osl868/exf.png')
			load_obj('models/ships/osl868/exf.obj')
			texture(nil)
			set_material('text', .6,.6,.6,1,.3,.3,.3,5)
			use_material('text')
			text(get_arg_string(0), v(-15.1,1.8,22.9), v(0,-1,0), v(-1,0,0.3), 1.5, {center = true})
			text(get_arg_string(0), v(15.1,1.8,22.9), v(0,-1,0), v(-1,0,-0.3), 1.5, {center = true})
			--lights
			--call_model('coll_warn', v(0,16.979,41.612), v(0,1,0), v(0,1,0), 1)
			call_model('posl_red', v(-45.908,5.954,30.826), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_red', v(-45.923,5.949,32.711), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_green', v(45.908,5.954,30.826), v(0,1,0), v(1,0,0), 1)
			call_model('posl_green', v(45.923,5.954,32.711), v(0,1,0), v(1,0,0), 1)
			--call_model('coll_warn', v(0,-2.704,15.991), v(0,1,0), v(0,-1,0), 1)
			--call_model('coll_warn', v(0,-2.704,5.592), v(0,1,0), v(0,-1,0), 1)
			--call_model('coll_warn', v(0,-2.704,-22.573), v(0,1,0), v(0,-1,0), 1)
			call_model('posl_white', v(-2.043,-0.57,30.39), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_white', v(2.043,-0.57,30.39), v(0,1,0), v(1,0,0), 1)
			--call_model('posl_white', v(-41.11,1.834,29.583), v(0,1,0), v(0,-1,0), 1)
			--call_model('posl_white', v(41.11,1.834,29.583), v(0,1,0), v(0,-1,0), 1)
			call_model('coll_warn', v(0,-1.443,30.724), v(0,1,0), v(0,-1,1), 1)
			call_model('coll_warn', v(0,1.417,35.436), v(0,1,0), v(0,-1,1), 1)
			texture(nil)
		end
		-- landing gear
		if get_arg(ARG_SHIP_WHEEL_STATE) ~= 0 then
			local F = v(0, -1.65, -24.6)
			local RL = v(-0.925, -0.9, 12)
			local RR = v(0.925, -0.9, 12)

			call_model('frontgear', F, v(1,0,0), v(0,-1,0), 0.35)
			call_model('reargear', RL, v(1,0,0), v(0,-1,0), 0.5)
			call_model('reargear', RR, v(1,0,0), v(0,-1,0), 0.5)
		end

		-- missiles are primary defensive and offensive weapons + small fore weapon bay + jammer
		-- L1 inside > L4 outside
		local L1 = v(-23.1,1.5,22.6)
		local L2 = v(-25.1,1.5,23.5)
		local L3 = v(-27.1,1.5,24.4)
		local L4 = v(-29.1,1.5,25.4)

		local R1 = v(23.1,1.5,22.6)
		local R2 = v(25.1,1.5,23.5)
		local R3 = v(27.1,1.5,24.4)
		local R4 = v(29.1,1.5,25.4)

		-- unguided missiles loading
		if get_arg(ARG_SHIP_EQUIP_MISSILE0) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',L1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',L1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE1) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',R1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',R1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE2) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',L2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',L2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE3) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',R2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',R2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE4) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',L3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',L3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE5) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',R3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',R3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE6) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',L4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',L4,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE7) == Equip.MISSILE_UNGUIDED then
			call_model('m_pod',R4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_unguided',R4,v(1,0,0),v(0,1,0),1)
		end

		-- guided missiles loading
		if get_arg(ARG_SHIP_EQUIP_MISSILE0) == Equip.MISSILE_GUIDED then
			call_model('m_pod',L1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',L1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE1) == Equip.MISSILE_GUIDED then
			call_model('m_pod',R1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',R1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE2) == Equip.MISSILE_GUIDED then
			call_model('m_pod',L2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',L2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE3) == Equip.MISSILE_GUIDED then
			call_model('m_pod',R2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',R2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE4) == Equip.MISSILE_GUIDED then
			call_model('m_pod',L3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',L3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE5) == Equip.MISSILE_GUIDED then
			call_model('m_pod',R3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',R3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE6) == Equip.MISSILE_GUIDED then
			call_model('m_pod',L4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',L4,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE7) == Equip.MISSILE_GUIDED then
			call_model('m_pod',R4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_guided',R4,v(1,0,0),v(0,1,0),1)
		end

		-- smart missiles loading
		if get_arg(ARG_SHIP_EQUIP_MISSILE0) == Equip.MISSILE_SMART then
			call_model('m_pod',L1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE1) == Equip.MISSILE_SMART then
			call_model('m_pod',R1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE2) == Equip.MISSILE_SMART then
			call_model('m_pod',L2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE3) == Equip.MISSILE_SMART then
			call_model('m_pod',R2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE4) == Equip.MISSILE_SMART then
			call_model('m_pod',L3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE5) == Equip.MISSILE_SMART then
			call_model('m_pod',R3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE6) == Equip.MISSILE_SMART then
			call_model('m_pod',L4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L4,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE7) == Equip.MISSILE_SMART then
			call_model('m_pod',R4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R4,v(1,0,0),v(0,1,0),1)
		end

		-- naval missiles loading
		if get_arg(ARG_SHIP_EQUIP_MISSILE0) == Equip.MISSILE_NAVAL then
			call_model('m_pod',L1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_naval',L1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE1) == Equip.MISSILE_NAVAL then
			call_model('m_pod',R1+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_naval',R1,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE2) == Equip.MISSILE_NAVAL then
			call_model('m_pod',L2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_naval',L2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE3) == Equip.MISSILE_NAVAL then
			call_model('m_pod',R2+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_naval',R2,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE4) == Equip.MISSILE_NAVAL then
			call_model('m_pod',L3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE5) == Equip.MISSILE_NAVAL then
			call_model('m_pod',R3+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R3,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE6) == Equip.MISSILE_NAVAL then
			call_model('m_pod',L4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',L4,v(1,0,0),v(0,1,0),1)
		end
		if get_arg(ARG_SHIP_EQUIP_MISSILE7) == Equip.MISSILE_NAVAL then
			call_model('m_pod',R4+v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('d_smart',R4,v(1,0,0),v(0,1,0),1)
		end

		-- Thrusters, x and z reversed from native wings3d model, +y up, -z forward, -x left
		-- Main back thrusters
		local backl1 = v(-4.6, 3.3, 21.8)
		local backl2 = v(-3.7, 3.4, 21.9)
		local backl3 = v(-2.8, 3.4, 22.0)
		xref_thruster(backl1, v(0,0,1), 5, true)
		xref_thruster(backl2, v(0,0,1), 5, true)
		xref_thruster(backl3, v(0,0,1), 5, true)
		local backl = v(-3.4, 0.9, 22.4)
		xref_thruster(backl, v(0,0,1), 10, true)

		--  Main  front thrusters
		local frontl1 = v(4.4, 3.4, 11.2)
		local frontl2 = v(3.5, 3.4, 10.9)
		local frontl3 = v(2.6, 3.4, 10.6)
		xref_thruster(frontl1, v(0,0,-1), 5, true)
		xref_thruster(frontl2, v(0,0,-1), 5, true)
		xref_thruster(frontl3, v(0,0,-1), 5, true)

		-- Main vertical thrusters
		local fvert1 = v(0, -2.2, -20.8)
		local fvert2 = v(0, -2.2, -18.8)
		local fvert3 = v(0,-2.2, -16.8)
		local fvert4 = v(0, -2.2, -14.8)
		thruster(fvert1, v(0,-1,0), 5, true)
		thruster(fvert2, v(0,-1,0), 5, true)
		thruster(fvert3, v(0,-1,0), 5, true)
		thruster(fvert4, v(0,-1,0), 5, true)
		local bvert1 = v(0, -2.2, 19.2)
		local bvert2 = v(0, -2.2, 21.2)
		local bvert3 = v(0, -2.2, 23.2)
		local bvert4 = v(0, -2.2, 25.2)
		thruster(bvert1, v(0,-1,0), 5, true)
		thruster(bvert2, v(0,-1,0), 5, true)
		thruster(bvert3, v(0,-1,0), 5, true)
		thruster(bvert4, v(0,-1,0), 5, true)

		-- Up facing (+y) thrusters front to back
		-- nose
		local fu1 = v(0.3, 0.5, -32.8)
		local fu2 = v(0, 0.6, -32.8)
		xref_thruster(fu1, v(0,1,0), 1)
		thruster(fu2, v(0,1,0), 1)

		-- centre
		local clu1 = v(-2.3, 0.8, -8.3)
		local clu2 = v(-2.3, 0.8, -4.4)
		local clu3 = v(-2.3, 0.8, -0.5)
		xref_thruster(clu1, v(0,1,0), 1)
		xref_thruster(clu2, v(0,1,0), 1)
		xref_thruster(clu3, v(0,1,0), 1)

		-- wing
		local lowu1 = v(-37.1, 2.4, 26.5)
		local lowu2 = v(-37.1, 2.4, 27.8)
		local lowu3 = v(-37.1, 2.3, 29.2)
		xref_thruster(lowu1, v(0,1,0), 1)
		xref_thruster(lowu2, v(0,1,0), 1)
		xref_thruster(lowu3, v(0,1,0), 1)
		local liwu1 = v(-13.1, 2.6, 13.0)
		local liwu2 = v(-13.1, 2.6, 16.3)
		local liwu3 = v(-13.1, 2.5, 19.6)
		xref_thruster(liwu1, v(0,1,0), 1)
		xref_thruster(liwu2, v(0,1,0), 1)
		xref_thruster(liwu3, v(0,1,0), 1)

		-- back
		local blu1 = v(-0.9, 3.1, 29.8)
		local blu2 = v(-1.4, 3.0, 29.8)
		local blu3 = v(-1.9, 2.9, 29.8)
		xref_thruster(blu1, v(0,1,0), 1)
		xref_thruster(blu2, v(0,1,0), 1)
		xref_thruster(blu3, v(0,1,0), 1)

		-- Down facing (-y) thrusters front to back
		-- front
		local fd1 = v(0.3, -0.8, -32.8)
		local fd2 = v(0, -0.9, -32.8)
		xref_thruster(fd1, v(0,-1,0), 1)
		thruster(fd2, v(0,-1,0), 1)

		-- centre
		local cld1 = v(-2.3, -0.8, -8.3)
		local cld2 = v(-2.3, -0.8, -4.4)
		local cld3 = v(-2.3, -0.8, -0.5)
		xref_thruster(cld1, v(0,-1,0), 1)
		xref_thruster(cld2, v(0,-1,0), 1)
		xref_thruster(cld3, v(0,-1,0), 1)

		-- wing
		local lowd1 = v(-37.1, 2.0, 26.7)
		local lowd2 = v(-37.1, 2.1, 28.0)
		local lowd3 = v(-37.1, 2.2, 29.4)
		xref_thruster(lowd1, v(0,-1,0), 1)
		xref_thruster(lowd2, v(0,-1,0), 1)
		xref_thruster(lowd3, v(0,-1,0), 1)
		local liwd1 = v(-13.1, 1.9, 13.2)
		local liwd2 = v(-13.1, 1.9, 16.5)
		local liwd3 = v(-13.1, 2.1, 19.3)
		xref_thruster(liwd1, v(0,-1,0), 1)
		xref_thruster(liwd2, v(0,-1,0), 1)
		xref_thruster(liwd3, v(0,-1,0), 1)

		-- back
		local bld1 = v(-0.5, -1.8, 29.7)
		local bld2 = v(0.0, -1.8, 29.7)
		xref_thruster(bld1, v(0,-1,0), 1)
		thruster(bld2, v(0,-1,0), 1)

		--  Side facing thrusters left (-x) then right (x)
		-- front
		local fl1 = v(-0.6, 0.2, -32.8)
		local fl2 = v(-0.6, 0.0, -32.8)
		local fl3 = v(-0.6, -0.1, -32.8)
		thruster(fl1, v(-1,0,0), 1)
		thruster(fl2, v(-1,0,0), 1)
		thruster(fl3, v(-1,0,0), 1)
		local fr1 = v(0.6, 0.2, -32.8)
		local fr2 = v(0.6, 0.0, -32.8)
		local fr3 = v(0.6, -0.1, -32.8)
		thruster(fr1, v(1,0,0), 1)
		thruster(fr2, v(1,0,0), 1)
		thruster(fr3, v(1,0,0), 1)

		-- centre
		local cl1 = v(-2.3, 0.0, 3.4)
		local cl2 = v(-2.3, 0.0, 7.8)
		local cl3 = v(-2.3, 0.0, 12.5)
		thruster(cl1, v(-1,0,0), 1)
		thruster(cl2, v(-1,0,0), 1)
		thruster(cl3, v(-1,0,0), 1)
		local cr1 = v(2.3, 0.0, 3.4)
		local cr2 = v(2.3, 0.0, 7.8)
		local cr3 = v(2.3, 0.0, 12.5)
		thruster(cr1, v(1,0,0), 1)
		thruster(cr2, v(1,0,0), 1)
		thruster(cr3, v(1,0,0), 1)

		-- back
		local bl1 = v(-2.0, 0.9, 29.8)
		local bl2 = v(-2.0, 0.3, 29.8)
		local bl3 = v(-2.0, -0.3, 29.8)
		thruster(bl1, v(-1,0,0), 1)
		thruster(bl2, v(-1,0,0), 1)
		thruster(bl3, v(-1,0,0), 1)
		local br1 = v(2.0, 0.9, 29.7)
		local br2 = v(2.0, 0.3, 29.7)
		local br3 = v(2.0, -0.3, 29.7)
		thruster(br1, v(1,0,0), 1)
		thruster(br2, v(1,0,0), 1)
		thruster(br3, v(1,0,0), 1)
	end
})

define_model('osl868olympic', {
	info = {
		scale = 1,
		lod_pixels={3,12,50,0},
		bounding_radius = 70,
		materials = {'oslo'},
		tags = {'ship'},
		ship_defs = {
			{
				name='OSL 868 Olympic',
				forward_thrust = -8.5e6,
				reverse_thrust = 4e6,
				up_thrust = 4e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 50e6,
				gun_mounts =
				{
					{ v(0,-2.2,-29.5), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 200,
				max_laser = 2,
				max_missile = 12,
				max_fuelscoop = 0,
				capacity = 200,
				hull_mass = 170,
				price = 300000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			texture('fuse.png')
			load_obj('osl868_fuse.obj')
			texture('engb.png')
			load_obj('osl868_engb.obj')
			texture('engt.png')
			load_obj('osl868_engt.obj')
			texture('wingb.png')
			load_obj('osl868_wingb.obj')
			texture('wingt.png')
			load_obj('osl868_wingt.obj')
			texture('tail.png')
			load_obj('osl868_tail.obj')
			texture('access.png')
			load_obj('osl868_access.obj')
			texture(nil)
		end
		call_model('osl868',v(0,0,0),v(1,0,0),v(0,1,0),1)
	end,
	dynamic = function (lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			local flap_ang = 0.1*math.pi*math.clamp(3*get_arg(ARG_SHIP_WHEEL_STATE),0,1)
			texture('models/ships/osl868/access.png')
			load_obj('models/ships/osl868/flwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/frwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,1)))
			load_obj('models/ships/osl868/rlwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/rrwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,1)))
			texture(nil)
		end
	end
})

define_model('osl868arktisk', {
	info = {
		scale = 1,
		lod_pixels={3,12,50,0},
		bounding_radius = 70,
		materials = {'oslo'},
		tags = {'ship'},
		ship_defs = {
			{
				name='OSL 868 Arktisk',
				forward_thrust = -8.5e6,
				reverse_thrust = 4e6,
				up_thrust = 4e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 50e6,
				gun_mounts =
				{
					{ v(0,-2.2,-29.5), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 200,
				max_laser = 2,
				max_missile = 12,
				max_fuelscoop = 0,
				capacity = 200,
				hull_mass = 170,
				price = 300000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			texture('fusearktisk.png')
			load_obj('osl868_fuse.obj')
			texture('engbarktisk.png')
			load_obj('osl868_engb.obj')
			texture('engtarktisk.png')
			load_obj('osl868_engt.obj')
			texture('wingbarktisk.png')
			load_obj('osl868_wingb.obj')
			texture('wingtarktisk.png')
			load_obj('osl868_wingt.obj')
			texture('tailarktisk.png')
			load_obj('osl868_tail.obj')
			texture('accessarktisk.png')
			load_obj('osl868_access.obj')
			texture(nil)
		end
		call_model('osl868',v(0,0,0),v(1,0,0),v(0,1,0),1)
	end,
	dynamic = function (lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			local flap_ang = 0.1*math.pi*math.clamp(3*get_arg(ARG_SHIP_WHEEL_STATE),0,1)
			texture('models/ships/osl868/accessarktisk.png')
			load_obj('models/ships/osl868/flwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/frwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,1)))
			load_obj('models/ships/osl868/rlwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/rrwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,1)))
			texture(nil)
		end
	end
})

define_model('osl868kosmos', {
	info = {
		scale = 1,
		lod_pixels={3,12,50,0},
		bounding_radius = 70,
		materials = {'oslo'},
		tags = {'ship'},
		ship_defs = {
			{
				name='OSL 868 Kosmos',
				forward_thrust = -8.5e6,
				reverse_thrust = 4e6,
				up_thrust = 4e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 50e6,
				gun_mounts =
				{
					{ v(0,-2.2,-29.5), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 200,
				max_laser = 2,
				max_missile = 12,
				max_fuelscoop = 0,
				capacity = 200,
				hull_mass = 170,
				price = 300000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			texture('fusekosmos.png')
			load_obj('osl868_fuse.obj')
			texture('engbkosmos.png')
			load_obj('osl868_engb.obj')
			texture('engtkosmos.png')
			load_obj('osl868_engt.obj')
			texture('wingbkosmos.png')
			load_obj('osl868_wingb.obj')
			texture('wingtkosmos.png')
			load_obj('osl868_wingt.obj')
			texture('tailkosmos.png')
			load_obj('osl868_tail.obj')
			texture('accesskosmos.png')
			load_obj('osl868_access.obj')
			texture(nil)
		end
		call_model('osl868',v(0,0,0),v(1,0,0),v(0,1,0),1)
	end,
	dynamic = function (lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			local flap_ang = 0.1*math.pi*math.clamp(3*get_arg(ARG_SHIP_WHEEL_STATE),0,1)
			texture('models/ships/osl868/accesskosmos.png')
			load_obj('models/ships/osl868/flwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/frwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,1)))
			load_obj('models/ships/osl868/rlwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/rrwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,1)))
			texture(nil)
		end
	end
})

define_model('osl868redstar', {
	info = {
		scale = 1,
		lod_pixels={3,12,50,0},
		bounding_radius = 70,
		materials = {'oslo'},
		tags = {'ship'},
		ship_defs = {
			{
				name='OSL 868 Red Star',
				forward_thrust = -8.5e6,
				reverse_thrust = 4e6,
				up_thrust = 4e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 50e6,
				gun_mounts =
				{
					{ v(0,-2.2,-29.5), v(0,0,-1) },
					{ v(0,-0.5,0), v(0,0,1) },
				},
				max_cargo = 200,
				max_laser = 2,
				max_missile = 12,
				max_fuelscoop = 0,
				capacity = 200,
				hull_mass = 170,
				price = 300000,
				hyperdrive_class = 3,
			}
		}
	},
	static = function(lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			texture('fuseredstar.png')
			load_obj('osl868_fuse.obj')
			texture('engbredstar.png')
			load_obj('osl868_engb.obj')
			texture('engtredstar.png')
			load_obj('osl868_engt.obj')
			texture('wingbredstar.png')
			load_obj('osl868_wingb.obj')
			texture('wingtredstar.png')
			load_obj('osl868_wingt.obj')
			texture('tailredstar.png')
			load_obj('osl868_tail.obj')
			texture('accessredstar.png')
			load_obj('osl868_access.obj')
			texture(nil)
		end
		call_model('osl868',v(0,0,0),v(1,0,0),v(0,1,0),1)
	end,
	dynamic = function (lod)
		if lod > 1 then
			set_material('oslo', 0.5,0.5,0.5,1,3,3,3,10,0.5,0.5,0.5)
			use_material('oslo')
			local flap_ang = 0.1*math.pi*math.clamp(3*get_arg(ARG_SHIP_WHEEL_STATE),0,1)
			texture('models/ships/osl868/accessredstar.png')
			load_obj('models/ships/osl868/flwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/frwell.obj', Matrix.rotate(0.8*flap_ang, v(0,0,1)))
			load_obj('models/ships/osl868/rlwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,-1)))
			load_obj('models/ships/osl868/rrwell.obj', Matrix.rotate(1.7*flap_ang, v(0,0,1)))
			texture(nil)
		end
	end
})
