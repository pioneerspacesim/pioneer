local paths = {
	{ v(0,250,0), v(0,100,0), v( 200,100,0) },
	{ v(0,250,0), v(0,100,0), v(-200,100,0) },
	{ v(0,250,0), v(0, 25,0),  v( 200, 25,0)  },
	{ v(0,250,0), v(0, 25,0),  v(-200, 25,0)  },
}

local bay_orient = {
	v(0,-1,0),
	v(0, 1,0),
	v(0,-1,0),
	v(0, 1,0),
}

local duration_table = function (v, n, b, a)
	local t = {}
	if b then table.insert(t,b) end
	for i = 1,n do table.insert(t,v) end
	if a then table.insert(t,a) end
	return t
end

define_orbital_station {
	model = 'hoop_spacestation',
	angular_velocity = 0.08,
	num_docking_ports = #paths,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = #paths[1]-1,
	dock_anim_stage_duration = duration_table(2.0, #paths[1], DOCKING_TIMEOUT_SECONDS),
	undock_anim_stage_duration = duration_table(2.0, #paths[1], nil, 20.0),
	dock_one_at_a_time = true,

	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		local docking = stage > 0

		local path = paths[port]
		local orient = docking and bay_orient[port] or v(-1,0,0)
		local up = v(0,0,-1)

		if docking then
			if stage > #path then
				return { path[#path], orient, up }
			end
			return { vlerp(t, from, path[stage]), orient, up }
		end

		if stage == -#path then
			return { path[1], orient, up }
		end

		return { vlerp(t, from, path[#path+stage]), orient, up }
	end,

	ship_approach_waypoints = function(port, stage)
		if stage == 1 then
			return { v(0,5000,0), v(1,0,0), v(0,0,1) }
		elseif stage == 2 then
			return { v(0,300,0), v(1,0,0), v(0,0,1) }
		end
	end,
}
