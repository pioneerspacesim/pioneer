		
-- NOTE
-- info->ship_dock_anim function's last docking anim ship location will be
-- used to place the ship when docked.

DOCKING_TIMEOUT_SECONDS = 300

define_model('spacestation_door', {
	info = {
			lod_pixels = { 10, 0},
			bounding_radius = 200.0,
			materials = {'stripes','walls'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		set_material('walls', .7,.7,.7,1)
		set_material('stripes', .9,.9,0,1)
		use_material('walls')
		if lod > 1 then
		texture('ships/4_eagles/tex8.png', v(.5,.5,0), v(.085,0,0), v(0,.111,0))
		end
		quad(a,b,c,d)
		quad(d,c,b,a)
		if lod > 1 then
			use_material('stripes')
			zbias(1, v(0,0,0), v(0,1,0))
			-- diagonal stripes on front
			quad(v(-10,0,-20), v(-30,0,-20), v(-70,0,20), v(-50,0,20))
			quad(v(30,0,-20), v(10,0,-20), v(-30,0,20), v(-10,0,20))
			quad(v(70,0,-20), v(50,0,-20), v(10,0,20), v(30,0,20))
			-- on back
			zbias(1, v(0,0,0), v(0,-1,0))
			quad(v(10,0,-20), v(30,0,-20), v(70,0,20), v(50,0,20))
			quad(v(-30,0,-20), v(-10,0,-20), v(30,0,20), v(10,0,20))
			quad(v(-70,0,-20), v(-50,0,-20), v(-10,0,20), v(-30,0,20))
			zbias(0)
		end
	end
})

-- final resting place of docked ship
define_model('spacestation_entry1_stage3', {
	info = {
			lod_pixels = { 50, 0 },
			bounding_radius = 300,
			materials = {'wall1','wall2','text'}
		},
	static = function(lod)
		-- mark as non-colliding (all >= 0x8000 is)
		geomflag(0x9000)
		local a = v(-100,200,50)
		local b = v(100,200,50)
		local c = v(100,200,-50)
		local d = v(-100,200,-50)
		local a2 = v(-100,0,50)
		local b2 = v(100,0,50)
		local c2 = v(100,0,-50)
		local d2 = v(-100,0,-50)
		set_material('text', 1,1,1,1)
		set_material('wall1', .8,.8,.8,1)
		use_material('wall1')
		
		if lod>1 then
		texture('ships/4_eagles/tex7.png', v(.5,.5,0), v(.01,0,0), v(0,.02,0)) --bottom
		end
		quad(a,b,b2,a2)
		set_material('wall2', .8,0,.8,1)
		use_material('wall2')
		if lod>1 then
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --top
		end
		quad(c,d,d2,c2)
		if lod>1 then
		texture('ships/4_eagles/tex1.png', v(.5,.55,0), v(.01,0,0), v(0,0,-.9)) --back
		end
		quad(d,c,b,a)
		if lod>1 then
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(0,0,.6), v(0,.005,0)) --l/r
		end
		xref_quad(b,c,c2,b2)
		
		geomflag(0x8020)
		invisible_tri(v(0,100,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			use_material('text')
			zbias(1, v(0,200,0), v(0,-1,0))
			-- starport name
			text(get_arg_string(0), v(60,200,-35), v(0,-1,0), v(-1,0,0), 5.0, {center=true})
			-- docking bay number
			text("DOCKING BAY 1", v(-60,200,-35), v(0,-1,0), v(-1,0,0), 7.0, {center=true})
			zbias(0)
			-- adverts
			call_model(get_arg_string(4), v(0,200,0), v(-1,0,0), v(0,0,-1), 40.0)
			call_model(get_arg_string(5), v(-100,100,0), v(0,-1,0), v(0,0,-1), 40.0)
			call_model(get_arg_string(6), v(100,100,0), v(0,1,0), v(0,0,-1), 40.0)
		end
	end
})

define_model('spacestation_entry1_stage2', {
	info = {
			bounding_radius = 300,
			materials = {'wall1', 'wall2'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		local a2 = v(-100,-200,50)
		local b2 = v(100,-200,50)
		local c2 = v(100,-200,-50)
		local d2 = v(-100,-200,-50)
		set_material('wall1', .8,.8,.8,1)
		use_material('wall1')
		--if lod > 1 then			
		texture('ships/4_eagles/tex7.png', v(.5,.5,0), v(.01,0,0), v(0,.02,0))
		--end
		quad(a,b,b2,a2)
		quad(c,d,d2,c2)
		quad(a2,b2,c2,d2)
		set_material('wall2', .8,.8,.5,1.0)
		use_material('wall2')
		xref_quad(b,c,c2,b2)
		geomflag(0x8010)
		invisible_tri(v(0,-100,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
	end,
	dynamic = function(lod)
--		material(.8,0,0, 0,0,0, 0, 0,0,0)
		local stage = get_arg(ARG_STATION_BAY1_STAGE)
		local pos = get_arg(ARG_STATION_BAY1_POS)
		if stage == 3 or stage == 7 or stage == -1 or stage == -5 then
			-- good, door is opening
		elseif stage == 5 or stage == 9 or stage == -3 or stage == -7 then -- door is closing
			pos = 1.0 - pos
		elseif stage == 4 or stage == 8 or stage == -2 or stage == -6 then -- door is open
			pos = 1
		else -- door is closed
			pos = 0
		end
		door2 = vlerp(pos, v(0,0,0), v(0,0,-100))
		call_model('spacestation_door', door2, v(1,0,0), v(0,1,0), 1.0)
		if stage >= 7 or (stage >= -4 and stage <= -1) then
			call_model('spacestation_entry1_stage3', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end
	end
})

define_model('spacestation_entry1_stage1', {
	info = {
			lod_pixels = { 20, 0 },
			bounding_radius = 400,
			materials = {'wall1'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		local a2 = v(-100,-300,50)
		local b2 = v(100,-300,50)
		local c2 = v(100,-300,-50)
		local d2 = v(-100,-300,-50)
		
		set_material('wall1', .5,.5,.5,1)
		use_material('wall1')
		if lod>1 then
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(0,0,.6), v(0,.005,0)) --l/r
		else texture('ships/4_eagles/tex2_s.png', v(.5,.5,0), v(0,0,.6), v(0,.005,0))
		end
		xref_quad(b,c,c2,b2)
		if lod>1 then
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --bottom
		else 	texture('ships/4_eagles/tex2_s.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0))
		end
		quad(a,b,b2,a2)
 		if lod>1 then
 		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --top
		else texture('ships/4_eagles/tex2_s.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0))
		end
		quad(c,d,d2,c2)
	end,
	dynamic = function(lod)
		-- adverts
		if lod > 1 then
			call_model(get_arg_string(4), v(-100,-100,0), v(0,-1,0), v(0,0,-1), 40.0)
			call_model(get_arg_string(4), v(100,-100,0), v(0,1,0), v(0,0,-1), 40.0)
		end
	end
})

define_model('spacestation_entry1', {
	info = {
			bounding_radius = 200,
			materials = {'wall'}
		},
	static = function(lod)
		set_material('wall', .2,.2,.6,1)
		-- docking surface
		geomflag(0x10)
		invisible_tri(v(-100,-100,50), v(100,-100,50), v(100,-100,-50))
		invisible_tri(v(-100,-100,50), v(100,-100,-50), v(-100,-100,-50))
		geomflag(0)
	end,
	dynamic = function(lod)
		local stage = get_arg(ARG_STATION_BAY1_STAGE)
		local pos = get_arg(ARG_STATION_BAY1_POS)
		if stage == 1 then
			-- open door at start of docking permission
			pos = math.min(pos*50, 1.0)
		elseif stage == -8 then
			-- open door
		elseif stage == 2 then
			-- close door
			pos = 1.0 - pos
		elseif stage == -9 then
			-- launch: close door after a little while
			pos = 1.0 - pos
		else
			pos = 0
		end
		local door1 = vlerp(pos, v(0,-20,0), v(0,-20,-100))
		call_model('spacestation_door', door1, v(1,0,0), v(0,1,0), 1.0)
		
		if (stage >= 1 and stage <= 5) or (stage >= -9 and stage <= -5) then
			call_model('spacestation_entry1_stage1', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end

		call_model('spacestation_entry1_stage2', v(0,-300,0), v(1,0,0), v(0,1,0), 1.0)
	end
})

function simple_lift_docking_port(baynum, pos)
	local stage = get_arg(ARG_STATION_BAY1_STAGE + baynum)
	local spos = get_arg(ARG_STATION_BAY1_POS + baynum)
	local baypos = 0

	if stage == 3 then
		baypos = -75*spos
	elseif stage == -2 then
		baypos = -75*(1-spos)
	end
	if stage >= 4 or stage == -1 then
		if stage > 4 then
			spos = 1
		elseif stage == -1 then
			spos = 1-spos
		end
		use_material('body')
		baypos = -75
		cuboid(pos+v(-50,-10,-50), v(50*spos, 5, 100))
		cuboid(pos+v(50*(1.0-spos),-10,-50), v(50*spos, 5, 100))
		function arrows(p1, p2)
			quad(p1, p1+v(5,0,-5), p1+v(0,0,-10), p1+v(-10,0,0))
			quad(p1, p1+v(-10,0,0), p1+v(0,0,10), p1+v(5,0,5))
			quad(p2, p2+v(10,0,0), p2+v(0,0,-10), p2+v(-5,0,-5))
			quad(p2, p2+v(-5,0,5), p2+v(0,0,10), p2+v(10,0,0))
		end
		use_material('markings')
		zbias(1, pos, v(0,1,0))
		arrows(pos+v(25+50*(1.0-spos),-5, 0), pos+v(-75+50*spos, -5, 0))
		zbias(0)
	end
	use_material('lift_floor')
	geomflag(0x10 + baynum)
	quad(pos+v(-50,baypos,50), pos+v(50,baypos,50), pos+v(50,baypos,-50), pos+v(-50,baypos,-50))
	geomflag(0)
	--use_material('body')
	zbias(1, pos+v(0,baypos,0), v(0,1,0))
	use_material('text')
	       text(0, pos+v(0,baypos,0), v(0,1,0), v(1,0,0), 2.0, {center=true})
       zbias(0)
	text(baynum+1, pos+v(0,baypos,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
	zbias(0)

	use_material('inside')
	texture('models/stationwall.png', v(.5,.9,0),v(0,0,1.1),v(0,-.009,0))
	xref_quad(pos+v(50,0,50), pos+v(50,0,-50), pos+v(50,-75,-50), pos+v(50,-75,50))

	texture('models/stationwall.png', v(.5,.9,0),v(.01,0,0),v(0,-.009,0))
	quad(pos+v(-50,-75,-50), pos+v(50,-75,-50), pos+v(50,0,-50), pos+v(-50,0,-50))
	quad(pos+v(50,-75,50), pos+v(-50,-75,50), pos+v(-50,0,50), pos+v(50,0,50))
	texture(nil)

	if (math.fmod(get_arg(1), 2) > 1) then
		local color
		if stage ~= 0 then
			color = v(1,0.5,0)
		else
			color = v(0,1,0)
		end
		billboard('smoke.png', 50, color, { pos+v(-50,1,50), pos+v(50,1,50), pos+v(-50,1,-50), pos+v(50,1,-50) })
	end
end

define_model('mushroom_station', {
	info = {
		lod_pixels = {10,100,300,0},
		bounding_radius=200.0,
		materials = {'body', 'text', 'markings', 'lift_floor', 'tower_base', 'inside'},
		tags = {'surface_station'},
		num_docking_ports = 2,
		-- 1 - permission granted
		-- 2 - position docked ship
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 2, 4, 4 },
		undock_anim_stage_duration = { 4, 4 },
		-- this stuff doesn't work right with the new docking
		-- code
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local port_pos = { v(-100,100,0), v(100,100,0) }
			if stage == 2 then
				return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			elseif stage == 3 then
				return { vlerp(t, from, port_pos[port] + v(0,-75,0) - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			elseif stage == 4 or stage == -1 then
				return { port_pos[port] + v(0,-75,0) - v(0,ship_aabb.min:y(),0), v(1,0,0), v(0,1,0) }
			elseif stage == -2 then
				return { vlerp(t, from, port_pos[port] + v(0,1,0) - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			local port_pos = { v(-100,100,0), v(100,100,0) }
			if stage == 1 then
				return { v(port_pos[port]:x(), port_pos[port]:y()+10000, port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			elseif stage == 2 then
				return { v(port_pos[port]:x(), port_pos[port]:y(), port_pos[port]:z()), v(1,0,0), v(0,1,0) }
			end
		end,
	},
	static = function(lod)
		set_material('inside',.35,.4,.4,1,.2,.35,.35,30)
		set_material('markings', 1,0,0,1)
		set_material('text', 1,1,1,1)
		set_material('body', .5,.5,.5,1)
  		set_material('lift_floor', .6,.6,.6,1)
		set_material('tower_base', .2,.2,.5,1)
		use_material('tower_base')
		tapered_cylinder(16, v(0,0,-350), v(0,120,-350), v(0,0,1), 200, 60)
		use_material('body')
		--if lod>2 then
		--texture('ships/4_eagles/tex11.png', v(.5,.5,0), v(10,0,0), v(0,.5,0))
		--else 	texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0))
		--end
		cylinder(8, v(0,120,-350), v(0,210,-350), v(0,0,1), 20)
		--texture(nil)
		tapered_cylinder(8, v(0,210,-350), v(0,225,-350), v(0,0,1), 20, 30)
		local port_pos = { v(-100,100,0), v(100,100,0) }
		function makePortTop(pos)
			local a = pos + v(-100,0,-100)
			local b = pos + v(100,0,-100)
			local c = pos + v(100,0,100)
			local d = pos + v(-100,0,100)
			local e = pos + v(-50,0,-50)
			local f = pos + v(50,0,-50)
			local g = pos + v(50,0,50)
			local h = pos + v(-50,0,50)
			-- top bits around docking lift
			quad(a,e,f,b)
			xref_quad(b,f,g,c)
			quad(d,c,g,h)
		end
		makePortTop(port_pos[1])
		makePortTop(port_pos[2])

		quad(v(200,100,100), v(-200,100,100), v(-250,0,150), v(250,0,150))
		quad(v(-200,100,-100), v(200,100,-100), v(250,0,-150), v(-250,0,-150))
		xref_quad(v(200,100,-100), v(200,100,100), v(250,0,150), v(250,0,-150))

		--zbias(1,v(0,100.01,20),v(0,1,0))
		--call_model('ad_pioneer_0',v(0,100.01,20),v(0,0,1),v(.001,0,-1),40)
		--zbias(0)

        if lod == 4 then
        	local pos1 = v(-100,100,0)
	    	zbias(1,pos1+v(0,-50,-50),v(0,0,1))
			call_model('ad_pioneer_0',pos1+v(0,-65,-50),v(1,0,0),v(0,1,0),20)
			zbias(0)

			zbias(1,pos1+v(0,-65,50),v(0,0,-1))
			call_model('ad_acme_1',pos1+v(0,-65,50),v(-1,0,0),v(0,1,0),20)
			zbias(0)

			zbias(1,pos1+v(50,-65,0),v(-1,0,0))
			call_model('ad_cola_1',pos1+v(50,-65,0),v(0,0,1),v(0,1,0),20)
			zbias(0)

			zbias(1,pos1+v(-49.999,-65,0),v(1,0,0))
			call_model('ad_sirius_1',pos1+v(-49.999,-65,0),v(0,0,-1),v(0,1,0),20)
			zbias(0)
		end
	end,
 	dynamic = function(lod)
		local port_pos = { v(-100,100,0), v(100,100,0) }
		simple_lift_docking_port(0, port_pos[1])
		simple_lift_docking_port(1, port_pos[2])
		-- light on tower
		local lightphase = math.fmod(get_arg(1)+0.46956, 1)
		billboard('smoke.png', 40, lightphase > .5 and v(1,0,0) or v(0,1,0), { v(0, 228, -350) })
	end
})


define_model('big_crappy_spacestation', {
	info = {
		bounding_radius=500.0,
		materials = {'text', 'body0'},
		tags = {'orbital_station'},
		angular_velocity = 0.1,
		lod_pixels = {0},
		num_docking_ports = 4,
		-- for stations where each docking port shares the
		-- same front door, set dock_one_at_a_time_please = true,
		dock_one_at_a_time_please = true,
		dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 10.0, 5.0, 5.0 },
		undock_anim_stage_duration = { 5.0, 5.0, 10.0 },
		ship_dock_anim = function(port, stage, t, from, ship_aabb)
			local baypos = { v(-150,-350,0), v(-100,-350,100),
				v(100,-350,100), v(150,-350,0) }
			if stage == 2 then
				return { vlerp(t, from, v(0,-350,0)), v(1,0,0), v(0,0,1) }
			elseif stage == 3 then
				return { v(0,-350,0), v(1,0,0), v(0,1,0) }
			elseif stage == 4 then
				return { vlerp(t, v(0,-350,0), baypos[port]), v(1,0,0), v(0,1,0) }
			elseif stage == -1 then
				return { vlerp(t, baypos[port], v(0,-350,0)), v(1,0,0), v(0,1,0) }
			elseif stage == -2 then
				return { v(0,-350,0), v(-1,0,0), v(0,0,-1) }
			elseif stage == -3 then
				return { vlerp(t, v(0,-350,0), v(0,600,0)), v(-1,0,0), v(0,0,-1) }
			end
		end,
		ship_approach_waypoints = function(port, stage)
			if stage == 1 then
				return { v(0,4000,0), v(1,0,0), v(0,0,1) }
			elseif stage == 2 then
				return { v(0,600,0), v(1,0,0), v(0,0,1) }
			end
		end,
	},
	static = function(lod)
		
		set_material('body0', 1,1,1,1, 1,1,1,100)
		use_material('body0')
		if lod<2 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.033,0,0), v(0,.033,0))
		else
		texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.033,0,0), v(0,.033,0))
		end
		lathe(16, v(0,500,0), v(0,-500,0), v(1,0,0), {0,100, 0,150, 0.1,200, 0.2,149, 0.4,149, 0.45,300,0.55,300, 0.6,149, 0.7,149, 0.75,300, 0.95,300, 1.0,150, 1.0,0.0})
		--alt tex body
		if lod<2 then
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		else
		texture('ships/4_eagles/tex2_s.png', v(.6,.5,0), v(.01,0,0), v(0,.01,0))
		end
		lathe(16, v(0,300,0), v(0,-250,0), v(1,0,0), {0,150, 1.0,150})
		-- front cap
		if lod<2 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.03,0,0), v(0,0,1))
		else
		texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.03,0,0), v(0,0,1))
		end
		lathe(16, v(0,501,0), v(0,-501,0), v(1,0,0), {0,100, 0,150, 0.1,199, 0.2,149, 0.4,149, 0.45,299,0.55,299, 0.6,149, 0.7,149, 0.75,299, 0.95,299, 1.0,150, 1.0,0.0})		
		--tube(16, v(0,500,0), v(0,501,0), v(1,0,0), 101, 144)
		--tube(16, v(0,500,0), v(0,501,0), v(0,1,0), 101, 150)
		-- struts to outer ring
		if lod<2 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.033,0,0), v(0,0,.33))
		else
		texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.033,0,0), v(0,0,.33))
		end
		ring(8, v(0,0,290), v(0,0,1500), v(1,0,0), 20)
		ring(8, v(0,0,-290), v(0,0,-1500), v(1,0,0), 20)
		-- outer ring
		if lod<2 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.005,0,0), v(0,0,.5))
		else
		texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.005,0,0), v(0,0,.5))
		end	
		tube(32, v(0,-100,0), v(0,100,0), v(0,0,1), 1500, 1600)
		if lod<2 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,.005,0))
		else
		texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.01,0,0), v(0,.005,0))
		end	
		tube(32, v(0,-99,0), v(0,99,0), v(0,0,1), 1499, 1601)
		-- the inside!
		set_insideout(true)
		set_local_lighting(true)
		use_light(1)
		use_light(2)
	if lod<2 then
		texture('ships/4_eagles/tex2.png', v(.6,.5,0), v(.003,0,0), v(0,.003,0))
		else
		texture('ships/4_eagles/tex2_s.png', v(.5,.5,0), v(.003,0,0), v(0,.003,0))
		end	
		lathe(16, v(0,500,0), v(0,-500,0), v(1,0,0), {0,100, 0.7,100, 0.75,250, 0.95,250, 0.95,0})
		set_insideout(false)
		--floor
		if lod<2 then
		texture('ships/4_eagles/tex2.png', v(.6,.5,0), v(.01,0,0), v(0,0,.5))
		else
		texture('ships/4_eagles/tex2_s.png', v(.5,.5,0), v(.003,0,0), v(0,.003,0))
		end
		lathe(16, v(0,-449,0), v(0,-450,0), v(1,0,0), {0,0, 0,249, 1,249, 1,0})
		set_local_lighting(false)
		billboard('smoke.png', 50.0, v(0,0,1), {v(0,200,0)})
		billboard('smoke.png', 50.0, v(1,1,0), {v(0,-300,0)})

		-- docking trigger surface (only need to indicate surface for
		-- port zero since this is a 'dock_one_at_a_time_please' station
		geomflag(0x10)
		invisible_tri(v(-100,600,-100),v(100,600,100),v(100,600,-100))
		invisible_tri(v(-100,600,-100), v(-100,600,100),v(100,600,100))
		geomflag(0)
	end,
	dynamic = function(lod)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		 if lod<2 then
			local textpos = v(0,500,-115) --0,500,-110
			use_material('text')
			zbias(1, textpos, v(0,100,0))
			text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 10.0, {center=true})
			--text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 10.0, {center=true})
			local textpos = v(0,-500,-10) --0,500,-110
			use_material('text')
			zbias(1, textpos, v(0,100,0))
			text("created by", textpos, v(0,-1000,-115), v(-1,0,0), 7.0, {center=true})
			local textpos = v(0,-500,0) --0,500,-110
			use_material('text')
			zbias(1, textpos, v(0,100,0))
			text("Tom Morton", textpos, v(0,-1000,0), v(-1,0,0), 10.0, {center=true})
			zbias(0)
		end
		 billboard('smoke.png', 20.0, v(1,1,0), {
			vlerp(get_arg(1),v(0,12,419),v(0,12,1500)),
			vlerp(get_arg(1),v(0,12,-419),v(0,12,-1500))})
		set_light(1, 0.00005, v(0,0+1000*math.fmod(get_arg(1),1.0),0), v(0,0,0.5))
		set_light(2, 0.00001, v(0,-300,0), v(0.5,1.0,0))
	end,
})

