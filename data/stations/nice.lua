define_orbital_station {
	model = 'nice_spacestation',
	angular_velocity = 0.15,
	num_docking_ports = 1,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 8,		-- lower than animation stage count
	-- docking:
	-- 1 - permission granted. open door1
	-- 2 - center ship, close door1
	-- 3 - open door2
	-- 4 - enter lift
	-- 5 - close door2 and rotate ship 180
	-- 6 - make lift noise for a while
	-- 7 - open door2
	-- 8 - move ship forward into docking bay
	-- 9 - close door2. dock
	-- undocking:
	-- -1 - open door2
	-- -2 - move backwards into lift
	-- -3 - close door2
	-- -4 - make lift noises ;)
	-- -5 - open door2
	-- -6 - move forward into docking bay
	-- -7 - close door2
	-- -8 - open door1, launch
	-- -9 - close door behind our hero
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0 },
	undock_anim_stage_duration = { 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 4.0, 20.0 },
	-- stage will be 1..n for dock_anim, and -1..-n for undock_anim
	-- t is where we are in the stage. 0.0 .. 1.0
	-- from is the ship position at the end of the previous stage (use for interpolating position)
	-- must return 3 vectors for position & orientation: { position, xaxis, yaxis }
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
