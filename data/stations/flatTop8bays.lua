function port_stage_loc(port, stage, from)
	if port == 3 then
		if stage == 2 or stage == -3 then
			v(-10.3072,610.5736,195.1540)
		elseif stage == 3 or stage == -2 then
			v(-10.3072,110.0000,195.1540)
		elseif stage == 4 or stage == -1 then
			v(29.6928,110.0000,195.1540)
		end
	elseif port == 1 then
		if stage == 2 or stage == -3 then
			v(-10.3071,610.5736,795.1541)
		elseif stage == 3 or stage == -2 then
			v(-10.3071,110.0000,795.1541)
		elseif stage == 4 or stage == -1 then
			v(29.6929,110.0000,795.1541)
		end
	elseif port == 5 then
		if stage == 2 or stage == -3 then
			v(-10.3072,610.5736,-204.8460)
		elseif stage == 3 or stage == -2 then
			v(-10.3072,110.0000,-204.8460)
		elseif stage == 4 or stage == -1 then
			v(29.6928,110.0000,-204.8460)
		end
	elseif port == 7 then
		if stage == 2 or stage == -3 then
			v(-10.3071,610.5736,-804.8459)
		elseif stage == 3 or stage == -2 then
			v(-10.3071,110.0000,-804.8459)
		elseif stage == 4 or stage == -1 then
			v(29.6929,110.0000,-804.8459)
		end
	elseif port == 2 then
		if stage == 2 or stage == -3 then
			v(-10.3071,-610.0940,795.1541)
		elseif stage == 3 or stage == -2 then
			v(-10.3071,-109.5204,795.1541)
		elseif stage == 4 or stage == -1 then
			v(29.6929,-109.5204,795.1541)
		end
	elseif port == 6 then
		if stage == 2 or stage == -3 then
			v(-10.3072,-610.0940,-204.8460)
		elseif stage == 3 or stage == -2 then
			v(-10.3072,-109.5204,-204.8460)
		elseif stage == 4 or stage == -1 then
			v(29.6928,-109.5204,-204.8460)
		end
	elseif port == 8 then
		if stage == 2 or stage == -3 then
			v(-10.3071,-610.0940,-804.8459)
		elseif stage == 3 or stage == -2 then
			v(-10.3071,-109.5204,-804.8459)
		elseif stage == 4 or stage == -1 then
			v(29.6929,-109.5204,-804.8459)
		end
	elseif port == 4 then
		if stage == 2 or stage == -3 then
			v(-10.3072,-610.0940,195.1540)
		elseif stage == 3 or stage == -2 then
			v(-10.3072,-109.5204,195.1540)
		elseif stage == 4 or stage == -1 then
			v(29.6928,-109.5204,195.1540)
		end
	end
	
	return from
end

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
	ship_dock_anim = function(port, stage, t, from, ship_aabb)
		local baypos = { 
			v(-150,-350,0), 
			v(-100,-350,100),
			v(100,-350,100), 
			v(150,-350,0) 
		}
		if stage == 2 then
			return { vlerp(t, from, port_stage_loc(port,stage,from)), v(1,0,0), v(0,0,1) }
		elseif stage == 3 then
			return { port_stage_loc(port,stage,from), v(1,0,0), v(0,1,0) }
		elseif stage == 4 then
			return { vlerp(t, port_stage_loc(port,stage-1,from), port_stage_loc(port,stage,from)), v(1,0,0), v(0,1,0) }
		elseif stage == -1 then
			return { vlerp(t, port_stage_loc(port,stage,from), port_stage_loc(port,stage+1,from)), v(1,0,0), v(0,1,0) }
		elseif stage == -2 then
			return { port_stage_loc(port,stage+1,from), v(-1,0,0), v(0,0,-1) }
		elseif stage == -3 then
			return { port_stage_loc(port,stage,from), v(-1,0,0), v(0,0,-1) }
		end
	end,
	ship_approach_waypoints = function(port, stage)
		if port == 3 then
			if stage == 2 then
				return { v(-10.3072,635.4337,195.1540), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3072,2000.0000,195.1540), v(1,0,0), v(0,0,1) }
			end
		elseif port == 1 then
			if stage == 2 then
				return { v(-10.3071,635.4337,795.1541), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3071,2000.0000,795.1541), v(1,0,0), v(0,0,1) }
			end
		elseif port == 5 then
			if stage == 2 then
				return { v(-10.3072,635.4337,-204.8460), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3072,2000.0000,-204.8460), v(1,0,0), v(0,0,1) }
			end
		elseif port == 7 then
			if stage == 2 then
				return { v(-10.3071,635.4337,-804.8459), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3071,2000.0000,-804.8459), v(1,0,0), v(0,0,1) }
			end
		elseif port == 2 then
			if stage == 2 then
				return { v(-10.3071,-634.9541,795.1541), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3071,-1999.5204,795.1541), v(1,0,0), v(0,0,1) }
			end
		elseif port == 6 then
			if stage == 2 then
				return { v(-10.3072,-634.9541,-204.8460), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3072,-1999.5204,-204.8460), v(1,0,0), v(0,0,1) }
			end
		elseif port == 8 then
			if stage == 2 then
				return { v(-10.3071,-634.9541,-804.8459), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3071,-1999.5204,-804.8459), v(1,0,0), v(0,0,1) }
			end
		elseif port == 4 then
			if stage == 2 then
				return { v(-10.3072,-634.9541,195.1540), v(1,0,0), v(0,0,1) }
			elseif stage == 1 then
				return { v(-10.3072,-1999.5204,195.1540), v(1,0,0), v(0,0,1) }
			end
		end
	end,
}
