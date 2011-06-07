
function bld_base_1(lod,scale)

        local v0 = v(3,0,10)*scale
		local v1 = v(-3,0,10)*scale
		local v2 = v(7,0,3)*scale
        local v4 = v(7,0,-3)*scale
		local v6 = v(3,0,-7)*scale
		local v7 = v(-3,0,-7)*scale
		local v10 = v(3,-50,10)*scale
		local v11 = v(-3,-50,10)*scale
		local v12 = v(7,-50,3)*scale
		local v14 = v(7,-50,-3)*scale
		local v16 = v(3,-50,-7)*scale
		local v17 = v(-3,-50,-7)*scale

        local v20 = v(3,1,10)*scale
		local v21 = v(-3,1,10)*scale
		local v22 = v(7,1,3)*scale
        local v24 = v(7,1,-3)*scale
		local v26 = v(3,1,-7)*scale
		local v27 = v(-3,1,-7)*scale

        if lod > 1 then
        	use_material('concrete')
            texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,0,1)/scale)
    	end
        xref_quad(v0,v2,v4,v6)
		quad(v0,v6,v7,v1)

		if lod > 1 then
		    texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,.05,0)/scale)
  		end
		quad(v0,v1,v11,v10)
		quad(v6,v16,v17,v7)

		if lod > 1 then
		    texture('conc.png', v(.5,.5,0),v(0,0,1/scale),v(0,.05,0)/scale)
  		end
		xref_quad(v0,v10,v12,v2)
		xref_quad(v2,v12,v14,v4)
		xref_quad(v4,v14,v16,v6)

		if lod > 1 then
			use_material('fce_glow')
				
		   	texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		    quad(v20,v0,v1,v21)
			quad(v26,v27,v7,v6)

		    texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		    xref_quad(v0,v20,v22,v2)
			xref_quad(v2,v22,v24,v4)
			xref_quad(v4,v24,v26,v6)

        	texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
			quad(v20,v21,v1,v0)
			quad(v26,v6,v7,v27)

		    texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		    xref_quad(v0,v2,v22,v20)
			xref_quad(v2,v4,v24,v22)
			xref_quad(v4,v6,v26,v24)
		end
end

function bld_base_2(lod,scale)

		local v0 = v(3,0,7)*scale
		local v1 = v(-3,0,7)*scale
		local v2 = v(7,0,3)*scale
        local v4 = v(7,0,-3)*scale
		local v6 = v(3,0,-7)*scale
		local v7 = v(-3,0,-7)*scale
		local v10 = v(3,-40,7)*scale
		local v11 = v(-3,-40,7)*scale
		local v12 = v(7,-40,3)*scale
		local v14 = v(7,-40,-3)*scale
		local v16 = v(3,-40,-7)*scale
		local v17 = v(-3,-40,-7)*scale

		local v20 = v(3,1,7)*scale
		local v21 = v(-3,1,7)*scale
		local v22 = v(7,1,3)*scale
        local v24 = v(7,1,-3)*scale
		local v26 = v(3,1,-7)*scale
		local v27 = v(-3,1,-7)*scale

		if lod > 1 then
           	use_material('concrete')
            texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,0,1)/scale)
		end
        xref_quad(v0,v2,v4,v6)
		quad(v0,v6,v7,v1)

		if lod > 1 then
			texture('conc.png', v(.5,.5,0),v(.05,0,0)/scale,v(0,.05,0)/scale)
		end
		quad(v0,v1,v11,v10)
		quad(v6,v16,v17,v7)

		if lod > 1 then
        	texture('conc.png', v(.5,.5,0),v(0,0,1)/scale,v(0,.05,0)/scale)
		end
        xref_quad(v0,v10,v12,v2)
		xref_quad(v2,v12,v14,v4)
		xref_quad(v4,v14,v16,v6)

		if lod > 1 then
			use_material('fce_glow')
			texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
		    quad(v20,v0,v1,v21)
			quad(v26,v27,v7,v6)

		    texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
		    xref_quad(v0,v20,v22,v2)
			xref_quad(v2,v22,v24,v4)
			xref_quad(v4,v24,v26,v6)

        	texture('fence_glow.png', v(.5,.28,0),v(.1,0,0)/scale,v(0,1,0)/scale)
			quad(v20,v21,v1,v0)
			quad(v26,v6,v7,v27)

		    texture('fence_glow.png', v(.5,.28,0),v(0,0,.25)/scale,v(0,1,0)/scale)
			xref_quad(v0,v2,v22,v20)
			xref_quad(v2,v4,v24,v22)
			xref_quad(v4,v6,v26,v24)
		end
end

function combo_sub(lod,type,r,g,b,a,sr,sg,sb,sl)

        set_material('default',r,g,b,a,sr,sg,sb,sl)

		set_material('fce_glow', 0,0,0,.9,0,0,0,0,1.5,2,.7)
        set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
		set_material('win_on', .2,.33,.35,1,1.5,1.8,2,100,1.6,1.8,1.8)
    	set_material('win_off', .2,.33,.35,1,1.5,1.8,2,100)
        set_material('win', .2,.33,.35,.4,1.5,1.8,2,100)
        set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		set_material('trees', .7,.8,.6,.9,.5,.3,.3,1)
        set_material('grass', .3,.6,.4,1,.5,.3,.3,1)

  		if lod > 1 then
		    use_material('default')
        	texture('alu.png')
		end
		load_obj('combo_0.obj')
		
		if lod > 1 then
   			use_material('cutout')
   			texture('door.png',v(.5,.9,0),v(.5,0,0),v(0,-.5,0)) -- front
            zbias(1,v(0,1.2,7),v(0,0,1))
			circle(4,v(0,1.2,7),v(0,0,1),v(1,0,0),1)
			zbias(0)
			
   			texture('door.png',v(.5,.88,0),v(.625,0,0),v(0,-.625,0)) -- front
   			zbias(1,v(0,5.8,3),v(0,0,1))
			circle(4,v(0,5.8,3),v(0,0,1),v(1,0,0),.8)
			zbias(0)
			
            texture('alu.png')
            use_material('win_on')
			load_obj('combo_wins_on.obj')
			use_material('win_off')
			load_obj('combo_wins_off.obj')
			
			texture('rgh.png',v(.5,.5,0),v(.2,0,0),v(0,0,1.2))
			use_material('grass')
			quad(v(3,5,7),v(3,5,3),v(-3,5,3),v(-3,5,7))
			
        end

		if type == 0 then
			if lod > 1 then
				texture('tree.png')
		        use_material('trees')
				load_obj('bush_0.obj')
			end
		elseif type == 1 then
		    if lod > 1 then
   			    texture('tree.png')
                use_material('trees')
                load_obj('bush_1.obj')

				use_material('win')
				texture('win.png',v(0,0,0),v(.2,0,0),v(0,0,1))
			end
 			quad(v(3,5,7),v(3,7,3),v(-3,7,3),v(-3,5,7))
 			



   		end
end

define_model('combo_b', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'concrete', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'fce_glow'},
            tags = {'city_building'},
			},
	static = function(lod)
	    set_material('fce_glow', 0,0,0,.9,0,0,0,0,1.5,2,.7)
        set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
    	combo_sub(lod,1,.73,.7,.83,1,1.26,1.4,1.66,30)
    	bld_base_1(lod,1)
	end
})


