--[[
define_model('f3k_top_1', {
	info = {
		lod_pixels = {5,20,50,0},
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

		texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top

		quad(v30,v32,v33,v31) -- top
		xref_tri(v20,v22,v30) -- sides
		xref_tri(v24,v26,v32)
		xref_quad(v32,v30,v22,v24)

		texture('pan0.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side

		xref_tri(v32,v36,v48)
		xref_tri(v28,v34,v30)

		texture('pan0.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0)) -- from front
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

		texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top

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

		texture('pan0.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side
		--xref_tri(v32,v26,v48)
		xref_tri(v(3,-.5,3),v(3,2,7),v(3,-.5,7))

		texture('pan0.png', v(.42,.81,0),v(-.1,0,0),v(0,.11,0)) -- from front
		--quad(v32,v38,v39,v33)
		quad(v(3,2,7),v(-3,2,7),v(-3,-.5,7),v(3,-.5,7))

		-- ring
		use_material('inner')

		texture('pan1.png', v(.5,.5,0),v(.1,0,0),v(0,.1,0)) -- from front
		quad(v60,v61,v71,v70)
		quad(v66,v76,v77,v67)

		texture('pan1.png', v(.5,.5,0),v(0,0,1),v(0,.1,0)) -- from side

		xref_quad(v60,v70,v72,v62)
		xref_quad(v62,v72,v74,v64)
		xref_quad(v64,v74,v76,v66)

		use_material('cutout')

		texture('door.png', v(.5,.87,0),v(.71,0,0),v(0,.73,0)) -- from front
		zbias(1,v(0,.5,7),v(0,0,1))
		circle(4,v(0,.5,7),v(0,0,1),v(1,0,0),.7)
		zbias(0)
	end
})

define_model('f3k_thing', {
	info = {
			lod_pixels = {1,10},
			bounding_radius = 5
	},
	static = function(lod)
		texture('wtr.png',v(.5,.5,0), v(.2,0,0),v(0,1,0))
		sphere_slice(4*lod,2*lod,0,.5*math.pi, Matrix.rotate(.5*math.pi,v(1,0,0)))
	end,
	dynamic = function(lod)
		local trans = get_time('SECONDS')*.1
		texture('models/buildings/city3k/wtr.png',v(trans,.5,0), v(.2,0,0),v(0,1,0))
		sphere_slice(4*lod,2*lod,0,.5*math.pi, Matrix.rotate(.5*math.pi,v(1,0,0)))
	end
})

define_model('f3k_thang', {
	info = {
			lod_pixels = {.1,10,20,0},
			bounding_radius = 5,
	   		},
	static = function(lod)
		if lod > 1 then
			if lod < 4 then
	  			texture('wtr.png',v(.5,.5,0), v(0,0,1),v(.2,0,0))
				ring(3*lod,v(-7.5,-.5,4.5), v(-7.5,3.5,4.5),v(0,0,1), .9)
				ring(3*lod,v(-6,-.5,5.5), v(-6,3.5,5.5),v(0,0,1), .9)
			end
		end
 	end,
	dynamic = function(lod)
		if lod > 3 then
			local trans = (get_time('SECONDS')*.05)
			texture('models/buildings/city3k/wtr.png',v(math.sin(trans),math.cos(trans),0), v(.2,0,0),v(0,.2,0))
			ring(3*lod,v(-7.5,-.5,4.5), v(-7.5,3.5,4.5),v(0,0,1), .9)
		end

		if lod > 3 then
			local trans = (get_time('SECONDS')*.05)
			texture('models/buildings/city3k/wtr.png',v(math.cos(trans),math.sin(trans),0), v(.2,0,0),v(0,.2,0))
			ring(3*lod,v(-6,-.5,5.5), v(-6,3.5,5.5),v(0,0,1), .9)
		end
	end
})

define_model('factory_3k_1', {
	info = {
			scale = 4,
			lod_pixels = {2,100},
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

		if lod == 1 then
			local w = 8
			local h = 8
			texture('pan0.png')
			extrusion(v(0,0,w), v(0,0,-w), v(0,1,0), 1.0,
				v(-w,-5,0), v(w,-5,0), v(w,h,0), v(-w,h,0))
		else
			set_material('glow1', .3,.3,.3,1,1,1.5,2,100,1.2,1.4,.6)
			set_material('fce_glow', .3,.3,.3,.9,0,0.0,0,0,1,.8,1.4)

			texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top
			use_material('default')
			quad(v0,v10,v11,v1) -- optional
			call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
			zbias(1,v(0,0,0),v(0,1,0))
			call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)
			zbias(0)

			use_material('default')
			cylinder(3*lod,v(-7.5,3.5,4.5), v(-7.5,4,4.5),v(0,0,1), .9)
			cylinder(3*lod,v(-6,3.5,5.5), v(-6,4,5.5),v(0,0,1), .9)

			texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			use_material('glow1')
			xref_cylinder(3*lod,v(4,4.7,-6), v(4,7,-6), v(0,0,1), .2)
			-- advert
			texture('wtr.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
			use_material('fce_glow')
			xref_cylinder(3*lod,v(3.2,-1,12), v(3.2,5,12), v(0,0,1), .2)

			call_model('ad_acme_1', v(0,2,12), v(1,0,0),v(0,1,0),3)
			call_model('ad_acme_1', v(0,2,12), v(-1,0,0),v(0,1,0),3)

			zbias(1,v(0,6,-3.001),v(0,0,-1))
			call_model('ad_acme_2', v(0,6,-3.001), v(-1,0,0),v(0,1,0),1.5)
			zbias(0)
			-- advert end

			bld_base_1(lod,1.4,0)

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

			texture('wtr.png',v(.5,.5,0), v(.5,0,0),v(0,.5,0))
			use_material('glow2')
			xref_cylinder(3*lod,v(4,4.7,-6), v(4,7,-6), v(0,0,1), .2)

			texture('pan0.png', v(.27,.3,0),v(.112,0,0),v(0,0,.87))   -- from top

			use_material('default')
		end
		quad(v0,v10,v11,v1) -- optional

		call_model('f3k_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
		zbias(1,v(0,0,0),v(0,1,0))
		call_model('f3k_top_1',v(0,0,0),v(1,0,0),v(0,1,0),1)
		zbias(0)

  		if lod > 1 then
			-- advert
			texture('wtr.png',v(.3,.5,0), v(.5,0,0),v(0,.3,0))
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

--Unfinished models?
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
