--
-- ground_stations.lua
--
DOCKING_TIMEOUT_SECONDS = 300

define_model('airport_control_tower', {
	info = {
		lod_pixels = {0},
		bounding_radius = 250.0,
		materials = {'tower_base', 'body', 'top'}
	},
	static = function(lod)
		set_material('tower_base', .2, .2, .5, 1)
		use_material('tower_base')
		tapered_cylinder(8, v(0,-50,0), v(0,25,0), v(0,0,1), 75, 20)
		set_material('body', .5, .5, .5, 1)
		use_material('body')
		cylinder(8, v(0,25, 0), v(0,150,0), v(0,0,1), 10) -- stem
		set_material('top', 1, 1, 1, 1)
		use_material('top')
		tapered_cylinder(8, v(0,150,0), v(0,175,0), v(0,0,1), 10, 20)
		tapered_cylinder(8, v(0,175,0), v(0,200,0), v(0,0,1), 20, 10)
	end,
	dynamic = function(lod)
	local lightphase = math.fmod(get_time('SECONDS')+0.620486, 1)
	billboard('smoke.png', 50, lightphase > .5 and v(1,0,0) or v(0,1,0), { v(0, 201, 0) })
	end
})

function createRunway(num, position)
	-- padNum: The landing pad number (zero based)
	-- position: vector of the landing pad v(0,0,0) is where the ship lands

	local padId = 'DOCKING_BAY_' .. (num + 1)
	local stage = get_animation_stage(padId) -- used to determine landing lights

	-- draw runway
	set_material('runway', .7, .7, .7, 1)
	use_material('runway')
	geomflag(0x10 + num)
	quad(position + v(-500,0,30), position + v(500,0,30), position + v(500,0,-30), position + v(-500,0,-30)  )
	-- draw the apron where we rest our aircraft
	quad(position + v(-500,0,-30), position + v(-400,0,-30), position + v(-400,0,-130), position + v(-500,0,-130)    )
	geomflag(0)
	--quad(position + v(-500,-1,-30), position + v(500,-1,-30), position + v(500,-1,-30), position + v(-500,-1,-30)   )

	
	-- draw out the runway numbers at each end
	set_material('text', 1, 1, 1, 1)
	use_material('text')
	zbias(1, position + v(0,0,0), v(0,1,0))
	text(num+1, position + v(475,0,0), v(0,1,0), v(0,0,-1), 20.0, {center=true})
	text(num+1, position + v(-475,0,0), v(0,1,0), v(0,0,1), 20.0, {center=true})
	text(num+1, position + v(-450,0,-80), v(0,1,0), v(1,0,0), 50.0, {center=true})
	zbias(0)

	-- draw the runway pylons
	set_material('body', .3, .3, .3, 1)
	use_material('body')
	cylinder(10, position + v(0,-1,0), position + v(0,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(150,-1,0), position + v(150,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(300,-1,0), position + v(300,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(450,-1,0), position + v(450,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(-150,-1,0), position + v(-150,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(-300,-1,0), position + v(-300,-100,0), v(0,0,1), 10)
	cylinder(10, position + v(-450,-1,0), position + v(-450,-100,0), v(0,0,1), 10)

	if (math.fmod(get_time('SECONDS'), 2) > 1) then
		local color
		if stage > 1 or stage < 0 then
			color = v(1,0,0) -- red
		elseif stage == 1 then
			color = v(0,1,0) -- green
		else
			color = v(1,0.5,0) -- orange
		end
		billboard('smoke.png', 50, color, { 
			position + v(0,1,30), position + v(0,1,-30),
			position + v(100,1,30), position + v(100,1,-30), position + v(-100,1,30), position + v(-100,1,-30),
			position + v(200,1,30), position + v(200,1,-30), position + v(-200,1,30), position + v(-200,1,-30),
			position + v(300,1,30), position + v(300,1,-30), position + v(-300,1,30), position + v(-300,1,-30),
			position + v(400,1,30), position + v(400,1,-30), position + v(-400,1,30), position + v(-400,1,-30),
			position + v(500,1,30), position + v(500,1,-30), position + v(-500,1,30), position + v(-500,1,-30)
			})
	end
end

define_model('airport_1', {
	info = {
		bounding_radius=1500.0,
		materials = {'text', 'runway', 'body'},
		tags = {'surface_station'},
		angular_velocity = 1.0,
		num_docking_ports = 1,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 2 },
		undock_anim_stage_duration = { 0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-450,50,-80) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-450,50,-80) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+2000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,	
	},
	static = function(lod)
		-- Control Tower
		call_model('airport_control_tower', v(0,0,-100), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		-- Runways (these are dynamic due to the landing lights)
		createRunway(0, v(0,50,0))
	end,
})
