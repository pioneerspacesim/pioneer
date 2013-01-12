define_surface_station {
	model = 'ground_station_3',
	num_docking_ports = 3,
	parking_distance = 5000.0,
	parking_gap_size = 2000.0,
	ship_launch_stage = 0,
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 4.0},
	undock_anim_stage_duration = { 0 },
	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150) }
		if stage == 2 then
			return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min.y,0)), v(1,0,0), v(0,1,0) }
		end
	end,
	ship_approach_waypoints = function(port, stage)
		local port_pos = { v(-150,50,0), v(150,50,0), v(0,50,-150) }
		if stage == 1 then
			return { v(port_pos[port].x, port_pos[port].y+5000, port_pos[port].z), v(1,0,0), v(0,1,0) }
		elseif stage == 2 then
			return { v(port_pos[port].x, port_pos[port].y, port_pos[port].z), v(1,0,0), v(0,1,0) }
		end
	end,
}
