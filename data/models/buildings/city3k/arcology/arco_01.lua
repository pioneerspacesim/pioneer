--[[
--baseless combo
define_model('combo_nobase', {
	info = {
		bounding_radius = 10,
		materials={'default', 'windows'},
	},
	static = function(lod)
		set_material('windows', .2,.33,.35,1.0,1.5,1.8,2,100)
		load_obj('../combo/combo1_1.obj')
		use_material('windows')
		texture('alu.png')
		--call_model('combo1_wins', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})

--some colour & amount variations follow
define_model('combo_2_0', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .45,.55,.6,1,.3,.3,.4,30)
			use_material('default')
			call_model('combo_nobase', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_2_1', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .65,.6,.3,1,.4,.3,.3,30)
			use_material('default')
			call_model('combo_nobase', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_2_2', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			materials={'default'},
			},
	static = function(lod)
			set_material('default', .45,.65,.4,1,.4,.4,.3,30)
			use_material('default')
			call_model('combo_nobase', v(0,0,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_2_3', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 20,
			materials={'default'},
   			},
	static = function(lod)
			call_model('combo_2_1', v(-14,-1,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_2_2', v(0,-1,0),v(1,0,0),v(0,1,0),1)
	end
})

define_model('combo_2_4', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 30,
			},
	static = function(lod)
			call_model('combo_2_0', v(-14,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_2_1', v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('combo_2_2', v(-24,0,10),v(1,0,0),v(0,1,0),1)
	end
})

--top part & insides of the arcology, except dome
define_model('arco01_inside', {
	info = {
			scale = 1,
			lod_pixels = {20,100,200},
			bounding_radius = 500,
			materials = {'grass', 'steel', 'gravel', 'metal', 'chrome', 'lake', 'win1', 'win2',
						'win3', 'win4', 'win5', 'win6', 'win7', 'win8'},
	},

	static = function(lod)
		set_material('steel', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('lake', .4,.5,.5,1,.3,.4,.5,50)
		set_material('gravel', .3,.31,.3,1,.3,.3,.3,5)
		set_material('grass', .2,.3,0,1,.3,.3,.3,5)
		set_material('metal', .5,.55,.55,1,.35,.38,.4,10)

		--outer ring
		use_material('steel')
		texture('bot5.png',v(.5,.5,0),v(.0019,0,0),v(0,0,1))
		tube(16*lod,v(0,480,0),v(0,520,0),v(1,0,0),230,250)

		--ground
		use_material('grass')
		texture('../rgh.png',v(.5,.5,0),v(.05,0,0),v(0,0,1))
		circle(16,v(0,515,0),v(0,1,0),v(1,0,0),245)

		--hill
		zbias(1,v(0,515,0), v(0,1,0))
		cubic_bezier_quad(lod,lod,
				v(-20,514,60), v(-60,514,80), v(-120,514,80), v(-160,514,90),
				v(0,514,100), v(-60,600,100), v(-120,514,100), v(-160,514,100),
				v(0,514,120), v(-60,514,120), v(-120,530,120), v(-160,514,120),
				v(-20,514,160), v(-60,514,180), v(-120,514,180), v(-160,514,160))

		--gravel crossroads
		use_material('gravel')
		texture('grav.png')
		quad(v(247,515,5), v(247,515,-5), v(-247,515,-5), v(-247,515,5))
		quad(v(5,515,247), v(5,515,-247), v(-5,515,-247), v(-5,515,247))
		zbias(0)

		--lake
		texture('../wtr.png',v(0.5,.5,0), v(.02,0,0),v(0,0,-1))
		use_material('lake')
		zbias(1,v(0,515,0),v(0,1,0))
			flat(3*lod,v(0,1,0),{v(-40,515,80), v(-50,515,40)}, {v(-60,515,-10),v(-100,515,60),v(-120,515,50)},{v(-140,515,50), v(-140,515,70)}, {v(-140,515,80),v(-120,515,80)})
		zbias(0)

		set_material('win1', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win2', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		set_material('win3', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		set_material('win4', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		set_material('win5', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		set_material('win6', .2,.33,.35,.9,1.5,1.8,2,100)
		set_material('win7', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		set_material('win8', .2,.33,.35,.9,1.5,1.8,2,100,1.4,1.6,1.8)
		zbias(1,v(0,500,0), v(0,1,0))
		texture('../win0.png')
		use_material('win1')
		load_obj('win1.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win2')
		load_obj('win2.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win3')
		load_obj('win3.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win4')
		load_obj('win4.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win5')
		load_obj('win5.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win6')
		load_obj('win6.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win7')
		load_obj('win7.obj',Matrix.scale(v(10.02,10,10.02)))
		use_material('win8')
		load_obj('win8.obj',Matrix.scale(v(10.02,10,10.02)))
		zbias(0)

		--houses
		call_model('combo_2_3', v(-110,515,30), v(1,0,-1),v(0,1,0),1.25)
		call_model('combo_2_4', v(30,515,70), v(0,0,1),v(0,1,0),1.25)
		call_model('combo_2_4', v(-20,515,-70), v(1,0,0),v(0,1,0),1.25)
		call_model('combo_2_0', v(70,515,-70), v(1,0,1),v(0,1,0),1.25)
		call_model('combo_2_1', v(30,515,100), v(0,0,1),v(0,1,0),1.25)
		call_model('combo_2_2', v(100,515,-30), v(1,0,0),v(0,1,0),1.25)
		call_model('combo_2_4', v(-50,515,-150), v(0,0,-1),v(0,1,0),1.25)
		call_model('combo_2_3', v(-160,515,70), v(-1,0,-1),v(0,1,0),1.25)
		call_model('combo_2_4', v(120,515,50), v(-1,0,0),v(0,1,0),1.25)
		call_model('combo_2_4', v(-120,515,-30), v(1,0,0),v(0,1,0),1.25)

		--trees
		call_model('woods_1',v(100,515,-100), v(0,0,1),v(0,1,0),1.25)
		call_model('woods_1',v(-150,515,-100), v(1,0,0),v(0,1,0),1.25)
		call_model('woods_1',v(100,515,150), v(0,0,1),v(0,1,0),1.25)
		call_model('woods_1',v(150,515,-120), v(1,0,0),v(0,1,0),1.25)
		call_model('woods_1',v(-130,515,-130), v(0,0,1),v(0,1,0),1.25)
		call_model('woods_1',v(-50,515,150), v(-1,0,0),v(0,1,0),1.25)
		call_model('woods_1',v(-110,515,120), v(-1,0,1),v(0,1,0),1.25)
		call_model('woods_1',v(-170,515,40), v(1,0,0),v(0,1,0),1.25)

		--temple thing at center
		call_model('church_new_0',v(0,515,0), v(1,0,0),v(0,1,0),1.5)
	end,
})

--base and dome at top
define_model('arco01_base', {
	info = {
		scale = .8,
		lod_pixels = {30,200,300},
		bounding_radius = 500,
		materials = {'grass', 'concrete', 'gravel', 'steel', 'text', 'metal', 'chrome', 'win', 'dome','dome_trans'},
		tags = {'city_building'},
	},

	static = function(lod)
		set_material('steel', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('dome', 0,.5,1,.3,1,1.5,2,100)
		set_material('dome_trans', 0,.5,1.0,.3,1,1.5,2,100)
		set_material('grass', .2,.3,0,1,.3,.3,.3,5)
		set_material('concrete',.7,.7,.6,1,.3,.3,.3,5)
		set_material('gravel', .3,.31,.3,1,.3,.3,.3,5)
		set_material('metal', .5,.55,.55,1,.35,.38,.4,10)
		set_material('text', .5,.5,0,1,0,0,0,0,.5,.5,0)
		set_material('dome', 0,.5,1,.3,1,1.5,2,100)
		if lod == 1 then
			--dome
			use_material('dome')
			sphere_slice(8,3,0,.3*math.pi, Matrix.translate(v(0,300,0)) * Matrix.scale(v(320,320,320)))
			--dome base
			use_material('steel')
			texture('bot5.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			tapered_cylinder(8,v(0,400,0),v(0,480,0),v(1,0,0),50,250)
			--leg
			use_material('concrete')
			texture('../conc.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			tapered_cylinder(8,v(0,-200,0),v(0,400,0),v(1,0,0),100,50)
		else
			use_material('concrete')
			texture('../conc.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			tapered_cylinder(16*lod,v(0,-200,0),v(0,400,0),v(1,0,0),100,50)

			use_material('steel')
			texture('bot5.png',v(.5,.5,0),v(.002,0,0),v(0,0,1))
			tapered_cylinder(16*lod,v(0,400,0),v(0,480,0),v(1,0,0),50,250)

			use_material('steel')
			texture('../pan0.png',v(.5,.5,0),v(.01,0,0),v(0,.01,0))
			xref_flat(8*lod,v(0,0,1), {v(49,400,10)}, {v(65,200,10)}, {v(50,400,10), v(250,480,10)})
			xref_flat(8*lod,v(0,0,-1), {v(-49,400,-10)}, {v(-65,200,-10)}, {v(-50,400,-10), v(-250,480,-10)})

			texture('../pan0.png',v(.5,.5,0),v(-.01,0,0),v(0,.01,0))
			quadric_bezier_quad(8*lod,1,v(10,200,65), v(10,400,50), v(10,480,250),
										v(0,200,65), v(0,400,50), v(0,480,250),
										v(-10,200,65), v(-10,400,50), v(-10,480,250))

			quadric_bezier_quad(8*lod,1,v(-10,200,-65), v(-10,400,-50), v(-10,480,-250),
										v(0,200,-65), v(0,400,-50), v(0,480,-250),
										v(10,200,-65), v(10,400,-50), v(10,480,-250))

			texture('../pan0.png',v(.5,.5,0),v(0,0,1),v(0,.01,0))
			xref_flat(8*lod,v(-1,0,0), {v(-10,400,49)}, {v(-10,200,65)}, {v(-10,400,50), v(-10,480,250)})
			xref_flat(8*lod,v(1,0,0), {v(10,400,-49)}, {v(10,200,-65)}, {v(10,400,-50), v(10,480,-250)})

			texture('../pan0.png',v(.5,.5,0),v(0,0,-1),v(0,.01,0))
			xref_quadric_bezier_quad(8*lod,1,	v(65,200,-10), v(50,400,-10), v(250,480,-10),
												v(65,200,0), v(50,400,0), v(250,480,0),
												v(65,200,10), v(50,400,10), v(250,480,10))
			texture(nil)

			--innards
			call_model('arco01_inside', v(0,0,0), v(1,1,0), v(0,1,0),1)

			--transparent dome
			use_material('dome_trans')
			sphere_slice(12*lod,4*lod,0,.3*math.pi, Matrix.translate(v(0,340,0)) * Matrix.scale(v(305,305,305)))
		end
  	end,
})
--]]
