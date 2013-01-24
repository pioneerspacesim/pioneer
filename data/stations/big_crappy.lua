define_orbital_station {
	model = 'big_crappy_spacestation',
	angular_velocity = 0.1,
	num_docking_ports = 4,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 3,
	-- for stations where each docking port shares the
	-- same front door, set dock_one_at_a_time = true,
	dock_one_at_a_time = true,
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 10.0, 5.0, 5.0 },
	undock_anim_stage_duration = { 5.0, 5.0, 10.0 },
	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		local baypos = { v(-150,-350,0), v(-100,-350,100),
			v(100,-350,100), v(150,-350,0) }
		if stage == 2 then
			return { vlerp(t, from, v(0,-350,0)), v(1,0,0), v(0,0,1) }
		elseif stage == 3 then
			return { v(0,-350,0), v(1,0,0), v(0,1,0) }
		elseif stage == 4 then
			return { vlerp(t, v(0,-350,0), baypos[port]), v(1,0,0), v(0,1,0) }
		elseif stage == -1 then
			return { vlerp(t, baypos[port], v(0,-350,0)), v(1,0,0), v(0,1,0) }
		elseif stage == -2 then
			return { v(0,-350,0), v(-1,0,0), v(0,0,-1) }
		elseif stage == -3 then
			return { vlerp(t, v(0,-350,0), v(0,600,0)), v(-1,0,0), v(0,0,-1) }
		end
	end,
	ship_approach_waypoints = function(port, stage)
		if stage == 1 then
			return { v(0,5000,0), v(1,0,0), v(0,0,1) }
		elseif stage == 2 then
			return { v(0,400,0), v(1,0,0), v(0,0,1) }
		end
	end,
}
