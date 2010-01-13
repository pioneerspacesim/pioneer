
ARG_STATION_BAY1_DOOR1 = 6
ARG_STATION_BAY1_DOOR2 = 10
ARG_STATION_BAY1_STAGE1 = 14
ARG_STATION_BAY1_STAGE2 = 18

define_model('spacestation_door', {
	info = function()
		return {
	--		lod_pixels = {20, 50, 0},
			bounding_radius = 200.0,
			materials = {'walls'}
		}
	end,
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		quad(a,b,c,d)
		quad(d,c,b,a)
		set_material('walls', .8,.8,0,1)
		use_material('walls')
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
})

-- final resting place of docked ship
define_model('spacestation_entry1_stage3', {
	info = function()
		return {
			lod_pixels = { 50, 0 },
			bounding_radius = 300,
			materials = {'wall1','wall2','text'}
		}
	end,
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
		quad(a,b,b2,a2)
		set_material('wall2', .8,0,.8,1)
		use_material('wall2')
		quad(c,d,d2,c2)
		quad(d,c,b,a)
		xref_quad(b,c,c2,b2)
		
		geomflag(0x8020)
		invisible_tri(v(0,100,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
	end,
	dynamic = function(lod)
		use_material('text')
		zbias(1, v(0,200,0), v(0,-1,0))
		-- starport name
		text(get_arg_string(0), v(60,200,-35), v(0,-1,0), v(-1,0,0), 5.0, {center=true})
		-- docking bay number
		text(get_arg_string(1), v(-60,200,-35), v(0,-1,0), v(-1,0,0), 7.0, {center=true})
		zbias(0)
		if lod > 1 then
			-- adverts
			call_model(get_arg_string(4), v(0,200,0), v(-1,0,0), v(0,0,-1), 40.0)
			call_model(get_arg_string(5), v(-100,100,0), v(0,-1,0), v(0,0,-1), 40.0)
			call_model(get_arg_string(6), v(100,100,0), v(0,1,0), v(0,0,-1), 40.0)
		end
	end
})

define_model('spacestation_entry1_stage2', {
	info = function()
		return {
			bounding_radius = 300,
			materials = {'wall1', 'wall2'}
		}
	end,
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
		door2 = vlerp(get_arg(ARG_STATION_BAY1_DOOR2), v(0,0,0), v(0,0,-100))
		call_model('spacestation_door', door2, v(1,0,0), v(0,1,0), 1.0)
		if get_arg(ARG_STATION_BAY1_STAGE2) ~= 0 then
			call_model('spacestation_entry1_stage3', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end
	end
})

define_model('spacestation_entry1_stage1', {
	info = function()
		return {
			lod_pixels = { 20, 0 },
			bounding_radius = 400,
			materials = {'wall1'}
		}
	end,
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
		xref_quad(b,c,c2,b2)
		quad(a,b,b2,a2)
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
	info = function()
		return {
			bounding_radius = 200,
			materials = {'wall'}
		}
	end,
	static = function(lod)
		set_material('wall', .2,.2,.6,1)
	end,
	dynamic = function(lod)
		local door1 = vlerp(get_arg(ARG_STATION_BAY1_DOOR1), v(0,-20,0), v(0,-20,-100))
		call_model('spacestation_door', door1, v(1,0,0), v(0,1,0), 1.0)
		
		if get_arg(ARG_STATION_BAY1_STAGE1) ~= 0 then
			call_model('spacestation_entry1_stage1', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end
		-- docking surface
		geomflag(0x10)
		invisible_tri(v(-100,-100,50), v(100,-100,50), v(100,-100,-50))
		invisible_tri(v(-100,-100,50), v(100,-100,-50), v(-100,-100,-50))
		geomflag(0x8000)
		invisible_tri(v(0,-150,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)

		call_model('spacestation_entry1_stage2', v(0,-300,0), v(1,0,0), v(0,1,0), 1.0)
	end
})

define_model('nice_spacestation', {
	info = function()
		return {
			bounding_radius=500.0,
			materials = {'text', 'body'}
		}
	end,
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
		call_model('spacestation_entry1', v(0,400,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		local textpos = v(0,400,-80)
		use_material('text')
		zbias(1, textpos, v(0,1,0))
		text(get_arg_string(0), textpos, v(0,1,0), v(1,0,0), 10.0)
		zbias(0)
	end
})

define_model('basic_groundstation', {
	info = function()
		return {
			bounding_radius=60.0,
			materials = {'body', 'text'}
		}
	end,
	static = function(lod)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		-- surface recognised for landing on (bay 1)
		geomflag(0x10)
		extrusion(v(0,0,-50), v(0,0,50), v(0,1,0), 1.0,
			v(-50,0,0), v(50,0,0), v(50,10,0), v(-50,10,0))
		geomflag(0x11)
		extrusion(v(0,0,70), v(0,0,170), v(0,1,0), 1.0,
			v(-50,0,0), v(50,0,0), v(50,10,0), v(-50,10,0))
		local bay1top = v(0,10,0)
		local bay2top = v(0,10,120)
		-- docking bay 1 location,xaxis,yaxis
		geomflag(0x8000)
		invisible_tri(bay1top, v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(1,0,0), v(0,1,0))
		geomflag(0x8001)
		invisible_tri(bay2top, v(1,0,0), v(0,1,0))
		geomflag(0)
		set_material('text', 1,1,1,1)
		use_material('text')
		zbias(1, bay1top, v(0,1,0))
		text("1", bay1top, v(0,1,0), v(1,0,0), 20.0)
		zbias(1, bay2top, v(0,1,0))
		text("2", bay2top, v(0,1,0), v(1,0,0), 20.0)
		zbias(0)
	end
})