define_model('nice_spacestation', {
	info = {
			bounding_radius=500.0,
			materials = {'text', 'body'},
			tags = {'orbital_station'},
			angular_velocity = 0.15,
			lod_pixels = { 50, 0 },
			num_docking_ports = 1,
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
					return { vlerp(t, from, v(0,250,0)), v(1,0,0), v(0,0,1) }
				elseif stage == 3 then
					return { from, v(1,0,0), v(0,0,1) }
				elseif stage == 4 then
					return { vlerp(t, from, v(0,0,0)), v(1,0,0), v(0,0,1) }
				elseif stage == 5 then
					return { vlerp(t, from, v(0,0,0)), v(-1,0,0), v(0,0,-1) }
				elseif stage == 6 or stage == 7 then
					return { from, v(-1,0,0), v(0,0,-1) }
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
					return { v(0,4000,0), v(1,0,0), v(0,0,1) }
				elseif stage == 2 then
					return { v(0,300,0), v(1,0,0), v(0,0,1) }
				end
			end,
		},
	static = function(lod)
		-- front
		-- f7   f3    f6
		--
		--    a b
		-- f4 d c   f2
		--
		-- f8  f1    f5
		local a = v(-100,400,50)
		local b = v(100,400,50)
		local c = v(100,400,-50)
		local d = v(-100,400,-50)
		
		local f1 = v(0,400,-400)
		local f2 = v(400,400,0)
		local f3 = v(0,400,400)
		local f4 = v(-400,400,0)

		local f5 = v(400,0,-400)
		local f6 = v(400,0,400)
		local f7 = v(-400,0,400)
		local f8 = v(-400,0,-400)

		-- back face points
		local f1b = v(0,-400,-400)
		local f2b = v(400,-400,0)
		local f3b = v(0,-400,400)
		local f4b = v(-400,-400,0)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		--front face
		if lod>1 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		else 	texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		end
		tri(f1,d,c)
		xref_tri(f1,c,f2)
		xref_tri(f2,c,b)
		xref_tri(f2,b,f3)
		tri(f3,b,a)
		--pyramid sides
		if lod>1 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		else texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		end
		xref_tri(f2,f3,f6)
		xref_tri(f5,f1,f2)
		xref_tri(f6,f3b,f2b)
		xref_tri(f5,f2b,f1b)
		if lod>1 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(0,0,.9), v(0,.01,0))
		else texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(0,0,.9), v(0,.01,0))
		end
		xref_quad(f2,f6,f2b,f5) -- sides
		if lod>1 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		else 	texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		end
		quad(f3,f7,f3b,f6) -- top
		quad(f5,f1b,f8,f1) -- bottom
		if lod>1 then
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		else texture('ships/4_eagles/tex12_s.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		end
		quad(f1b,f2b,f3b,f4b) -- rear
		call_model('spacestation_entry1', v(0,400,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			local textpos = v(0,400,-80)
			use_material('text')
			zbias(1, textpos, v(0,1,0))
			text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 11, {center=true})
			zbias(0)
		end
		if (math.fmod(get_arg(1), 2) > 1) then
			billboard('smoke.png', 50, v(0,1,0), { v(-150,401,0), v(-175,401,0), v(-200,401,0) })
		else
			billboard('smoke.png', 50, v(0,1,0), { v(150,401,0), v(175,401,0), v(200,401,0) })
		end
	end
})
--[[
define_model('hoop_spacestation', {
	info = {
			bounding_radius=500.0,
			materials = {'text', 'body'},
			tags = {'orbital_station'},
			angular_velocity = 0.08,
			lod_pixels = { 50, 0 },
			num_docking_ports = 1,
		},
	static = function(lod)
		-- front
		-- f7   f3    f6
		--
		--    a b
		-- f4 d c   f2
		--
		-- f8  f1    f5
		local a = v(-100,400,50)
		local b = v(100,400,50)
		local c = v(100,400,-50)
		local d = v(-100,400,-50)
		
		local f1 = v(0,400,-400)
		local f2 = v(400,400,0)
		local f3 = v(0,400,400)
		local f4 = v(-400,400,0)

		local f5 = v(400,0,-400)
		local f6 = v(400,0,400)
		local f7 = v(-400,0,400)
		local f8 = v(-400,0,-400)

		-- back face points
		local f1b = v(0,-400,-400)
		local f2b = v(400,-400,0)
		local f3b = v(0,-400,400)
		local f4b = v(-400,-400,0)
		
		set_material('text', 1.0,0.5,1.0,1)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		--front face
		tri(f1,d,c)
		xref_tri(f1,c,f2)
		xref_tri(f2,c,b)
		xref_tri(f2,b,f3)
		tri(f3,b,a)
		--pyramid sides
		xref_tri(f2,f3,f6)
		xref_tri(f5,f1,f2)
		xref_tri(f6,f3b,f2b)
		xref_tri(f5,f2b,f1b)
		xref_quad(f2,f6,f2b,f5) -- sides
		quad(f3,f7,f3b,f6) -- top
		quad(f5,f1b,f8,f1) -- bottom
		quad(f1b,f2b,f3b,f4b) -- rear
		tube(16, v(0,200,0), v(0,-200,0), v(0,0,1), 1300.0, 1500.0)
		extrusion(v(0,0,-400), v(0,0,-1300), v(1,0,0), 100.0,
			v(-1,-1,0), v(1,-1,0), v(1,1,0), v(-1,1,0))
		extrusion(v(0,0,1300), v(0,0,400), v(1,0,0), 100.0,
			v(-1,-1,0), v(1,-1,0), v(1,1,0), v(-1,1,0))
		call_model('spacestation_entry1', v(0,400,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			local textpos = v(0,400,-80)
			use_material('text')
			zbias(1, textpos, v(0,1,0))
			text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 10.0)
			zbias(0)
		end
	end
})
--]]

define_model('basic_groundstation', {
	info = {
			bounding_radius=200.0,
			materials = {'body', 'text', 'tower_base'},
			num_docking_ports = 2,
			-- 1 - permission granted
			-- 2 - position docked ship
			dock_anim_stage_duration = { DOCKING_TIMEOUT_SECONDS, 2 },
			undock_anim_stage_duration = { 0 }, 
			-- this stuff doesn't work right with the new docking
			-- code
			ship_dock_anim = function(port, stage, t, from, ship_aabb)
				local port_pos = { v(-100,10,50), v(100,10,50) }
				if stage == 2 then 
					return { vlerp(t, from, port_pos[port] - v(0,ship_aabb.min:y(),0)), v(1,0,0), v(0,1,0) }
				end
			end,
		},
	static = function(lod)
		set_material('tower_base', .2,.2,.5,1)
		use_material('tower_base')
		tapered_cylinder(16, v(0,0,-150), v(0,40,-150), v(0,0,1), 100, 30)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		cylinder(8, v(0,40,-150), v(0,70,-150), v(0,0,1), 10)
		tapered_cylinder(8, v(0,70,-150), v(0,85,-150), v(0,0,1), 10, 15)
		-- surface recognised for landing on (bay 1)
		geomflag(0x10)
		extrusion(v(-100,0,0), v(-100,0,100), v(0,1,0), 1.0,
			v(-50,0,0), v(50,0,0), v(50,10,0), v(-50,10,0))
		geomflag(0x11)
		extrusion(v(100,0,0), v(100,0,100), v(0,1,0), 1.0,
			v(-50,0,0), v(50,0,0), v(50,10,0), v(-50,10,0))
		local bay1top = v(-100,10,50)
		local bay2top = v(100,10,50)
		-- docking bay 1 location,xaxis,yaxis
		set_material('text', 1,1,1,1)
		use_material('text')
		zbias(1, bay1top, v(0,1,0))
		text("1", bay1top, v(0,1,0), v(1,0,0), 20.0, {center=true})
		zbias(1, bay2top, v(0,1,0))
		text("2", bay2top, v(0,1,0), v(1,0,0), 20.0, {center=true})
		zbias(0)
	end,
	dynamic = function(lod)
		-- light on tower
		local lightphase = math.fmod(get_arg(1)+0.620486, 1)
		billboard('smoke.png', 40, lightphase > .5 and v(1,0,0) or v(0,1,0), { v(0, 88, -150) })
	end
})

