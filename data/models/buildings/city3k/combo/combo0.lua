--[[
define_model('combo1_wins', {
	info = {
			lod_pixels = {1,10,50,0},
			bounding_radius = 10,
			--materials={'win0', 'win1', 'win2', 'win3'},
		},
	static = function(lod)
		--texture('alu.png')
		--use_material('win0')
		load_obj('combo1_win0.obj')
		--use_material('win1')
		load_obj('combo1_win1.obj')
		--use_material('win2')
		load_obj('combo1_win2.obj')
		--use_material('win3')
		load_obj('combo1_win3.obj')
		--ceiling window
		load_obj('combo1_plus_win.obj')
	end,
})

define_model('combo0', {
	info = {
			lod_pixels = {1,20,30,40},
			bounding_radius = 10,
			materials={'win', 'cutout', 'trees', 'concrete'},
	},
	static = function(lod)
		set_material('cutout', .45,.55,.6,.9,.5,.5,.6,30)
		set_material('trees', .8,.7,.6,.9,.3,.5,.3,30)
		set_material('concrete',.6,.6,.5,1,.3,.3,.3,5)
		set_material('win', .2,.33,.35,.4,1.5,1.8,2,100)
		if lod == 1 then
			local w = 8
			local h = 8
			texture('alu.png')
			extrusion(v(0,0,w), v(0,0,-w), v(0,1,0), 1.0,
				v(-w,-5,0), v(w,-5,0), v(w,h,0), v(-w,h,0))
		else
			texture('alu.png')
			load_obj('combo1_1.obj')
			use_material('concrete')
			call_model('bld_base_1', v(0,0,0),v(1,0,0),v(0,1,0),1)
			if lod > 3 then
				--upper door
				texture('door.png')
				use_material('cutout')
				zbias(1,v(0,5.909,3),v(0,0,1))
				load_obj('combo1_door2.obj')
				zbias(0)
				--lower door (not a model?)
				zbias(1,v(0,1.25,7),v(0,0,1))
				circle(6,v(0,1.25,7),v(0,0,1),v(1,0,0),.8)
				zbias(0)
				--some bushes
				use_material('trees')
				texture('tree.png')
				load_obj('bush_1.obj')
			end
			--windows
			use_material('win')
			texture('alu.png')
			call_model('combo1_wins', v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})
--]]
