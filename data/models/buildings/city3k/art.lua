--[[
-- Mysterious objects

define_model('green_bubble', {
	info = {
		scale = 1.5,
		lod_pixels = {1,10},
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

		texture('alu.png',v(.5,.5,0),v(.2,0,0),v(0,0,1))
		use_material('default')

		quad(v12,v13,v11,v10)
		tri(v12,v14,v13)
		quad(v0,v2,v12,v10)
		quad(v2,v4,v14,v12)
		quad(v4,v3,v13,v14)
		quad(v3,v1,v11,v13)
		quad(v1,v0,v10,v11)

		if lod > 1 then

			use_material('glow2')
			texture('wtr_x.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
			sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(2.99,2.99,2.99)))

			use_material('cutout')
			texture('bubbles.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
			sphere_slice(4*lod,2*lod,0,math.pi*.5, Matrix.translate(v(0,3,0)) * Matrix.scale(v(3,3,3)))
			bld_base_2(lod,1.2,0)
		end
	end,
	dynamic = function(lod)
		--pulsating glow
		if lod > 1 then
			set_material('glow2',lerp_materials(get_time('SECONDS')*0.3, {0,0,0,1,0,0,0,0,1.6,1.9,0},
																 {0,0,0,1,0,0,0,0,1,2.5,0}))
		end
	end
})
--]]

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
			set_material('glow2',lerp_materials(get_time('SECONDS')*0.3, {0,0,0,1,0,0,0,0,2.5,1,1.5},
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
			set_material('glow2',lerp_materials(get_time('SECONDS')*2, {0,0,0,1,0,0,0,0,1.6,1.9,0},
																 {0,0,0,1,0,0,0,0,1.4,1,1.8}))
		end

	end
})
--]]

--[[
define_model('pink_obelisk', {
	info = {
		scale = .7,
		lod_pixels = {1,10},
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

		--set_material('glow1',0,0,0,.9,0,0,0,0,1,1.6,1.8)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
		set_material('default', .2,.2,.2,1,.3,.3,.3,30)
		set_material('fce_glow',0,0,0,1,0,0,0,0,2,1,2.5)
		set_material('cutout',0,0,0,.999,.6,.6,.6,30)

		use_material('default')
		texture('alu.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))

		--cylinder(6*lod, v(0,0,0), v(0,5,0), v(0,0,1), 10)

		use_material('glow2')
		texture('wtr.png',v(.5,.5,0),v(.1,0,0),v(0,.03,0))

		tri(v0,v1,v2)
		tri(v0,v8,v7)
		quad(v1,v3,v4,v2)
		quad(v8,v10,v9,v7)
		quad(v3,v5,v6,v4)
		quad(v10,v12,v11,v9)

		texture('wtr.png',v(.5,.5,0),v(0,0,-2),v(0,.03,0))
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
			set_material('glow2',lerp_materials(get_time('SECONDS')*0.1, {0,0,0,1,1,1,1,100,2.2,1,1.5},
																 {0,0,0,1,1,1,1,100,1.5,1,2.2}))
		end
	end
})
--]]
