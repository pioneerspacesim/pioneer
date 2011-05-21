--
-- ground_stations.lua
--

define_model('control_tower', {
	info = {
		lod_pixels = {0},
		bounding_radius = 250.0,
		materials = {'tower_base', 'body', 'top'}
	},
	static = function(lod)
		set_material('tower_base', .2, .2, .5, 1)
		use_material('tower_base')
		tapered_cylinder(8, v(0,-50,0), v(0,25,0), v(0,0,1), 75, 20)
		set_material('body', .5, .5, .5, 1)
		use_material('body')
		cylinder(8, v(0,25, 0), v(0,150,0), v(0,0,1), 10) -- stem
		set_material('top', 1, 1, 1, 1)
		use_material('top')
		tapered_cylinder(8, v(0,150,0), v(0,175,0), v(0,0,1), 10, 20)
		tapered_cylinder(8, v(0,175,0), v(0,200,0), v(0,0,1), 20, 10)
	end,
	dynamic = function(lod)
	local lightphase = math.fmod(get_arg(1)+0.620486, 1)
	billboard('smoke.png', 50, lightphase > .5 and v(1,0,0) or v(0,1,0), { v(0, 201, 0) })
	end
})

define_model('landing_pad', {
	info = {
		lod_pixels = {0},
		bounding_radius = 100.0,
		materials = {'pad', 'body'}
	},
	static = function(lod)
		set_material('pad', .7, .7, .7, 1)
		use_material('pad')
		cylinder(10, v(0,0,0), v(0,-10,0), v(0,0,1), 50)

		set_material('body', .3, .3, .3, 1)
		use_material('body')
		tapered_cylinder(10, v(0,-10,0), v(0,-100,0), v(0,0,1), 10, 20)
	end
})

define_model('ground_station_4x4', {
	info = {
		bounding_radius=300.0,
		materials = {'text'},
		tags = {'surface_station'},
		num_docking_ports = 4,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0 },
		undock_anim_stage_duration = { 0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150), v(0,50,150) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150), v(0,50,150) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+5000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		set_material('text', 1, 1, 1, 1)
	
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		
		-- Landing Bay #1
		geomflag(0x10)
		call_model('landing_pad', v(-150,50,0), v(1,0,0), v(0,1,0), 1.0)
		zbias(1, v(-150,50,0), v(0,1,0))
		use_material('text')
		text("1", v(-150,50,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
		geomflag(0)

		-- Landing Bay #2
		geomflag(0x11)
		call_model('landing_pad', v(150,50,0), v(1,0,0), v(0,1,0), 1.0)
		zbias(1, v(150,50,0), v(0,1,0))
		use_material('text')
		text("2", v(150,50,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
		geomflag(0)
		
		-- Landing Bay #3
		geomflag(0x12)
		call_model('landing_pad', v(0,50,-150), v(1,0,0), v(0,1,0), 1.0)
		zbias(1, v(0,50,-150), v(0,1,0))
		use_material('text')
		text("3", v(0,50,-150), v(0,1,0), v(1,0,0), 20.0, {center=true})
		geomflag(0)

		-- Landing Bay #4
		geomflag(0x13)
		call_model('landing_pad', v(0,50,150), v(1,0,0), v(0,1,0), 1.0)
		zbias(1, v(0,50,150), v(0,1,0))
		use_material('text')
		text("4", v(0,50,150), v(0,1,0), v(1,0,0), 20.0, {center=true})
		geomflag(0)
	end,
})
