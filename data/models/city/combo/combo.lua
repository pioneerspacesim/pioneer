
define_model('combo', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			tags = {'city_building', 'city_power', 'city_starport_building'},
		},
	static = function(lod)
			set_material('default', .43,.5,.63,1,1.26,1.4,1.66,30)
			use_material('default')
			call_model('combo0', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_1', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .45,.55,.6,1,.3,.3,.4,30)
			use_material('default')
			call_model('combo0', v(0,-1,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_2', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .65,.6,.3,1,.4,.3,.3,30)
			use_material('default')
			call_model('combo0', v(0,-1,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_3', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .45,.65,.4,1,.4,.4,.3,30)
			use_material('default')
			call_model('combo0', v(0,-1,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_1_1', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 20,
			materials={'default'},
			tags = {'city_building', 'city_power', 'city_starport_building'},
		},
	static = function(lod)
			call_model('combo_1', v(-14,-1,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_2', v(0,-1,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_1_2', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 30,
			materials={'default', 'concrete'},
			tags = {'city_building', 'city_power', 'city_starport_building'},
		},
	static = function(lod)
            set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
			call_model('combo_1', v(-14,2,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_2', v(0,2,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_3', v(-24,2,10),v(1,0,0),v(0,1,0),1)
			zbias(1,v(-14,2.01,15),v(0,1,0))
			use_material('concrete')
			call_model('bld_base_1', v(-14,1.01,15),v(1,0,0),v(0,1,0),1)
			zbias(0)
	end
})

