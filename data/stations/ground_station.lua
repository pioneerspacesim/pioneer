-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_surface_station {
	model = 'ground_station',
    num_docking_ports = 14,
	-- define groups of bays, in this case 1 group with 1 bay.
	-- params are = {minSize, maxSize, {list,of,bay,numbers}}
	bay_groups = {
		{20, 500, {1}},
		{20, 500, {2}},
		{20, 500, {3}},
		{20, 500, {4}},
		{20, 500, {5}},
		{20, 500, {6}},
		{0, 20, {7}},
		{0, 20, {8}},
		{0, 20, {9}},
		{0, 20, {10}},
		{0, 20, {11}},
		{0, 20, {12}},
		{0, 20, {13}},
		{0, 20, {14}},
	},
    parking_distance = 5000.0,
    parking_gap_size = 2000.0,
    ship_launch_stage = 0,
    dock_anim_stage_duration = { 300, 4.0},
    undock_anim_stage_duration = { 0 },
}
