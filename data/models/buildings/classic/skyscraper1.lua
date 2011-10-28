--[[
--skyscraper with a slanted triangle roof
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
--]]
