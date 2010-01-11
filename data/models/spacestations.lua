
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
		
		--[[ adverts
		lod1:subobject(var:16, v(0,200,0), v(0,0,-1), v(0,-1,0), scale=40.0)
		lod1:subobject(var:17, v(-100,100,0), v(0,0,-1), v(1,0,0), scale=40.0)
		lod1:subobject(var:18, v(100,100,0), v(0,0,-1), v(-1,0,0), scale=40.0)
		--]]
		geomflag(0x8020)
		--[[invisible]]tri(v(0,100,0), v(0,0,0), v(0,0,0))
		--[[invisible]]tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
	end,
	dynamic = function(lod)
		zbias(1, v(0,200,0), v(0,-1,0))
		-- starport name
		text(get_arg_string(0), v(60,200,-35), v(0,-1,0), v(-1,0,0), 5.0)
		-- docking bay number
		text(get_arg_string(1), v(-60,200,-35), v(0,-1,0), v(-1,0,0), 7.0)
		zbias(0)
	end
})

