


--[[
define_model('building1', {
	info = {
		bounding_radius = 10,
		materials={'mat'},
		tags = {'city_building', 'city_power', 'city_starport_building'},
	},
	static = function(lod)
		set_material("mat", .5, .5, .5, 1)
		use_material("mat")
		extrusion(v(0,0,-7.5), v(0,0,7.5), v(0,1,0), 6.0,
			v(-1,0,0), v(1,0,0), v(1,1,0), v(-1,1,0))
	end
})

define_model('building2', {
	info = {
		bounding_radius = 25,
		materials={'mat'},
		tags = {'city_building'},
	},
	static = function(lod)
		set_material("mat", .5, .5, .5, 1)
		use_material("mat")
		extrusion(v(0,0,-16), v(0,0,16), v(0,1,0), 1.0,
			v(-16,0,0), v(16,0,0), v(16,20,0), v(-16,20,0))
	end
})
--]]

define_model('skyscraper1', {
	info = {
		bounding_radius=200,
		materials={'gray1', 'gray2'},
		tags = {'city_building'},
	},
	static = function(lod)
		-- Inset black bit
		-- h g
		--
		-- e f
		-- d c
		-- a b
		local a = v(-20,0,10)
		local b = v(20,0,10)
		local c = v(20,40,10)
		local d = v(-20,40,10)
		local e = v(-20,48,10)
		local f = v(20,48,10)
		local g = v(20,200,10)
		local h = v(-20,200,10)
		set_material("gray1", .2,.2,.2,1, .7,.7,.7, 10)
		use_material("gray1")
		quad(a,b,c,d)
		quad(e,f,g,h)
		-- Outset
		-- h2 g2
		--
		-- e2 f2
		-- d2 c2
		-- a2 b2
		local a2 = v(-20,0,15)
		local b2 = v(20,0,15)
		local c2 = v(20,40,15)
		local d2 = v(-20,40,15)
		local e2 = v(-20,48,15)
		local f2 = v(20,48,15)
		local g2 = v(20,200,15)
		local h2 = v(-20,200,15)
		set_material("gray2", .9,.9,.9,1)
		use_material("gray2")
		-- inset bits
		quad(b,b2,c2,c)
		quad(d,c,c2,d2)
		quad(a,d,d2,a2)
		quad(f,e,e2,f2)
		quad(g,f,f2,g2)
		quad(h,g,g2,h2)
		quad(e,h,h2,e2)
		-- front face
		-- fd fc
		-- fa fb
		local fa = v(-30,0,15)
		local fb = v(30,0,15)
		local fc = v(30,210,15)
		local fd = v(-30,210,15)
		quad(d2,c2,f2,e2)
		quad(fc,fd,h2,g2)
		
		-- tri fan on front face
		xref_tri(g2,f2,c2)
		xref_tri(g2,c2,b2)
		xref_tri(g2,b2,fb)
		xref_tri(g2,fb,fc)

		top = v(0,260,-40)
		-- rear
		xref_quad(fc,fb,v(0,0,-40),v(0,210,-40))
		xref_tri(fc,v(0,210,-40),top)
		-- spire
		spire = v(0,300,-40)
		xref_tri(v(1.5,257.5,-37.25),
			top,spire)
		tri(v(-1.5,257.5,-37.25),v(1.5,257.5,-37.25),
			spire)
		-- front roof triangle section
		--      top
		--      tc
		--
		--   ta   tb
		-- fd       fc
		local ta = v(-21, 215, 9.5)
		local tb = v(21, 215, 9.5)
		local tc = v(0, 250, -29)
		quad(fd,fc,tb,ta)
		xref_quad(fc,top,tc,tb)
		use_material("gray1")
		tri(ta,tb,tc)
	end
})

define_model('clockhand', {
	info = {
		bounding_radius = 1,
		materials={'mat'}
	},
	static = function(lod)
		set_material("mat",0,0,1,1, 0,0,0, 10)
		use_material("mat")
		zbias(3, v(0,0,0), v(0,0,1))
		tri(v(-0.06, -0.06, 0), v(0.06, -0.06, 0), v(0, 1, 0))
	end
})

