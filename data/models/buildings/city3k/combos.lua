--[[
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

	use_material('default')
	texture('alu.png')
	if lod == 1 then
		local w = 8
		local h = 8
		extrusion(v(0,0,w), v(0,0,-w), v(0,1,0), 1.0,
			v(-w,-5,0), v(w,-5,0), v(w,h,0), v(-w,h,0))
	else
		load_obj('combo_0.obj')
	end

	if lod == 3 then
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

		if type == 0 then
			texture('tree.png')
			use_material('trees')
			load_obj('bush_0.obj')
		elseif type == 1 then
			texture('tree.png')
			use_material('trees')
			load_obj('bush_1.obj')

			use_material('win') --roof window
			texture('win.png',v(0,0,0),v(.2,0,0),v(0,0,1))
 			quad(v(3,5,7),v(3,7,3),v(-3,7,3),v(-3,5,7))
		end
	end
end

define_model('combo_b', {
	info = {
		lod_pixels = {1,3,5},
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
		lod_pixels = {1,3,5},
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
		lod_pixels = {1,3,5},
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
		lod_pixels = {1,3,5},
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
		lod_pixels = {1,3,5},
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
		lod_pixels = {1,3,5},
		bounding_radius = 10,
		materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
		tags = {'city_building'},
	},
	static = function(lod)
		combo_sub(lod,0,.73,.7,.83,1,1.26,1.4,1.66,30)
	end
})

define_model('combo_y_i', {
	info = {
		lod_pixels = {1,3,5},
		bounding_radius = 10,
		materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
		tags = {'city_building'},
	},
	static = function(lod)
		combo_sub(lod,0,1,.9,.4,1,1.26,1.4,1.66,30)
	end
})

define_model('combo_g_i', {
	info = {
		lod_pixels = {1,3,5},
		bounding_radius = 10,
		materials = {'default', 'cutout', 'trees', 'grass', 'win', 'win_on', 'win_off', 'concrete','fce_glow'},
		tags = {'city_building'},
	},
	static = function(lod)
		combo_sub(lod,0,.7,1,.7,1,1.26,1.4,1.66,30)
	end
})

define_model('combo_twin_i', {
	info = {
		lod_pixels = {1,3,5},
		bounding_radius = 10,
		tags = {'city_building'},
	},
	static = function(lod)
			call_model('combo_y_i', v(-7,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g_i', v(7,0,0),v(-1,0,0),v(0,1,0),1)
	end
})

define_model('combo_tri_i', {
	info = {
			lod_pixels = {1,3,5},
			bounding_radius = 10,
			tags = {'city_building'},
	},
	static = function(lod)
			call_model('combo_b_i', v(14,0,-14),v(-1,0,0),v(0,1,0),1)
			call_model('combo_y_i', v(14,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_g_i', v(0,0,-14),v(-1,0,0),v(0,1,0),1)
	end
})
--]]