define_model('combo_y', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'concrete', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'fce_glow'},
            tags = {'city_building'},
			},
	static = function(lod)
	    set_material('fce_glow', 0,0,0,.9,0,0,0,0,1.5,2,.7)
        set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
    	combo_sub(lod,1,1,.9,.4,1,1.26,1.4,1.66,30)
    	bld_base_1(lod,1)
	end
})

define_model('combo_g', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'concrete', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'fce_glow'},
            tags = {'city_building'},
			},
	static = function(lod)
	    set_material('fce_glow', 0,0,0,.9,0,0,0,0,1.5,2,.7)
        set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
    	combo_sub(lod,1,.7,1,.7,1,1.26,1.4,1.66,30)
    	bld_base_1(lod,1)
	end
})

define_model('combo_twin', {
	info = {                                                                                 
	        lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
            tags = {'city_building'},
			},
	static = function(lod)
    		call_model('combo_y', v(-7,1,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g', v(7,1,0),v(-1,0,0),v(0,1,0),1)
	end
})

define_model('combo_tri', {
	info = {
	        lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'concrete','fce_glow'},
            tags = {'city_building'},
			},
	static = function(lod)
	        set_material('fce_glow', 0,0,0,.9,0,0,0,0,1.5,2,.7)
        	set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
            call_model('combo_b', v(14,2,-14),v(-1,0,0),v(0,1,0),1)
			call_model('combo_y', v(14,2,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g', v(0,2,-14),v(-1,0,0),v(0,1,0),1)
			bld_base_1(lod,1)
	end
})

define_model('combo_b_i', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
			},
	static = function(lod)
    	combo_sub(lod,0,.73,.7,.83,1,1.26,1.4,1.66,30)
	end
})


define_model('combo_y_i', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
			},
	static = function(lod)
    	combo_sub(lod,0,1,.9,.4,1,1.26,1.4,1.66,30)
	end
})

define_model('combo_g_i', {
	info = {
			lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
   			materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
			},
	static = function(lod)
    	combo_sub(lod,0,.7,1,.7,1,1.26,1.4,1.66,30)
	end
})

define_model('combo_twin_i', {
	info = {
	        lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			},
	static = function(lod)
    		call_model('combo_y_i', v(-7,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g_i', v(7,0,0),v(-1,0,0),v(0,1,0),1)
	end
})

define_model('combo_tri_i', {
	info = {
	        lod_pixels = {.1,10,50,0},
			bounding_radius = 10,
			},
	static = function(lod)
            call_model('combo_b_i', v(14,0,-14),v(-1,0,0),v(0,1,0),1)
			call_model('combo_y_i', v(14,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g_i', v(0,0,-14),v(-1,0,0),v(0,1,0),1)
	end
})


define_model('f3k_top_0', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 10,
			},
	static = function(lod)

		local v20 = v(4.5,4.5,7)
		local v21 = v(-4.5,4.5,7)
		local v22 = v(10,4.5,3)
		local v24 = v(10,4.5,-3)
		local v26 = v(4.5,4.5,-7)
		local v27 = v(-4.5,4.5,-7)
		local v30 = v(4.5,7.5,3)
		local v31 = v(-4.5,7.5,3)
		local v32 = v(4.5,7.5,-3)
		local v33 = v(-4.5,7.5,-3)

		quad(v20,v30,v31,v21)
		quad(v26,v27,v33,v32)

		quad(v30,v32,v33,v31)
		xref_tri(v20,v22,v30)
		xref_tri(v24,v26,v32)
		xref_quad(v32,v30,v22,v24)

	end
})

define_model('f3k_top_0', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 10,
			},
	static = function(lod)

		local v20 = v(4.5,4.5,7)
		local v21 = v(-4.5,4.5,7)
		local v22 = v(10,4.5,3)
		local v24 = v(10,4.5,-3)
		local v26 = v(4.5,4.5,-7)
		local v27 = v(-4.5,4.5,-7)
		local v30 = v(4.5,7.5,3)
		local v31 = v(-4.5,7.5,3)
		local v32 = v(4.5,7.5,-3)
		local v33 = v(-4.5,7.5,-3)

		quad(v20,v30,v31,v21)
		quad(v26,v27,v33,v32)

		quad(v30,v32,v33,v31)
		xref_tri(v20,v22,v30)
		xref_tri(v24,v26,v32)
		xref_quad(v32,v30,v22,v24)

	end
})

