define_orbital_station {
	model = 'hoop_spacestation',
	angular_velocity = 0.08,
	num_docking_ports = 1,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 8,				-- lower than animation stage count
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0 },
	undock_anim_stage_duration = { 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 20.0 },
	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		-- docking
		if stage == 2 then
			return { vlerp(t, from, v(0,250,0)), v(1,0,0), v(0,0,-1) }
		elseif stage == 3 then
			return { from, v(1,0,0), v(0,0,-1) }
		elseif stage == 4 then
			return { vlerp(t, from, v(0,0,0)), v(1,0,0), v(0,0,-1) }
		elseif stage == 5 then
			return { vlerp(t, from, v(0,0,0)), v(-1,0,0), v(0,0,-1) }
		elseif stage == 6 or stage == 7 then
			return { v(0,0,0), v(-1,0,0), v(0,0,-1) }
		elseif stage == 8 then
			return { vlerp(t, from, v(0,200,0)), v(-1,0,0), v(0,0,-1) }
		elseif stage == 9 then
			return { v(0,200,0), v(-1,0,0), v(0,0,-1) }
		end
		-- undocking
		if stage == -1 then
			return { v(0,200,0), v(-1,0,0), v(0,0,-1) }
		elseif stage == -2 then
			return { vlerp(t, from, v(0,0,0)), v(-1,0,0), v(0,0,-1) }
		elseif stage == -3 or stage == -4 or stage == -5 then
			return { v(0,0,0), v(-1,0,0), v(0,0,-1) }
		elseif stage == -6 then
			return { vlerp(t, from, v(0,250,0)), v(-1,0,0), v(0,0,-1) }
		elseif stage == -7 or stage == -8 then
			return { v(0,250,0), v(-1,0,0), v(0,0,-1) }
		end
		-- note stage -9 returns nil. this means 'launch ship but continue space station
		-- animations'
	end,
	ship_approach_waypoints = function(port, stage)
		if stage == 1 then
			return { v(0,5000,0), v(1,0,0), v(0,0,1) }
		elseif stage == 2 then
			return { v(0,300,0), v(1,0,0), v(0,0,1) }
		end
	end,
}