define_model('clock', {
	info = {
		bounding_radius=1,
		materials={'face','numbers'}
	},
	static = function(lod)
		set_material("face", 1,1,1,1)
		set_material("numbers",.5,.5,0,1)
		use_material("face")
		zbias(1, v(0,0,0), v(0,0,1))
		circle(24, v(0,0,0), v(0,0,1), v(0,1,0), 1.0)
		zbias(2, v(0,0,0), v(0,0,1))

		use_material("numbers")
		text("12", v(0,0.85,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("1", v(0.425,0.74,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("2", v(0.74,0.43,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("3", v(0.85,0,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("4", v(0.74,-.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("5", v(0.425,-.74,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("6", v(0,-.85,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("7", v(-.425,-.736,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("8", v(-.74,-.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("9", v(-.85,0,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("10", v(-.74,.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("11", v(-.425,.736,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		zbias(0)

	end,
	dynamic = function(lod)
		local handPos = -2*math.pi * get_arg(3)
		call_model("clockhand", v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 0.65)
		handPos = handPos / 12
		call_model("clockhand", v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 0.45)
	end
})

--[[
define_model('church', {
	info = {
		bounding_radius=45,
		materials={'body','spire'},
		tags = {'city_building'},
	},
	static = function(lod)
		set_material("body", .5, .5, .3,1)
		use_material("body")
		extrusion(v(0,0,-18), v(0,0,12), v(0,1,0), 1.0,
			v(-7,0,0), v(7,0,0), v(7,10,0), v(-7,10,0))
		extrusion(v(0,0,-18), v(0,0,-4), v(0,1,0), 1.0,
			v(-7,10,0), v(7,10,0), v(7,25,0), v(-7,25,0))
		local roof1 = v(0,20,12)
		local roof2 = v(0,20,-4)
		local spire = v(0,45,-11)
		tri(v(-7,10,12), v(7,10,12), roof1)
		
		set_material("spire", .8, .3, .0,1)
		use_material("spire")
		xref_quad(roof2, roof1, v(7,10,12), v(7,10,-4))
		xref_tri(spire, v(7,25,-4), v(7,25,-18))
		tri(spire, v(-7,25,-4), v(7,25,-4))
		tri(spire, v(7,25,-18), v(-7,25,-18))

		local clockpos1 = v(-7,18,-11)
		local clockpos2 = v(7,18,-11)
		
		call_model("clock", clockpos1, v(0,0,1), v(0,1,0), 2.5)
		call_model("clock", clockpos2, v(0,0,-1), v(0,1,0), 2.5)
	end
})
--]]

function biodome(lod, trans)
	local d = 1/math.sqrt(2)
	local height = 40
	use_material('base')
	local yank=-690
	xref_cubic_bezier_quad(32, 1, trans*v(0,0,500), trans*v(0,0.25*height,500), trans*v(0,0.75*height,500), trans*v(0,height,500),
			trans*v(yank,0,500), trans*v(yank,0.25*height,500), trans*v(yank,0.75*height,500), trans*v(yank,height,500),
			trans*v(yank,0,-500), trans*v(yank,0.25*height,-500), trans*v(yank,0.75*height,-500), trans*v(yank,height,-500),
			trans*v(0,0,-500), trans*v(0,0.25*height,-500), trans*v(0,0.75*height,-500), trans*v(0,height,-500))
	use_material('green')
	zbias(1, v(0,height,0), v(0,1,0))
	local s = 660
	quadric_bezier_quad(16, 16, trans*v(d*500,height,d*-500), trans*v(0,height,-s), trans*v(d*-500,height,d*-500),
		trans*v(s,height,0), trans*v(0,height,0), trans*v(-s,height,0),
		trans*v(d*500,height,d*500), trans*v(0,height,s), trans*v(d*-500,height,d*500))
	use_material('dome')
	quadric_bezier_quad(16, 16, trans*v(d*500,height,d*-500), trans*v(0,height,-s), trans*v(d*-500,height,d*-500),
		trans*v(s,height,0), trans*v(0,500,0), trans*v(-s,height,0),
		trans*v(d*500,height,d*500), trans*v(0,height,s), trans*v(d*-500,height,d*500))
	zbias(0)
end

define_model('biodomes', {
	info = {
		bounding_radius=1000,
		materials={'base','green','dome'},
		tags = {'city_building'},
		},
	static = function(lod)
		set_material('base', .4,.4,.5,1, .5,.5,.7,40)
		set_material('green', .1,.6,.1,1)
		set_material('dome', .5,.5,1,.3, 1,1,1,100)
		m = Matrix.translate(v(0,0,0))
		biodome(lod, m)
	--	m = Matrix.scale(v(0.5,0.5,0.5)) * Matrix.translate(v(600,0,0))
	--	biodome(lod, m)

		use_material("base")
	end
})

