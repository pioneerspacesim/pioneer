define_orbital_station {
	model = 'flatTop8bays',
	angular_velocity = 0.0,
	num_docking_ports = 8,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 3,
	-- for stations where each docking port shares the
	-- same front door, set dock_one_at_a_time = true,
	dock_one_at_a_time = false,
	dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 10.0, 5.0, 5.0 },
	undock_anim_stage_duration = { 5.0, 5.0, 10.0 },
}
