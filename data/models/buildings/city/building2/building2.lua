-- Copyright Â© 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--[[
define_model('octaeder1', {
	info = {
			bounding_radius = 20,
			materials={'wins'}
		},
	static = function(lod)

		local  v0 = v(3,7,3)
		local  v2 = v(3,7,-3)
		local  v4 = v(3,5,7)
		local  v6 = v(7,5,3)
		local  v8 = v(7,5,-3)
		local v10 = v(3,5,-7)
		local v12 = v(3,2,7)
		local v14 = v(7,2,3)
		local v16 = v(7,2,-3)
		local v18 = v(3,2,-7)
		local v20 = v(3,0,3)
		local v22 = v(3,0,-3)
		local  v1 = v(-3,7,3)
		local  v3 = v(-3,7,-3)
		local  v5 = v(-3,5,7)
		local v11 = v(-3,5,-7)
		local v13 = v(-3,2,7)
		local v19 = v(-3,2,-7)
		local v21 = v(-3,0,3)
		local v23 = v(-3,0,-3)
		local v24 = v(2,4.7,-7)
		local v25 = v(-2,4.7,-7)
		local v26 = v(-2,3.5,-7)
		local v27 = v(2,3.5,-7)

		quad(v1,v0,v2,v3) -- all 90° quads
		xref_quad(v8,v6,v14,v16)
		quad(v11,v10,v18,v19)
		quad(v4,v5,v13,v12)
		xref_quad(v6,v4,v12,v14)
		xref_quad(v10,v8,v16,v18)

		xref_quad(v2,v0,v6,v8) -- all 22.5° quads
		quad(v3,v2,v10,v11)
		quad(v0,v1,v5,v4)
		quad(v19,v18,v22,v23)
		quad(v12,v13,v21,v20)
		xref_quad(v16,v14,v20,v22)

		xref_tri(v0,v4,v6) -- all tris
		xref_tri(v2,v8,v10)
		xref_tri(v20,v14,v12)
		xref_tri(v22,v18,v16)

		set_material('wins', .7, .8, .9, 1, .7, .8 , .9, 100) -- light blue for the windows, so far
		use_material('wins')
		extrusion( v(0,4.7,0), v(0,3.5,0), v(0,0,-1), 1.0,
			v(2,-7,0), v(-2,-7,0), v(-2,-7.05,0), v(2,-7.05,0)) -- makes that thing looking like somekind of window

		extrusion( v(0,4.7,0), v(0,3.5,0), v(0,0,1), 1.0,
			v(2,-7,0), v(-2,-7,0), v(-2,-7.05,0), v(2,-7.05,0))

		extrusion( v(0,4.7,0), v(0,3.5,0), v(1,0,0), 1.0,
			v(2,-7,0), v(-2,-7,0), v(-2,-7.05,0), v(2,-7.05,0))



end
})

define_model('building2', {
	info = {
   			bounding_radius = 60,
			materials={'mat1', 'mat2', 'mat3'},
            tags = {'city_building', 'city_power', 'city_starport_building'},
			},

	static = function(lod)

	set_material('mat1', .5, .5, .3, 1, .6, .6, .6, 0)
	use_material('mat1')
	call_model('octaeder1', v(0,0,0), v(1,0,0), v(0,1,0), 1)

	set_material('mat2', .4, .6, .5, 1, .5, .5, .5, 0)
	use_material('mat2')
	call_model('octaeder1', v(-10,0,10), v(1,0,0), v(0,1,0), 1)

	set_material('mat3', .5, .4, .5, 1, .5, .5, .5, 0)
	use_material('mat3')
	call_model('octaeder1', v(14,0,0), v(-1,0,0), v(0,1,0), 1)
end
})

-- merci descartez
--]]
