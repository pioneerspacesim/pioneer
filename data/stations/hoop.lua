
define_orbital_station {
	model = 'hoop_spacestation',
	angular_velocity = 0.08,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 3,
	-- define groups of bays, in this case 1 group with 1 bay.
	-- params are = {minSize, maxSize, {list,of,bay,numbers}}
	bay_groups = {
		{0, 500, {1,2,3,4}},
	},
	dock_anim_stage_duration = { 300, 2.0, 5.0, 5.0 },
	undock_anim_stage_duration = { 5.0, 5.0, 2.0 },
}
