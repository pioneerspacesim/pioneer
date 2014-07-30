-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_surface_station {
	model = 'new_ground',
    num_docking_ports = 6,
	-- define groups of bays, in this case 1 group with 1 bay.
	-- params are = {minSize, maxSize, {list,of,bay,numbers}}
	bay_groups = {
		{0, 500, {1}},
		{0, 500, {2}},
		{0, 500, {3}},
		{0, 500, {4}},
		{0, 500, {5}},
		{0, 500, {6}},
	},
    parking_distance = 5000.0,
    parking_gap_size = 2000.0,
    ship_launch_stage = 0,
    dock_anim_stage_duration = { 300, 4.0},
    undock_anim_stage_duration = { 0 },
}
