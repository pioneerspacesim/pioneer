define_surface_station {
	model = 'mushroom_station_2',
	num_docking_ports = 2,
	parking_distance = 5000.0,
	parking_gap_size = 2000.0,
	ship_launch_stage = 2,
	-- 1 - permission granted
	-- 2 - position docked ship
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 2, 4, 4 },
	undock_anim_stage_duration = { 4, 4 },
	-- this stuff doesn't work right with the new docking
	-- code
	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		local port_pos = { v(-100,100,0), v(100,100,0) }
		if stage == 2 then
			return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min.y,0)), v(1,0,0), v(0,1,0) }
		elseif stage == 3 then
			return { vlerp(t, from, port_pos[port] + v(0,-75,0) - v(0,ship_aabb.min.y,0)), v(1,0,0), v(0,1,0) }
		elseif stage == 4 or stage == -1 then
			return { port_pos[port] + v(0,-75,0) - v(0,ship_aabb.min.y,0), v(1,0,0), v(0,1,0) }
		elseif stage == -2 then
			return { vlerp(t, from, port_pos[port] + v(0,1,0) - v(0,ship_aabb.min.y,0)), v(1,0,0), v(0,1,0) }
		end
	end,
	ship_approach_waypoints = function(port, stage)
		local port_pos = { v(-100,100,0), v(100,100,0) }
		if stage == 1 then
			return { v(port_pos[port].x, port_pos[port].y+5000, port_pos[port].z), v(1,0,0), v(0,1,0) }
		elseif stage == 2 then
			return { v(port_pos[port].x, port_pos[port].y, port_pos[port].z), v(1,0,0), v(0,1,0) }
		end
	end,
}
