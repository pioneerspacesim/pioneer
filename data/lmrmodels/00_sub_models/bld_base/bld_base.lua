-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('bld_base_1', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			materials = {'concrete'},
            },
	static = function(lod)

        local v0 = v(3,0,10)
		local v1 = v(-3,0,10)
		local v2 = v(7,0,3)
        local v4 = v(7,0,-3)
		local v6 = v(3,0,-7)
		local v7 = v(-3,0,-7)
		local v10 = v(3,-40,10)
		local v11 = v(-3,-40,10)
		local v12 = v(7,-40,3)
		local v14 = v(7,-40,-3)
		local v16 = v(3,-40,-7)
		local v17 = v(-3,-40,-7)

		--set_material('concrete',.6,.6,.5,1,.3,.3,.3,10)
        --use_material('concrete')
		if lod > 3 then
		    texture('conc.png', v(.5,.5,0),v(.05,0,0),v(0,0,1))
		elseif lod > 1 then
            texture('conc_s.png', v(.5,.5,0),v(.05,0,0),v(0,0,1))
		end
        xref_quad(v0,v2,v4,v6)
		quad(v0,v6,v7,v1)

		if lod > 3 then
		    texture('conc.png', v(.5,.5,0),v(.1,0,0),v(0,.1,0))
		elseif lod > 1 then
            texture('conc_s.png', v(.5,.5,0),v(.1,0,0),v(0,.1,0))
		end
		quad(v0,v1,v11,v10)
		quad(v6,v16,v17,v7)

		if lod > 3 then
		    texture('conc.png', v(.5,.5,0),v(0,0,1),v(0,.1,0))
		elseif lod > 1 then
            texture('conc_s.png', v(.5,.5,0),v(0,0,1),v(0,.1,0))
		end
        xref_quad(v0,v10,v12,v2)
		xref_quad(v2,v12,v14,v4)
		xref_quad(v4,v14,v16,v6)
	end
})

define_model('bld_base_fce', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			materials = {'glow'},
   	        },
	static = function(lod)
        local v0 = v(3,0,10)
		local v1 = v(-3,0,10)
		local v2 = v(7,0,3)
        local v4 = v(7,0,-3)
		local v6 = v(3,0,-7)
		local v7 = v(-3,0,-7)

        local v20 = v(3,1,10)
		local v21 = v(-3,1,10)
		local v22 = v(7,1,3)
        local v24 = v(7,1,-3)
		local v26 = v(3,1,-7)
		local v27 = v(-3,1,-7)

		--set_material('glow',.5,.5,.5,.5,0,0.0,0,0,1,.8,1.4)
		--use_material('glow')
		if lod > 1 then
	    	if lod > 3 then
			   	texture('fence_glow.png', v(.5,.3,0),v(.1,0,0),v(0,1,0))
			else
		        texture('fence_glow_s.png', v(.5,.3,0),v(.1,0,0),v(0,1,0))
			end
            quad(v20,v0,v1,v21)
			quad(v26,v27,v7,v6)

			if lod > 3 then
			    texture('fence_glow.png', v(.5,.3,0),v(0,0,.25),v(0,1,0))
			else
	            texture('fence_glow_s.png', v(.5,.3,0),v(0,0,.25),v(0,1,0))
			end
            xref_quad(v0,v20,v22,v2)
			xref_quad(v2,v22,v24,v4)
			xref_quad(v4,v24,v26,v6)

            if lod > 3 then
				texture('fence_glow.png', v(.5,.3,0),v(.1,0,0),v(0,1,0))
			else
	            texture('fence_glow_s.png', v(.5,.3,0),v(.1,0,0),v(0,1,0))
			end

			quad(v20,v21,v1,v0)
			quad(v26,v6,v7,v27)

			if lod > 3 then
			    texture('fence_glow.png', v(.5,.3,0),v(0,0,.25),v(0,1,0))
			else
	            texture('fence_glow_s.png', v(.5,.3,0),v(0,0,.25),v(0,1,0))
			end

			xref_quad(v0,v2,v22,v20)
			xref_quad(v2,v4,v24,v22)
			xref_quad(v4,v6,v26,v24)

		end
	end
})