define_model('f3k_top_1', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 10,
			},
	static = function(lod)

		local v20 = v(4.5,4.5,7)
		local v21 = v(-4.5,4.5,7)
		local v22 = v(10,4.5,3)
		local v24 = v(10,4.5,-3)
		local v26 = v(4.5,4.5,-7)
		local v27 = v(-4.5,4.5,-7)
		local v28 = v(4.5,4.7,3)
		local v29 = v(-4.5,4.7,3)
		local v30 = v(4.5,7.5,3)
		local v31 = v(-4.5,7.5,3)
		local v32 = v(4.5,7.5,-3)
		local v33 = v(-4.5,7.5,-3)
		local v34 = v(4.5,4.7,6.75)
		local v35 = v(-4.5,4.7,6.75)
		local v36 = v(4.5,4.7,-6.75)
		local v37 = v(-4.5,4.7,-6.75)
		local v38 = v(4.5,4.7,-3)
		local v39 = v(-4.5,4.7,-3)
		local v48 = v(4.5,4.7,-3)

        if lod > 3 then
			    texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top
			elseif lod > 1 then
			    texture('pan0_s.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))
		end

		quad(v30,v32,v33,v31) -- top
		xref_tri(v20,v22,v30) -- sides
		xref_tri(v24,v26,v32)
		xref_quad(v32,v30,v22,v24)

		if lod > 3 then
			    texture('pan0.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side
			elseif lod > 1 then
                texture('pan0_s.png', v(.5,.5,0),v(0,0,1),v(0,.1,0))
		end
		xref_tri(v32,v36,v48)
		xref_tri(v28,v34,v30)

		if lod > 3 then
			    texture('pan0.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0)) -- from front
			elseif lod > 1 then
                texture('pan0_s.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0))
		end
		quad(v32,v38,v39,v33)
		quad(v30,v31,v29,v28)


	end
})

define_model('f3k_body', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 10,
			materials={'cutout', 'inner'},
			},
	static = function(lod)
        set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
  		set_material('inner', .2,.2,.2,1,.2,.2,.2,10)
     	--set_material('win', .2,.33,.35,1,1.5,1.8,2,100) -- .2,.33,.35,.4,1.5,1.8,2,100)

		local v0 = v(4.5,-.5,10)
		local v1 = v(-4.5,-.5,10)
		local v2 = v(10 ,-.5,3)

		local v4 = v(10,-.5,-3)
		local v6 = v(5,-.5,-7)
		local v7 = v(-5,-.5,-7)
		local v10 = v(4.5,-20,10)
		local v11 = v(-4.5,-20,10)
		local v12 = v(10,-20,3)
		local v14 = v(10,-20,-3)
		local v16 = v(5,-20,-7)
		local v17 = v(-5,-20,-7)

		local v20 = v(4.5,4.5,7)
		local v21 = v(-4.5,4.5,7)
		local v22 = v(10,4.5,3)
		local v24 = v(10,4.5,-3)
		local v26 = v(4.5,4.5,-7)
		local v27 = v(-4.5,4.5,-7)
		local v28 = v(4.5,4.7,3)
		local v29 = v(-4.5,4.7,3)
		local v30 = v(4.5,7.5,3)
		local v31 = v(-4.5,7.5,3)
		local v32 = v(4.5,7.5,-3)
		local v33 = v(-4.5,7.5,-3)
		local v34 = v(4.5,4.7,6.733)
		local v35 = v(-4.5,4.7,6.733)
		local v36 = v(4.5,4.7,-6.733)
		local v37 = v(-4.5,4.7,-6.733)
		local v38 = v(4.5,4.7,-3)
		local v39 = v(-4.5,4.7,-3)

  		local v40 = v(4.5,2,7)
		local v41 = v(-4.5,2,7)
		local v42 = v(10,2,3)
		local v44 = v(10,2,-3)
		local v45 = v(-10,2,-3)
		local v46 = v(4.5,2,-7)
		local v47 = v(-4.5,2,-7)
		local v48 = v(4.5,4.7,-3)

		local v50 = v(4.5,-.5,3)
		local v51 = v(-4.5,-.5,3)
		local v52 = v(4.5,-.5,-3)
		local v53 = v(-4.5,-.5,-3)

		local v60 = v(4,4.5,6)
		local v61 = v(-4,4.5,6)
		local v62 = v(9,4.5,2.5)
		local v64 = v(9,4.5,-2.5)
		local v66 = v(4,4.5,-6)
		local v67 = v(-4,4.5,-6)
		local v70 = v(4,2,6)
		local v71 = v(-4,2,6)
		local v72 = v(9,2,2.5)
		local v74 = v(9,2,-2.5)
		local v76 = v(4,2,-6)
		local v77 = v(-4,2,-6)

        local v80 = v(9.633,4.7,3)
        local v81 = v(-9.633,4.7,3)
		local v82 = v(9.633,4.7,-3)
		local v83 = v(-9.633,4.7,-3)

		--body

        if lod > 3 then
			    texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top
			elseif lod > 1 then
			    texture('pan0_s.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))
		end

		quad(v34,v36,v37,v35) -- table
		xref_quad(v34,v80,v82,v36)

		quad(v36,v26,v27,v37) -- borders
		quad(v20,v34,v35,v21)
		xref_quad(v20,v22,v80,v34)
		xref_quad(v22,v24,v82,v80)
		xref_quad(v24,v26,v36,v82)

		quad(v46,v52,v53,v47)
		quad(v50,v40,v41,v51)
		xref_tri(v40,v50,v42)
		xref_tri(v44,v52,v46)
		xref_quad(v42,v50,v52,v44)

        quad(v60,v20,v21,v61)
        quad(v26,v66,v67,v27)
        xref_quad(v60,v62,v22,v20)
        xref_quad(v62,v64,v24,v22)
        xref_quad(v64,v66,v26,v24)

        quad(v70,v71,v41,v40)
        quad(v46,v47,v77,v76)
        xref_quad(v70,v40,v42,v72)
        xref_quad(v72,v42,v44,v74)
        xref_quad(v74,v44,v46,v76)

		if lod > 3 then
			    texture('pan0.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side
			elseif lod > 1 then
                texture('pan0_s.png', v(.5,.5,0),v(0,0,1),v(0,.1,0))
		end
		--xref_tri(v32,v26,v48)
		xref_tri(v(3,-.5,3),v(3,2,7),v(3,-.5,7))

        if lod > 3 then
			    texture('pan0.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0)) -- from front
			elseif lod > 1 then
                texture('pan0_s.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0))
		end
		--quad(v32,v38,v39,v33)
		quad(v(3,2,7),v(-3,2,7),v(-3,-.5,7),v(3,-.5,7))

		-- ring
		use_material('inner')

        if lod > 3 then
			    texture('pan1.png', v(.5,.5,0),v(.1,0,0),v(0,.1,0)) -- from front
			elseif lod > 1 then
                texture('pan1_s.png', v(.5,.5,0),v(.1,0,0),v(0,.1,0))
		end
		quad(v60,v61,v71,v70)
		quad(v66,v76,v77,v67)

  		if lod > 3 then
			    texture('pan1.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side
			elseif lod > 1 then
                texture('pan1_s.png', v(.5,.5,0),v(0,0,1),v(0,.1,0))
		end

		xref_quad(v60,v70,v72,v62)
		xref_quad(v62,v72,v74,v64)
		xref_quad(v64,v74,v76,v66)

		use_material('cutout')
        if lod > 3 then
			    texture('door.png', v(.5,.87,0),v(.71,0,0),v(0,.73,0)) -- from front
			elseif lod > 1 then
                texture('door_s.png', v(.5,.87,0),v(.71,0,0),v(0,.73,0))
		end
		zbias(1,v(0,.5,7),v(0,0,1))
		circle(4,v(0,.5,7),v(0,0,1),v(1,0,0),.7)
		zbias(0)
	end
})

define_model('f3k_thing', {
	info = {
			lod_pixels = {.1,10,20,0},
			bounding_radius = 5
       		},
	static = function(lod)
	    ---[[
		if lod > 1 then
	        	if lod < 4 then
			 		texture('wtr_s.png',v(.5,.5,0), v(.2,0,0),v(0,1,0))
	                sphere_slice(4*lod,2*lod,0,.5*math.pi, Matrix.rotate(.5*math.pi,v(1,0,0)))
				end
			end

		end,
		dynamic = function(lod)
	        if lod > 3 then
				local trans = get_arg(1)*.1
	            texture('models/city3k/wtr.png',v(trans,.5,0), v(.2,0,0),v(0,1,0))
	            sphere_slice(4*lod,2*lod,0,.5*math.pi, Matrix.rotate(.5*math.pi,v(1,0,0)))
			end

	  	--]]
	  	--[[
	  	if lod > 1 then
	        	if lod < 4 then
			 		texture('wtr_s.png',v(.5,.5,0), v(.2,0,0),v(0,1,0))
	           	else
	                texture('wtr.png',v(.5,.5,0), v(.2,0,0),v(0,1,0))
	            end
	            sphere_slice(4*lod,2*lod,0,.5*math.pi, Matrix.rotate(.5*math.pi,v(1,0,0)))
		end
		--]]
	end
})

define_model('f3k_thang', {
	info = {
			lod_pixels = {.1,10,20,0},
			bounding_radius = 5,
       		},
	static = function(lod)
		---[[
		if lod > 1 then
	    	if lod < 4 then
      			texture('wtr_s.png',v(.5,.5,0), v(0,0,1),v(.2,0,0))
	            ring(3*lod,v(-7.5,-.5,4.5), v(-7.5,3.5,4.5),v(0,0,1), .9)
	            ring(3*lod,v(-6,-.5,5.5), v(-6,3.5,5.5),v(0,0,1), .9)
			end
		end
 	end,
	dynamic = function(lod)
        if lod > 3 then
			local trans = (get_arg(1)*.05)
            texture('models/city3k/wtr.png',v(math.sin(trans),math.cos(trans),0), v(.2,0,0),v(0,.2,0))
            ring(3*lod,v(-7.5,-.5,4.5), v(-7.5,3.5,4.5),v(0,0,1), .9)
		end

        if lod > 3 then
			local trans = (get_arg(1)*.05)
            texture('models/city3k/wtr.png',v(math.cos(trans),math.sin(trans),0), v(.2,0,0),v(0,.2,0))
            ring(3*lod,v(-6,-.5,5.5), v(-6,3.5,5.5),v(0,0,1), .9)
		end
	--]]
	--[[
        if lod > 1 then
	    	if lod < 4 then
      			texture('wtr_s.png',v(.5,.5,0), v(0,0,1),v(.2,0,0))
      		else
      		    texture('wtr.png',v(.5,.5,0), v(0,0,1),v(.2,0,0))
      		end
            ring(3*lod,v(-7.5,-.5,4.5), v(-7.5,3.5,4.5),v(0,0,1), .9)
	        ring(3*lod,v(-6,-.5,5.5), v(-6,3.5,5.5),v(0,0,1), .9)
        end
	--]]

	end
})

---[[
define_model('factory_3k_1', {
	info = {
			scale = 4,
			lod_pixels = {.1,50,100,0},
			bounding_radius = 10,
			materials={'cutout', 'default', 'concrete', 'glow1', 'glow2', 'win0', 'win_on', 'win_off', 'fce_glow'},
            tags = {'city_building',},
		},
	static = function(lod)
        set_material('win_off', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win_on', .2,.33,.35,.9,1.5,1.8,2,100,1,1.2,1.4)
		set_material('default', .45,.55,.6,1,.5,.5,.6,30)
  		set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		--set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,10)
		set_material('win0', .2,.33,.35,1,1.5,1.8,2,100) -- .2,.33,.35,.4,1.5,1.8,2,100)


		local v0 = v(4.5,4.5,7)
		local v1 = v(-4.5,4.5,7)
		local v2 = v(10,4.5,3)
		local v4 = v(10,4.5,-3)
		local v6 = v(4.5,4.5,-7)
		local v7 = v(-4.5,4.5,-7)
		local v8 = v(4.5,4.7,3)
		local v9 = v(-4.5,4.7,3)
		local v10 = v(4.5,7.5,3)
		local v11 = v(-4.5,7.5,3)
		local v12 = v(4.5,7.5,-3)
		local v13 = v(-4.5,7.5,-3)

		if lod > 1 then
            set_material('glow1', .3,.3,.3,1,1,1.5,2,100,1.2,1.4,.6)
			set_material('fce_glow', .3,.3,.3,.9,0,0.0,0,0,1,.8,1.4)

			if lod > 2 then
				texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top
			else
			   	texture('pan0_s.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))
			end
		end
            use_material('default')
            quad(v0,v10,v11,v1) -- optional
			call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
			zbias(1,v(0,0,0),v(0,1,0))
			call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)
			zbias(0)

  		if lod > 1 then
			use_material('default')
		    cylinder(3*lod,v(-7.5,3.5,4.5), v(-7.5,4,4.5),v(0,0,1), .9)
			cylinder(3*lod,v(-6,3.5,5.5), v(-6,4,5.5),v(0,0,1), .9)

			if lod > 3 then
				texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			else
			    texture('wtr_s.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			end
			use_material('glow1')
			xref_cylinder(3*lod,v(4,4.7,-6), v(4,7,-6), v(0,0,1), .2)
            -- advert
			if lod > 3 then
				texture('wtr.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
			else
			    texture('wtr_s.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
			end
			use_material('fce_glow')
		    xref_cylinder(3*lod,v(3.2,-1,12), v(3.2,5,12), v(0,0,1), .2)

			call_model('ad_acme_1', v(0,2,12), v(1,0,0),v(0,1,0),3)
			call_model('ad_acme_1', v(0,2,12), v(-1,0,0),v(0,1,0),3)

			zbias(1,v(0,6,-3.001),v(0,0,-1))
			call_model('ad_acme_2', v(0,6,-3.001), v(-1,0,0),v(0,1,0),1.5)
	        zbias(0)
	        -- advert end
		end

  		bld_base_1(lod,1.4,0)

		if lod > 1 then
      texture('win0.png')
	        use_material('win_on')
			load_obj('f3k_win_on.obj',Matrix.scale(v(1.011,1,.995)))
			use_material('win_off')
			load_obj('f3k_win_off.obj',Matrix.scale(v(1.011,1,.995)))
            use_material('glow1')
			call_model('f3k_thang', v(0,0,0),v(1,0,0),v(0,1,0),1)
   		end
	end
})
--]]
---[[
define_model('factory_3k_2', {
	info = {
			scale = 4,
			lod_pixels = {.1,50,100,0},
			bounding_radius = 10,
			materials={'cutout', 'default', 'concrete', 'fce_glow', 'glow2', 'win0', 'win_on', 'win_off'},
            tags = {'city_building'},
		},
	static = function(lod)
        set_material('win_off', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win_on', .2,.33,.35,.9,1.5,1.8,2,100,1,1.2,1.4)

		set_material('default', .45,.55,.6,1,.5,.5,.6,30)
  		set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		--set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,10)
		set_material('win0', .2,.33,.35,1,1.5,1.8,2,100) -- .2,.33,.35,.4,1.5,1.8,2,100)
        set_material('fce_glow', .3,.3,.3,.9,0,0,0,0,1.2,1.4,.6)
        set_material('glow2', .3,.3,.3,1,1,1.5,2,100,1,.8,1.4)

		local v0 = v(4.5,4.5,7)
		local v1 = v(-4.5,4.5,7)

		local v10 = v(4.5,7.5,3)
		local v11 = v(-4.5,7.5,3)


		if lod > 1 then



			use_material('default')
		    cylinder(3*lod,v(-7.5,3.5,4.5), v(-7.5,4,4.5),v(0,0,1), .9)
			cylinder(3*lod,v(-6,3.5,5.5), v(-6,4,5.5),v(0,0,1), .9)

			if lod > 3 then
				texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			else
			    texture('wtr_s.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			end
			use_material('glow2')
			xref_cylinder(3*lod,v(4,4.7,-6), v(4,7,-6), v(0,0,1), .2)

			if lod > 2 then
				texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top
			else
			   	texture('pan0_s.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))
			end

		    use_material('default')
        end
		quad(v0,v10,v11,v1) -- optional

		call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
		zbias(1,v(0,0,0),v(0,1,0))
		call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)
		zbias(0)

  		if lod > 1 then
        	--[[
			if lod > 3 then
				texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,0,1))
			else
			    texture('wtr_s.png',v(.5,.5,0), v(.5,0,0),v(0,0,1))
			end
			use_material('glow2')
			sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(3.5,4.8,-6))*Matrix.scale(v(.6,.2,.6)))
            sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(-3.5,4.8,-6))*Matrix.scale(v(.6,.2,.6)))
            --]]
            -- advert
            if lod > 3 then
				texture('wtr.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
			else
			    texture('wtr_s.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
			end
			use_material('glow2')
		    xref_cylinder(3*lod,v(3.2,-1,12), v(3.2,5,12), v(0,0,1), .2)

			call_model('ad_sirius_1', v(0,2,12), v(1,0,0),v(0,1,0),3)
			call_model('ad_sirius_1', v(0,2,12), v(-1,0,0),v(0,1,0),3)

			zbias(1,v(0,4.7,-3.001),v(0,0,-1))
			call_model('ad_sirius_2', v(0,6,-3.001), v(-1,0,0),v(0,1,0),1.5)
	        zbias(0)
            -- advert end
		end

		bld_base_1(lod,1.4,0)
		--use_material('concrete')
		--call_model('bld_base_1', v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)

		if lod > 1 then
            --wins
			texture('win0.png')
	        use_material('win_on')
			load_obj('f3k_win_on.obj',Matrix.scale(v(1.011,1,.995)))
			use_material('win_off')
			load_obj('f3k_win_off.obj',Matrix.scale(v(1.011,1,.995)))

   	  		use_material('glow2')
			call_model('f3k_thing', v(6.3,3.3,4.3),v(1.2,0,-1),v(0,1,0),1.2)
            use_material('fce_glow')
			--call_model('f3k_thang', v(0,0,0),v(1,0,0),v(0,1,0),1)

			--use_material('glow1')
			--call_model('bld_base_fce',v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)
		end
	end,
})
--]]
--[[
define_model('factory_3k_3', {
	info = {
			scale = 2,
			lod_pixels = {.1,50,100,0},
			bounding_radius = 5,
			materials={'cutout', 'default', 'concrete', 'fce_glow', 'glow2', 'win0', 'win_on', 'win_off'},
            tags = {'city_building'},
		},
	static = function(lod)
	    set_material('win_off', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win_on', .2,.33,.35,.9,1.5,1.8,2,100,1,1.2,1.4)

		set_material('default', .45,.55,.6,1,.5,.5,.6,30)
  		set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		--set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,10)
		set_material('win0', .2,.33,.35,1,1.5,1.8,2,100) -- .2,.33,.35,.4,1.5,1.8,2,100)

		use_material('default')
		set_material('fce_glow', .3,.3,.3,.9,0,0,0,0,1.2,1.4,.6)
		set_material('glow2', .3,.3,.3,1,1,1.5,2,100,1,.8,1.4)
        if lod > 1 then


            if lod > 3 then
				texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,0,1))
			else
			    texture('wtr_s.png',v(.5,.5,0), v(.5,0,0),v(0,0,1))
			end
			use_material('fce_glow')
			sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(3.5,4.8,-6))*Matrix.scale(v(.6,.2,.6)))
            sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(-3.5,4.8,-6))*Matrix.scale(v(.6,.2,.6)))
            sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(3.5,4.8,6))*Matrix.scale(v(.6,.2,.6)))
            sphere_slice(3*lod,2*lod,0,math.pi,Matrix.translate(v(-3.5,4.8,6))*Matrix.scale(v(.6,.2,.6)))
		end

        use_material('default')
		call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
        call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)

		bld_base_1(lod,1.4,0)
		--use_material('concrete')
		--call_model('bld_base_1', v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)

		if lod > 1 then
            texture('win0.png')
	        use_material('win_on')
			load_obj('f3k_win_on.obj',Matrix.scale(v(1.011,1,.995)))
            load_obj('f3k_win4_0.obj',Matrix.scale(v(1.011,1,.995)))

			use_material('win_off')
			load_obj('f3k_win_off.obj',Matrix.scale(v(1.011,1,.995)))
            load_obj('f3k_win4_1.obj',Matrix.scale(v(1.011,1,.995)))

			--use_material('glow1')
			--call_model('bld_base_fce',v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)
		end
	end
})
--]]
--[[
define_model('factory_3k_4', {
	info = {
			scale = 2,
			lod_pixels = {.1,50,100,0},
			bounding_radius = 5,
			materials={'cutout', 'default', 'concrete', 'fce_glow', 'glow2', 'win0', 'win_on', 'win_off'},
            tags = {'city_building'},
		},
	static = function(lod)
        set_material('win_off', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win_on', .2,.33,.35,.9,1.5,1.8,2,100,1,1.2,1.4)

		set_material('default', .45,.55,.6,1,.5,.5,.6,30)
		set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		--set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,10)
		set_material('win0', .2,.33,.35,1,1.5,1.8,2,100) -- .2,.33,.35,.4,1.5,1.8,2,100)

		set_material('fce_glow', .3,.3,.3,1,1,1.5,2,100,1.2,1.4,.6)
		set_material('glow2', .3,.3,.3,.9,0,0,0,0,1,.8,1.4)

		if lod > 1 then


            if lod > 3 then
				texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			else
			    texture('wtr_s.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			end
			use_material('glow2')
			xref_cylinder(3*lod,v(4,4.7,-6), v(4,7,-6), v(0,0,1), .2)
			xref_cylinder(3*lod,v(4,4.7,6), v(4,7,6), v(0,0,1), .2)
		end

		use_material('default')
		call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
		call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)

		bld_base_1(lod,1.4,0)
		--use_material('concrete')
        --call_model('bld_base_1', v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)

		if lod > 1 then
            texture('win0.png')
	        use_material('win_on')
			load_obj('f3k_win_on.obj',Matrix.scale(v(1.011,1,.995)))
            load_obj('f3k_win4_0.obj',Matrix.scale(v(1.011,1,.995)))

			use_material('win_off')
			load_obj('f3k_win_off.obj',Matrix.scale(v(1.011,1,.995)))
            load_obj('f3k_win4_1.obj',Matrix.scale(v(1.011,1,.995)))

			--use_material('glow2')
			--call_model('bld_base_fce',v(0,-.5,0),v(1,0,0),v(0,1,0),1.4)
		end
	end
})
--]]

