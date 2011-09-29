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

function createLandingPad(padNum, position)
	-- padNum: The landing pad number (zero based)
	-- position: vector of the landing pad v(0,0,0) is where the ship lands

	local stage = get_arg(ARG_STATION_BAY1_STAGE + padNum) -- used to determine landing lights

	-- draw landing pad
	set_material('pad', .7, .7, .7, 1)
	use_material('pad')
	geomflag(0x10 + padNum)
	cylinder(10, position + v(0,0,0), position + v(0,-10,0), v(0,0,1), 50)
	geomflag(0)

	-- draw out the pad number
	set_material('text', 1, 1, 1, 1)
	use_material('text')
	zbias(1, position + v(0,0,0), v(0,1,0))
	text(padNum+1, position + v(0,0,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
	zbias(0)

	-- draw the pad body
	set_material('body', .3, .3, .3, 1)
	use_material('body')
	tapered_cylinder(10, position + v(0,-10,0), position + v(0,-100,0), v(0,0,1), 10, 20)

	if (math.fmod(get_arg(1), 2) > 1) then
		local color
		if stage > 1 or stage < 0 then
			color = v(1,0,0) -- red
		elseif stage == 1 then
			color = v(0,1,0) -- green
		else
			color = v(1,0.5,0) -- orange
		end
		billboard('smoke.png', 50, color, { position + v(-30,1,30), position + v(30,1,30), position + v(-30,1,-30), position + v(30,1,-30) })
	end
end

define_model('ground_station_1', {
	info = {
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body'},
		tags = {'surface_station'},
		num_docking_ports = 1,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0},
		undock_anim_stage_duration = { 0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-150,50,0) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-150,50,0) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+1000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		-- Landing pads (these are dynamic due to the landind lights)
		createLandingPad(0, v(-150,50,0))
	end,
})

define_model('ground_station_2', {
	info = {
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body'},
		tags = {'surface_station'},
		num_docking_ports = 2,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0},
		undock_anim_stage_duration = { 0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-150,50,0), v(150,50,0) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-150,50,0), v(150,50,0) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+1000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		-- Landing pads (these are dynamic due to the landind lights)
		createLandingPad(0, v(-150,50,0))
		createLandingPad(1, v(150,50,0))
	end,
})

define_model('ground_station_3', {
	info = {
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body'},
		tags = {'surface_station'},
		num_docking_ports = 3,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0},
		undock_anim_stage_duration = { 0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+1000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		-- Landing pads (these are dynamic due to the landind lights)
		createLandingPad(0, v(-150,50,0))
		createLandingPad(1, v(150,50,0))
		createLandingPad(2, v(0,50,-150))
	end,
})
define_model('ground_station_4', {
	info = {
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body'},
		tags = {'surface_station'},
		num_docking_ports = 4,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0},
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
				return { v(port_pos[port]:x(), port_pos[port]:y()+1000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		-- Landing pads (these are dynamic due to the landind lights)
		createLandingPad(0, v(-150,50,0))
		createLandingPad(1, v(150,50,0))
		createLandingPad(2, v(0,50,-150))
		createLandingPad(3, v(0,50,150))
	end,
})
