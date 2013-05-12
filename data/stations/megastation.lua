define_orbital_station {
	model = 'megastation',
	angular_velocity = 0,
	num_docking_ports = 120,
	parking_distance = 5000.0,
	parking_gap_size = 500.0,
	ship_launch_stage = 4,
	-- define groups of bays, in this case 1 group with 1 bay.
	-- params are = {minSize, maxSize, {list,of,bay,numbers}}
	bay_groups = {
		{0, 500, {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48}},
		{0, 500, {49,50,51,52,53,54,55,56,57,58,59,60}},
		{0, 500, {61,62,63,64,65,66,67,68,69,70,71,72}},
		{0, 500, {73,74,75,76,77,78,79,80,81,82,83,84,85,86,87,88,89,90,91,92,93,94,95,96,97,98,99,100,101,102,103,104,105,106,107,108,109,110,11,112,113,114,115,116,117,118,119,120}},
	},
	dock_anim_stage_duration = { 300, 10.0, 5.0, 5.0, 5.0 },
	undock_anim_stage_duration = { 5.0, 5.0, 5.0, 10.0 },
}