define_model('green_bubble', {
	info = {
			scale = 1.5,
			lod_pixels = {.1,10,50,0},
			bounding_radius = 5,
			materials={'default', 'concrete', 'cutout', 'fce_glow', 'glow2'},
			tags = {'city_starport_building'},
			},
	static = function(lod)
		    --set_material('glow1',0,0,0,.9,0,0,0,0,1,1.6,1.8)
            set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
			set_material('default', .43,.5,.63,1,1.26,1.4,1.66,30)
			set_material('fce_glow',0,0,0,1,0,0,0,0,1.5,2,.7)
		    set_material('cutout',0,0,0,.999,.6,.6,.6,30)


		    local  v0 = v(4.359,0,6)
			local  v1 = v(-4.359,0,6)
			local  v2 = v(7.053,0,-2.292)
			local  v3 = v(-7.053,0,-2.292)
			local  v4 = v(0,0,-7.416)
			local v10 = v(2.18,3,3)
			local v11 = v(-2.18,3,3)
		    local v12 = v(3.527,3,-1.146)
		    local v13 = v(-3.527,3,-1.146)
		    local v14 = v(0,3,-3.708)

			if lod > 3 then
				texture('alu.png',v(.5,.5,0),v(.2,0,0),v(0,0,1))
			elseif lod > 1 then
				texture('alu_s.png',v(.5,.5,0),v(.2,0,0),v(0,0,1))
			end
		    use_material('default')

		    quad(v12,v13,v11,v10)
		    tri(v12,v14,v13)
		    quad(v0,v2,v12,v10)
		    quad(v2,v4,v14,v12)
		    quad(v4,v3,v13,v14)
		    quad(v3,v1,v11,v13)
		    quad(v1,v0,v10,v11)

			if lod > 1 then
				if lod < 3 then
				    texture('wtr_x_s.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				else
					texture('wtr_x_s.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				end
				use_material('glow2')
				sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(2.99,2.99,2.99)))

			    if lod < 3 then
					texture('bubbles_s.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
				else
				    texture('bubbles.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
				end
				use_material('cutout')
				sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(3,3,3)))
			end

			bld_base_2(lod,1.2,0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('glow2',lerp_materials(get_arg(1)*0.3, {0,0,0,1,0,0,0,0,1.6,1.9,0},
																 {0,0,0,1,0,0,0,0,1,2.5,0}))
		end
	    --[[
	    if lod > 2 then
	    	local trans = get_arg(1)*.1
			if lod > 3 then
				texture('models/city3k/wtr_x.png',v(math.sin(trans),math.cos(trans),0),v(.05,0,0),v(0,0,1))
			else
				texture('models/city3k/wtr_x_s.png',v(.5,.5,.5),v(.05,0,0),v(0,0,1))
			end
			use_material('glow2')
			sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(2.99,2.99,2.99)))

			if lod > 3 then
				texture(models/city3k/bubbles.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
			else
				texture('models/city3k/bubbles_s.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
			end
			use_material('cutout')
			sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(3,3,3)))

		end
	    --]]


	end
})
--[[
define_model('blue_bubble', {
	info = {
			scale = 1.5,
			lod_pixels = {.1,10,50,0},
			bounding_radius = 5,
			materials={'default', 'cutout', 'glow1', 'glow2'},
			tags = {'city_starport_building'},
			},
	static = function(lod)
		    --set_material('glow1',0,0,0,.9,0,0,0,0,1,1.6,1.8)
		    set_material('default', .43,.5,.63,1,1.26,1.4,1.66,30)
			set_material('glow1',0,0,0,1,0,0,0,0,1.5,2,.7)
		    set_material('cutout',0,0,0,.999,.6,.6,.6,30)


		    local  v0 = v(4.359,0,6)
			local  v1 = v(-4.359,0,6)
			local  v2 = v(7.053,0,-2.292)
			local  v3 = v(-7.053,0,-2.292)
			local  v4 = v(0,0,-7.416)
			local v10 = v(2.18,3,3)
			local v11 = v(-2.18,3,3)
		    local v12 = v(3.527,3,-1.146)
		    local v13 = v(-3.527,3,-1.146)
		    local v14 = v(0,3,-3.708)

			if lod > 3 then
				texture('alu.png',v(.5,.5,0),v(.2,0,0),v(0,0,1))
			elseif lod > 1 then
				texture('alu_s.png',v(.5,.5,0),v(.2,0,0),v(0,0,1))
			end
		    use_material('default')

		    quad(v12,v13,v11,v10)
		    tri(v12,v14,v13)
		    quad(v0,v2,v12,v10)
		    quad(v2,v4,v14,v12)
		    quad(v4,v3,v13,v14)
		    quad(v3,v1,v11,v13)
		    quad(v1,v0,v10,v11)

			if lod > 1 then
				if lod < 3 then
				    texture('wtr_x_s.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				else
					texture('wtr_x_s.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				end
				use_material('glow2')
				sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(2.99,2.99,2.99)))

			    if lod > 1 then
					texture('bubbles_s.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
				end
				use_material('cutout')
				sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(3,3,3)))
			end

			call_model('bld_base2', v(0,0,0),v(1,0,0),v(0,1,0),1.2)
			use_material('glow1')
			call_model('bld_base2_fce', v(0,.5,0),v(1,0,0),v(0,1,0),1.2)
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('glow2',lerp_materials(get_arg(1)*0.3, {0,0,0,1,0,0,0,0,2.5,1,1.5},
																 {0,0,0,1,0,0,0,0,1,1.5,2.5}))
		end

	end
})
--]]
--[[
define_model('advert_0', {
	info = {
			scale = 1,
			lod_pixels = {.1,10,50,0},
			bounding_radius = 5,
			materials={'glow1', 'glow2'},
			tags = {'city_power'},
			},
	static = function(lod)
			set_material('glow1',0,0,0,.9,0,0,0,0,1,1.6,1.8)

			call_model('bld_base2', v(0,0,0),v(1,0,0),v(0,1,0),1)


			--if lod > 1 then
				if lod < 4 then
					texture('wtr_s.png',v(.1,.5,0), v(.2,0,0),v(0,.3,0))
				else
				    texture('wtr.png',v(.1,.5,0), v(.2,0,0),v(0,.3,0))
				end
				use_material('glow2')
			    xref_cylinder(3*lod,v(6.3,0,0), v(6.3,18,0), v(0,0,1), .3)

				call_model('ad_acme_1', v(0,5,0), v(1,0,0),v(0,1,0),6)
				call_model('inteloutside', v(0,5,0), v(-1,0,0),v(0,1,0),6)

				call_model('ad_cola_1', v(0,12,0), v(1,0,0),v(0,1,0),6)
				call_model('ad_cola_1', v(0,12,0), v(-1,0,0),v(0,1,0),6)

				use_material('glow1')
				call_model('bld_base2_fce', v(0,.5,0),v(1,0,0),v(0,1,0),1)
			--end
	end,
	dynamic = function(lod)
		if lod > 1 then
			set_material('glow2',lerp_materials(get_arg(1)*2, {0,0,0,1,0,0,0,0,1.6,1.9,0},
																 {0,0,0,1,0,0,0,0,1.4,1,1.8}))
		end

	end
})
--]]

