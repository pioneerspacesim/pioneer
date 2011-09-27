define_model('ad_cola_1', {
	info = {
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,2,.8,1.2)
		set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

		use_material('glow1')
		texture('wtr.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)
		
		use_material('glow2')
		texture('coolcola.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		zbias(1,v(0,.5,0), v(0,0,1))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.3, {0,0,0,1,0,0,0,0,.6,1.5,2.5},
								{0,0,0,1,0,0,0,0,2,.6,1.5}))
	end
})

define_model('ad_cola_0', {
	info = {
		lod_pixels = {1,10,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,2,.8,1.2)
		set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

		use_material('glow1')
		texture('wtr.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)
		use_material('glow2')
		zbias(1,v(0,.5,0), v(0,0,1))
		texture('coolcola.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		if lod > 3 then
			local v0 = v(1,0,0)
			local v1 = v(1,1,0)
			local v2 = v(-1,1,0)
			local v3 = v(-1,0,0)
			local trans = get_arg(1)*.1

			use_material('glow1')
			texture('sub_models/adverts/wtr.png', v(.5,math.sin(trans),0),v(.5,0,0),v(0,math.cos(trans),0))
			quad(v0,v1,v2,v3)
			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('sub_models/adverts/coolcola.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
			zbias(0)
		end
	end
})

define_model('ad_acme_0', {
	info = {
		lod_pixels = {1,10,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,2,1.8,.6)
		set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

		use_material('glow1')
		texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)

		use_material('glow2')
		zbias(1,v(0,.5,0), v(0,0,1))
		texture('acme_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		if lod > 3 then
			local v0 = v(1,0,0)
			local v1 = v(1,1,0)
			local v2 = v(-1,1,0)
			local v3 = v(-1,0,0)
			local trans = get_arg(1)*.1

			use_material('glow1')
			texture('sub_models/adverts/wtr_x.png', v(.5,trans,0),v(.5,0,0),v(0,1,0))
			quad(v0,v1,v2,v3)

			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			texture('sub_models/adverts/acme_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			quad(v0,v1,v2,v3)
			zbias(0)
		end
	end
})

define_model('ad_acme_1', {
	info = {
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,2,1.8,.6)
		set_material('glow2', 0,0,0,.99,0,0,0,0,1,1.6,1.8)

		use_material('glow1')
		texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)

		use_material('glow2')
		texture('acme_0.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))

		zbias(1,v(0,.5,0), v(0,0,1))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.1, {0,0,0,1,0,0,0,0,2.5,1.2,.6},
								{0,0,0,1,0,0,0,0,1.5,1.8,.6}))	
	end
})

define_model('ad_acme_2', {
	info = {
		bounding_radius = 1.0,
		materials = {'glow1'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,.99,0,0,0,0,2,1.8,.6)

		texture('acme_1.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))	
		use_material('glow1')	
		quad(v0,v1,v2,v3)	
	end
})

define_model('ad_pioneer_0', {
	info = {
		lod_pixels = {1,20,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
		set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		use_material('glow1')
		texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)

		use_material('glow2')
		zbias(1,v(0,.5,0), v(0,0,1))
		texture('pioneer_0_l.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		if lod > 3 then
			local v0 = v(1,0,0)
			local v1 = v(1,1,0)
			local v2 = v(-1,1,0)
			local v3 = v(-1,0,0)
			local trans = get_arg(1)*.1

			use_material('glow1')
			texture('sub_models/adverts/wtr_x.png', v(math.sin(trans),math.cos(trans),0),v(.5,0,0),v(0,1,0))
			quad(v0,v1,v2,v3)
			texture('sub_models/adverts/pioneer_0_l.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			quad(v0,v1,v2,v3)
			zbias(0)
		end
	end
})

define_model('ad_sirius_0', {
	info = {
		lod_pixels = {1,10,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
		set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		use_material('glow1')
		texture('tie-d_b.png', v(.5,.5,0),v(.5,0,0),v(0,.05,0))
		quad(v0,v1,v2,v3)

		use_material('glow2')
		zbias(1,v(0,.5,0), v(0,0,1))
		texture('sirius_2.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		if lod > 3 then
			local v0 = v(1,0,0)
			local v1 = v(1,1,0)
			local v2 = v(-1,1,0)
			local v3 = v(-1,0,0)
			local trans = get_arg(1)*.05

			use_material('glow1')
			texture('sub_models/adverts/tie-d_b.png', v(.5,trans,0),v(.4,0,0),v(0,.05,0))
			quad(v0,v1,v2,v3)
			texture('sub_models/adverts/sirius_2.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
			use_material('glow2')
			zbias(1,v(0,.5,0), v(0,0,1))
			quad(v0,v1,v2,v3)
			zbias(0)
		end
	end
})

define_model('ad_sirius_1', {
	info = {
		lod_pixels = {1,10,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow2', 0,0,0,.99,0,0,0,0,.8,1,1.2)
		
		use_material('glow1')
		texture('tie-d_b.png', v(0,0,0),v(.25,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)
		use_material('glow2')
		texture('sirius_2.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		zbias(1,v(0,.5,0), v(0,0,1))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.2, {0,0,0,1,0,0,0,0,.5,2.5,1.5},
								{0,0,0,1,0,0,0,0,.5,1.5,2.5}))
	end
})

define_model('ad_sirius_2', {
	info = {
		bounding_radius = 1.0,
		materials = {'glow1'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow1', 0,0,0,.99,0,0,0,0,1,1.5,2)

		texture('sirius_3.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		use_material('glow1')	
		quad(v0,v1,v2,v3)	
	end
})

define_model('inteloutside', {
	info = {
		lod_pixels = {1,10,30,0},
		bounding_radius = 1.0,
		materials = {'glow1', 'glow2'},
		tags = {'advert'}
	},
	static = function(lod)
		local v0 = v(1,0,0)
		local v1 = v(1,1,0)
		local v2 = v(-1,1,0)
		local v3 = v(-1,0,0)

		set_material('glow2', 0,0,0,.9,0,0,0,0,.5,.6,.9)

		use_material('glow1')
		texture('wtr_x.png', v(.5,.5,0),v(.5,0,0),v(0,1,0))
		quad(v0,v1,v2,v3)

		use_material('glow2')
		texture('inteloutside.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))
		zbias(1,v(0,.5,0), v(0,0,1))
		quad(v0,v1,v2,v3)
		zbias(0)
	end,
	dynamic = function(lod)
		set_material('glow1',lerp_materials(get_arg(1)*0.1, {0,0,0,1,0,0,0,0,2.5,2,.6},
								{0,0,0,1,0,0,0,0,.6,2,2.5}))
	end
})
