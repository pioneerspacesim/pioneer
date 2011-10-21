define_model('m_pod', {
	info = {
		bounding_radius = 2,
		materials = {'d_grey'},
	},
	static = function(lod)
		set_material('d_grey', .3,.32,.35,1,.33,.35,.4,30)
		use_material('d_grey')
		texture('pod.png',v(.5,0,0),v(0,0,.5),v(0,-2,0))
		extrusion(v(0,0,-.5),v(0,0,.5),v(0,1,0),1,v(.1,0,0),v(.1,.5,0),v(-.1,.5,0),v(-.1,0,0))
	end
})

define_model('missile_head', {
	info = {
		bounding_radius = 1,
	},
	static = function(lod)
		texture('metal_s.png')
		load_obj('m_head.obj')
	end
})

define_model('missile_body', {
	info = {
		bounding_radius = 1,
	},
	static = function(lod)
		texture('metal_s.png')
		load_obj('m_body.obj')
	end
})

define_model('missile_neck', {
	info = {
		bounding_radius = 1,
	},
	static = function(lod)
		texture('metal_s.png')
		load_obj('m_neck.obj')
	end
})

define_model('missile_nazzle', {
	info = {
		bounding_radius = 1,
		materials = {'nazzle'},
	},
	static = function(lod)
		set_material('nazzle', .3,.3,.3,1,.3,.3,.3,10)
		use_material('nazzle')
		texture('naz_ns.png')
		load_obj('m_nazzle.obj')
	end
})

define_model('m_unguided', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body'},
		ship_defs = {
			{
				'MISSILE_UNGUIDED',
				{ 0, -1*10^10, 0, 0, 0, 0 },
				0,
				{},
				{ 0, 0, 1, 0 },
				10, 1, 100
			}
		},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .6,.65,.7,1,.5,.6,.7,30)

			use_material('body')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)

			thruster(v(0,0,2.2), v(0,0,1), 5, true)
		else
			cylinder(3, v(0,0,2), v(0,0,-2), v(0,1,0), .25)
		end
	end
})

define_model('m_guided', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'head'},
		ship_defs = {
			{
				'MISSILE_GUIDED',
				{ 1*10^6, -2*10^6, 5*10^5, 5*10^5, 5*10^5, 5*10^5 },
				1*10^6,
				{},
				{ 0, 0, 1, 0 },
				10, 1, 100
			}
		},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .5,.6,.7,1,.5,.6,.7,30)
			set_material('head', .8,0,0,1,.4,.4,.4,30)

			use_material('head')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('body')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)

			thruster(v(0,0,2.2), v(0,0,1), 5, true)
		else
			cylinder(3, v(0,0,2), v(0,0,-2), v(0,1,0), .25)
		end
	end
})

define_model('m_smart', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'neck'},
		ship_defs = {
			{
				'MISSILE_SMART',
				{ 2*10^6, -4*10^6, 1*10^6, 1*10^6, 1*10^6, 1*10^6 },
				2*10^6,
				{},
				{ 0, 0, 1, 0 },
				10, 1, 100
			}
		},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .8,0,0,1,.4,.4,.4,30)
			set_material('neck', .8,.8,.8,1,.5,.5,.5,30)

			use_material('body')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('neck')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)

			thruster(v(0,0,2.2), v(0,0,1), 5, true)
		else
			cylinder(3, v(0,0,2), v(0,0,-2), v(0,1,0), .25)
		end
	end
})

define_model('m_naval', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'neck', 'head'},
		ship_defs = {
			{
				'MISSILE_NAVAL',
				{ 4*10^6, -6*10^6, 2*10^6, 2*10^6, 2*10^6, 2*10^6 },
				4*10^6,
				{},
				{ 0, 0, 1, 0 },
				20, 1, 100
			}
		},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', 0,.1,.4,1,.5,.6,.7,30)
			set_material('head', .8,0,0,1,.4,.4,.4,30)
			set_material('neck', .6,.4,0,1,.4,.4,.4,30)

			use_material('head')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('neck')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('body')
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)

			thruster(v(0,0,2.2), v(0,0,1), 5, true)
		else
			cylinder(3, v(0,0,2), v(0,0,-2), v(0,1,0), .25)
		end
	end
})

-- dummy missiles, to mount on ships

define_model('d_unguided', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .6,.65,.7,1,.5,.6,.7,30)
			use_material('body')

			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('d_guided', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'head'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .5,.6,.7,1,.5,.6,.7,30)
			set_material('head', .8,0,0,1,.4,.4,.4,30)

			use_material('head')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('body')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('d_smart', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'neck'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', .8,0,0,1,.4,.4,.4,30)
			set_material('neck', .8,.8,.8,1,.5,.5,.5,30)

			use_material('body')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('neck')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('d_naval', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 4,
		materials={ 'body', 'neck', 'head'},
	},
	static = function(lod)
		if lod > 1 then
			set_material('body', 0,.1,.4,1,.5,.6,.7,30)
			set_material('head', .8,0,0,1,.4,.4,.4,30)
			set_material('neck', .6,.4,0,1,.4,.4,.4,30)

			use_material('head')
			call_model('missile_head',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('neck')
			call_model('missile_neck',v(0,0,0),v(1,0,0),v(0,1,0),1)

			use_material('body')
			call_model('missile_body',v(0,0,0),v(1,0,0),v(0,1,0),1)

			call_model('missile_nazzle',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})