define_model('pink_obelisk', {
	info = {
			scale = .7,
			lod_pixels = {.1,10,50,0},
			bounding_radius = 5,
			materials={'default', 'concrete', 'cutout', 'fce_glow', 'glow2'},
			tags = {'city_power'},
			},
	static = function(lod)

			local  v0 = v(0,62,0)
			local  v1 = v(-3.886,60,1.665)
			local  v2 = v(1.665,60,3.886)
			local  v3 = v(-9.191,10,3.939)
			local  v4 = v(3.939,10,9.191)
			local  v5 = v(-4.6,5,1.972)
			local  v6 = v(1.972,5,4.6)
			local  v7 = v(-1.665,60,-3.886)
			local  v8 = v(3.886,60,-1.665)
			local  v9 = v(-3.939,10,-9.191)
			local v10 = v(9.191,10,-3.939)
			local v11 = v(-1.972,5,-4.6)
			local v12 = v(4.6,5,-1.972)

			if lod > 1 then
				--set_material('glow1',0,0,0,.9,0,0,0,0,1,1.6,1.8)
                set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
				set_material('default', .2,.2,.2,1,.3,.3,.3,30)
				set_material('fce_glow',0,0,0,1,0,0,0,0,2,1,2.5)
			    set_material('cutout',0,0,0,.999,.6,.6,.6,30)

				use_material('default')
				if lod > 3 then
					texture('alu_s.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
			    else
			    	texture('alu.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
			    end
			end
			--cylinder(6*lod, v(0,0,0), v(0,5,0), v(0,0,1), 10)

			if lod > 1 then
				use_material('glow2')

				if lod < 3 then
				    texture('wtr_s.png',v(.5,.5,0),v(.1,0,0),v(0,.03,0))
				else
					texture('wtr.png',v(.5,.5,0),v(.1,0,0),v(0,.03,0))
				end

			end
 			tri(v0,v1,v2)
            tri(v0,v8,v7)
            quad(v1,v3,v4,v2)
            quad(v8,v10,v9,v7)
            quad(v3,v5,v6,v4)
            quad(v10,v12,v11,v9)

			if lod > 1 then

            	if lod < 3 then
				    texture('wtr_s.png',v(.5,.5,0),v(0,0,-2),v(0,.03,0))
				else
					texture('wtr.png',v(.5,.5,0),v(0,0,-2),v(0,.03,0))
				end

			end
			tri(v0,v2,v8)
			tri(v0,v7,v1)
			quad(v2,v4,v10,v8)
			quad(v7,v9,v3,v1)
			quad(v4,v6,v12,v10)
			quad(v9,v11,v5,v3)


			bld_base_2(lod,1.8,0)
	end,

	dynamic = function(lod)
		if lod > 1 then
			set_material('glow2',lerp_materials(get_arg(1)*0.1, {0,0,0,1,1,1,1,100,2.2,1,1.5},
																 {0,0,0,1,1,1,1,100,1.5,1,2.2}))
		end

	end
})


define_model('church_3k_clockhd', {
    info = {
			lod_pixels = {1,20,50,0},
			bounding_radius = 30,
   			},
	static = function(lod)
        load_obj('church_new_pyr.obj',Matrix.rotate(.5*math.pi,v(-1,0,0)))
	end
})

define_model('church_3k_0', {
	info = {
			lod_pixels = {.1,20,50,0},
			bounding_radius = 30,
			materials={'default', 'glow', 'cutout', 'hour', 'min'},
            },
	static = function(lod)
		if lod > 1 then
			set_material('default', .5,.5,.45,1,.5,.5,.6,10)
	        set_material('cutout', .65,.6,.55,.9,.55,.5,.5,10)
	        set_material('glow', .5,.5,.5,1,1,1.5,2,100,.6,1.2,1.2)
	        set_material('hour', .7,.1,.1,1,.3,.3,.3,30)
			set_material('min', .2,.5,.6,1,.3,.3,.3,30)

			use_material('default')
         	texture('conc.png')
            load_obj('church_new_0.obj')

			use_material('glow')
     		texture('glow.png')
            load_obj('church_new_win.obj')

            use_material('cutout')
			texture('door.png',v(.5,.13,0),v(.445,0,0),v(0,.52,0))
            zbias(1,v(0,1.2,16.897),v(0,0,1))
			circle(4,v(0,1.2,16.897),v(0,0,1),v(1,0,0),1)
			zbias(0)
			--[[ -- moved to arco01_rot
			if lod > 3 then
	           	texture('church_new_clock.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			elseif lod > 2 then
			    texture('church_new_clock_m.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			else
			    texture('church_new_clock_s.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			end
   			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(-8,-7,-8)))
      		sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,34,0)) * Matrix.scale(v(8.01,7.01,8.01)))
			--]]
		end
	end,
	
	dynamic = function(lod)

		local minutePos = -2*math.pi * get_arg(3)
        local hourPos = minutePos / 12
        
		use_material('min')
		zbias(1,v(0,34,0),v(0,1,0))
        call_model('church_3k_clockhd', v(0,35,0),v(math.cos(minutePos),0,math.sin(minutePos)),v(math.cos(minutePos+math.pi*0.5),0, math.sin(minutePos+math.pi*0.5)), 1.5)

        use_material('hour')
        zbias(2,v(0,34,0),v(0,1,0))
		call_model('church_3k_clockhd', v(0,35,0),v(math.cos(hourPos),0,math.sin(hourPos)),v(math.cos(hourPos+math.pi*0.5),0, math.sin(hourPos+math.pi*0.5)), 1.5)
	    zbias(0)
		--[[
     	local move = .02*get_arg(1)

		if lod > 2 then
			texture('models/church_new_0/glow.png',v(move,-move,0), v(.02,0,0),v(0,0,-1))
		else
		    texture('models/church_new_0/glow_s.png',v(move,-move,0), v(.02,0,0),v(0,0,-1))
		end
		--]]

	end
})
--[[
define_model('arco_lift_0', {
	info = {
   			lod_pixels = {.1,5,20,0},
			bounding_radius = 10,
			materials = {'steel', 'cutout'},
			},
    static = function(lod)
			set_material('steel', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('steel')
			sphere_slice(3*lod,2*lod,0,.5*math.pi, Matrix.scale(v(.5,1.5,.5)) * Matrix.rotate(.5*math.pi,v(1,0,0)))
		
	end
})

function arco_lift_1(lod)

	--if lod > 1 then
		--if math.fmod(get_arg(1)/2,1) ~= 0 then
		    --math.randomseed(os.clock*100)
			--local floor = math.random(1,20)*10
			--local anim = math.fmod(get_arg(1)*4,1)*floor

			call_model('arco_lift_0',v(55,100,55),v(1,0,-1),v(0,1,0),20)
	    --end
	
	--end




end

--]]





define_model('arco01_rot', {
	info = {
			scale = 1,
			lod_pixels = {.1,100,200,0},
			bounding_radius = 500,
			materials = {'grass', 'steel', 'gravel', 'metal', 'chrome', 'lake', 'wins_on', 'wins_off', 'dome', 'cutout'},
			},
			
	static = function(lod)
		if lod > 1 then
            set_material('steel', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('steel')
            	if lod > 3 then
					texture('bot5.png',v(.5,.5,0),v(.0019,0,0),v(0,0,1))
				elseif lod > 2 then
	                texture('bot5_m.png',v(.5,.5,0),v(.0019,0,0),v(0,0,1))
				else
	       			texture('bot5_s.png',v(.5,.5,0),v(.0019,0,0),v(0,0,1))
				end
		end
		tube(16*lod,v(0,480,0),v(0,520,0),v(1,0,0),230,250)

		if lod > 1 then
		    set_material('cutout', .65,.6,.55,.9,.55,.5,.5,10)
		    set_material('wins_on', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
            set_material('wins_off', .2,.33,.35,.9,1.5,1.8,2,100)
            set_material('lake', .4,.5,.5,1,.3,.4,.5,50)
            set_material('gravel', .3,.31,.3,1,.3,.3,.3,5)
			set_material('grass', .2,.3,0,1,.3,.3,.3,5)
            set_material('metal', .5,.55,.55,1,.35,.38,.4,10)

            use_material('grass')
			if lod > 2 then
				texture('rgh.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
			else
			    texture('rgh_s.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
			end
			circle(16,v(0,515,0),v(0,1,0),v(1,0,0),245)

	        zbias(1,v(0,515,0), v(0,1,0))
			cubic_bezier_quad(lod,lod,
			        v(-20,514,60), v(-60,514,80), v(-120,514,80), v(-160,514,90),
					v(0,514,100), v(-60,600,100), v(-120,514,100), v(-160,514,100),
					v(0,514,120), v(-60,514,120), v(-120,530,120), v(-160,514,120),
					v(-20,514,160), v(-60,514,180), v(-120,514,180), v(-160,514,160))

			use_material('gravel')
			if lod > 2 then
				texture('grav.png')
			else
			    texture('grav_s.png')
			end

			quad(v(247,515,5), v(247,515,-5), v(-247,515,-5), v(-247,515,5))
			quad(v(5,515,247), v(5,515,-247), v(-5,515,-247), v(-5,515,247))

			circle(4*lod,v(0,515,0),v(0,1,0),v(0,0,1),40)
			zbias(0)

            zbias(1,v(0,500,0), v(0,1,0))
   			texture('win0.png')
            use_material('wins_on')
			load_obj('wins_on.obj',Matrix.scale(v(10.02,10,10.02)))
			
			use_material('wins_off')
			load_obj('wins_off.obj',Matrix.scale(v(10.02,10,10.02)))


			---[[
            texture('wtr.png',v(.5,.5,0), v(.015,0,0),v(0,0,-1))
			use_material('lake')
			zbias(1,v(0,515,0),v(0,1,0))
            	flat(3*lod,v(0,1,0),{v(-40,515,80), v(-50,515,40)}, {v(-60,515,-10),v(-100,515,60),v(-120,515,50)},{v(-140,515,50), v(-140,515,70)}, {v(-140,515,80),v(-120,515,80)})
			zbias(0)

			--]]
			---[[
			call_model('combo_twin_i', v(-40,515,180), v(1,0,-1),v(0,1,0),1.25)
            call_model('combo_twin_i', v(60,515,180), v(1,0,-1),v(0,1,0),1.25)
            
			call_model('combo_tri_i', v(40,515,50), v(1,0,0),v(0,1,0),1.25)

			call_model('combo_tri_i', v(-130,515,55), v(0,0,-1),v(0,1,0),1.25)

			call_model('combo_twin_i', v(120,515,-100), v(1,0,1),v(0,1,0),1.25)
			call_model('combo_twin_i', v(60,515,-180), v(1,0,1),v(0,1,0),1.25)
			
			call_model('combo_tri_i', v(30,515,100), v(0,0,1),v(0,1,0),1.25)
			
   			call_model('combo_tri_i', v(140,515,-50), v(0,0,1),v(0,1,0),1.25)
   			call_model('combo_tri_i', v(80,515,-30), v(0,0,-1),v(0,1,0),1.25)
   			
   			call_model('combo_twin_i', v(-45,515,-155), v(1,0,0),v(0,1,0),1.25)
   			call_model('combo_twin_i', v(-25,515,-110), v(0,0,-1),v(0,1,0),1.25)
   			call_model('combo_twin_i', v(-25,515,-60), v(0,0,-1),v(0,1,0),1.25)
			
			call_model('combo_twin_i', v(-170,515,90), v(-1,0,-1),v(0,1,0),1.25)
            
			call_model('combo_tri_i', v(100,515,80), v(-1,0,0),v(0,1,0),1.25)
            call_model('combo_tri_i', v(180,515,100), v(-1,0,0),v(0,1,0),1.25)
			
			call_model('combo_tri_i', v(-200,515,-30), v(1,0,0),v(0,1,0),1.25)
            call_model('combo_tri_i', v(-180,515,-80), v(1,0,0),v(0,1,0),1.25)
            --]]
			call_model('woods_0',v(60,515,-120), v(.5,0,-1),v(0,1,0),1.25)
			call_model('woods_1',v(160,515,-40), v(1,0,0),v(0,1,0),1.25)

			call_model('woods_0',v(140,515,80), v(0,0,1),v(0,1,0),1.25)
			call_model('woods_1',v(50,515,50), v(-1,0,1),v(0,1,0),1.25)

			call_model('woods_1',v(10,515,180), v(1,0,.2),v(0,1,0),1.25)
            call_model('woods_0',v(-80,515,110), v(-1,0,0),v(0,1,0),1.25)
			call_model('woods_1',v(-145,515,40), v(-.4,0,-1),v(0,1,0),1.25)

			call_model('woods_0',v(-80,515,-50), v(1,0,0),v(0,1,0),1.25)
			call_model('woods_1',v(-40,515,-160), v(1,0,0),v(0,1,0),1.25)
			
            call_model('church_3k_0',v(0,515,0), v(1,0,0),v(0,1,0),1.5)
            
            -- import from church_3k_0
            use_material('cutout')
			if lod > 3 then
	           	texture('church_new_clock.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			elseif lod > 2 then
			    texture('church_new_clock_m.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			else
			    texture('church_new_clock_s.png',v(.5,.5,0), v(.05,0,0),v(0,0,1))
			end
   			sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,566,0)) * Matrix.scale(v(-12,-10.5,-12)))
      		sphere_slice(6*lod,3*lod, 0, math.pi, Matrix.translate(v(0,566,0)) * Matrix.scale(v(12.5,10.51,12.5))) 
		end

	end
})

define_model('arco_01', {
	info = {
			scale = .8,
			lod_pixels = {.1,200,500,0},
			bounding_radius = 200,
			materials = {'grass', 'concrete', 'gravel', 'steel', 'text', 'metal', 'chrome', 'win', 'dome'},
            tags = {'city_building'},
			},
			
	static = function(lod)
        if lod > 1 then
			set_material('steel', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('dome', 0,.5,1,.3,1,1.5,2,100)
			set_material('grass', .2,.3,0,1,.3,.3,.3,5)
			set_material('concrete',.7,.7,.6,1,.3,.3,.3,5)
			set_material('gravel', .3,.31,.3,1,.3,.3,.3,5)
			set_material('metal', .5,.55,.55,1,.35,.38,.4,10)
			set_material('text', .5,.5,0,1,0,0,0,0,.5,.5,0)
	        set_material('dome', 0,.5,1,.3,1,1.5,2,100)

		    use_material('concrete')
			texture('conc.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
		end
		tapered_cylinder(16*lod,v(0,-200,0),v(0,400,0),v(1,0,0),100,50)

		if lod > 1 then
        	use_material('steel')
			if lod > 3 then

				texture('bot5.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			elseif lod > 2 then
			   	texture('bot5_m.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			else
		    	texture('bot5_s.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
	  		end
		end
		tapered_cylinder(16*lod,v(0,400,0),v(0,480,0),v(1,0,0),50,250)

		if lod > 1 then
		    use_material('steel')
			texture('pan0.png',v(.5,.5,0),v(.01,0,0),v(0,.01,0))
		end
		xref_flat(8*lod,v(0,0,1), {v(49,400,10)}, {v(65,200,10)}, {v(50,400,10), v(250,480,10)})
        xref_flat(8*lod,v(0,0,-1), {v(-49,400,-10)}, {v(-65,200,-10)}, {v(-50,400,-10), v(-250,480,-10)})

		if lod > 1 then
			texture('pan0.png',v(.5,.5,0),v(-.01,0,0),v(0,.01,0))
		end
		quadric_bezier_quad(8*lod,1,v(10,200,65), v(10,400,50), v(10,480,250),
									v(0,200,65), v(0,400,50), v(0,480,250),
									v(-10,200,65), v(-10,400,50), v(-10,480,250))

		quadric_bezier_quad(8*lod,1,v(-10,200,-65), v(-10,400,-50), v(-10,480,-250),
									v(0,200,-65), v(0,400,-50), v(0,480,-250),
									v(10,200,-65), v(10,400,-50), v(10,480,-250))

		if lod > 1 then
        	texture('pan0.png',v(.5,.5,0),v(0,0,1),v(0,.01,0))
		end
		xref_flat(8*lod,v(-1,0,0), {v(-10,400,49)}, {v(-10,200,65)}, {v(-10,400,50), v(-10,480,250)})
        xref_flat(8*lod,v(1,0,0), {v(10,400,-49)}, {v(10,200,-65)}, {v(10,400,-50), v(10,480,-250)})

		if lod > 1 then
			texture('pan0.png',v(.5,.5,0),v(0,0,-1),v(0,.01,0))
		end
		xref_quadric_bezier_quad(8*lod,1,	v(65,200,-10), v(50,400,-10), v(250,480,-10),
											v(65,200,0), v(50,400,0), v(250,480,0),
											v(65,200,10), v(50,400,10), v(250,480,10))
											
		--arco_lift_1(lod)
											

		---[[
		call_model('arco01_rot', v(0,0,0), v(1,0,0), v(0,1,0),1)
		texture(nil)
		use_material('dome')

		sphere_slice(12*lod,4*lod,0,.3*math.pi, Matrix.translate(v(0,340,0)) * Matrix.scale(v(305,305,305)))
		--]]
		--[[
		if lod == 1 then
            call_model('arco01_rot', v(0,0,0), v(1,0,0), v(0,1,0),1)
            texture(nil)
			sphere_slice(12*lod,4*lod,0,.3*math.pi, Matrix.translate(v(0,340,0)) * Matrix.scale(v(305,305,305)))
		end

  	end,
	
	dynamic = function(lod)
		if lod > 1 then
			local rot = math.pi*get_arg(3)/12
	  		call_model('arco01_rot', v(0,0,0), v(math.cos(rot),0,math.sin(rot)), v(0,1,0),1)

			texture(nil)
	        use_material('dome')
	  		sphere_slice(12*lod,4*lod,0,.3*math.pi, Matrix.translate(v(0,340,0)) * Matrix.scale(v(305,305,305)))
		end --]]
  	end

})	
