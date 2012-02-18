define_model('courier_tip', {     -- engine tips
	info = {
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 30,
		materials={'courier', 'color1', 'color2', 'grey', 'nazzle'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('color1', .6,.6,.6,1,1,.85,.9,100,1.5,.4,.7)  -- ('color1', .35,.1,.15,1,1,.85,.9,100)
		set_material('color2', .6,.6,.6,1,.90,.85,1,100,.7,.4,1.5) -- ('color2', .15,.1,.35,1,.90,.85,1,100)
		set_material('grey', .3,.3,.3,1,.3,.3,.3,10)
		set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,10)

		use_material('courier')
		if lod > 1 then
			texture('c_nazzles.png')
			load_obj('c_tip_n_out.obj')
		end

		texture('c_eng_l.png')
		load_obj('c_tip1.obj')

		texture('wtr.png')
		use_material('color1')
		load_obj('c_tip2.obj')

		use_material('color2')
		load_obj('c_tip3.obj')

		if lod > 1 then
			texture('c_nazzles.png')
			load_obj('c_tip_n_out.obj')
			use_material('grey')
			load_obj('c_tip_n_in.obj')
		end

		use_material('nazzle')
		texture('c_chrome.png')
		load_obj('c_nazzle.obj')

		local vMainThruster = v(0,-15,0)
		local vRetroThruster1 = v(3.5,6.3,0)
		local vRetroThruster2 = v(-3.5,6.3,0)
		local vRetroThruster3 = v(0,6.3,3.5)
		local vRetroThruster4 = v(0,6.3,-3.5)

		thruster(vMainThruster, v(0,-1,0), 30, true)
		thruster(vRetroThruster1, v(0,1,0), 6, true)
		thruster(vRetroThruster2, v(0,1,0), 6, true)
		thruster(vRetroThruster3, v(0,1,0), 6, true)
		thruster(vRetroThruster4, v(0,1,0), 6, true)
	end
})

define_model('courier_top_v1', {    -- courier top v1
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_top.obj')
	end
})

define_model('courier_top_v2', {    -- courier top v2
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_top.obj')
	end
})

define_model('trader_top_v1', {    -- trader top v1
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('t_top.obj')
	end
})

define_model('trader_top_v2', {    -- trader top v2
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('t_top.obj')
	end
})

define_model('courier_lwp_top5', {  -- courier leftwing top-part 5
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_lwpt_5.obj')
	end
})

define_model('courier_rwp_top5', {  -- courier rightwing top-part 5
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_rwpt_5.obj')
	end
})

define_model('courier_lwp_top4', {  -- courier leftwing top-part 4
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_lwpt_4.obj')
	end
})

define_model('courier_rwp_top4', {  -- courier rightwing top-part 4
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_rwpt_4.obj')
	end
})

define_model('courier_lwp_top3', {  -- courier leftwing top-part 3
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_lwpt_3.obj')
	end
})

define_model('courier_rwp_top3', {  -- courier rightwing top-part 3
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_rwpt_3.obj')
	end
})

define_model('courier_lwp_top2', {  -- courier leftwing top-part 2
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_lwpt_2.obj')
	end
})

define_model('courier_rwp_top2', {  -- courier rightwing top-part 2
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('c_shell_l.png')
		load_obj('c_rwpt_2.obj')
	end
})

define_model('mercury_lwp_top5', {  -- mercury leftwing top-part 5
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_lwpt_5.obj')
	end
})

define_model('mercury_rwp_top5', {  -- mercury rightwing top-part 5
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_rwpt_5.obj')
	end
})

define_model('mercury_lwp_top4', {  -- mercury leftwing top-part 4
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_lwpt_4.obj')
	end
})

define_model('mercury_rwp_top4', {  -- mercury rightwing top-part 4
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_rwpt_4.obj')
	end
})

define_model('mercury_lwp_top3', {  -- mercury leftwing top-part 3
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_lwpt_3.obj')
	end
})

define_model('mercury_rwp_top3', {  -- mercury rightwing top-part 3
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_rwpt_3.obj')
	end
})

define_model('mercury_lwp_top2', {  -- mercury leftwing top-part 2
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_lwpt_2.obj')
	end
})

define_model('mercury_rwp_top2', {  -- mercury rightwing top-part 2
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
	},
	static = function(lod)
		texture('m_shell_l.png')
		load_obj('c_rwpt_2.obj')
	end
})

define_model('courier_body', {    -- courier shell
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
		materials = {'courier', 'mercury', 'text1', 'text2'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_bot.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_top_v1', v(0,0,0), v(1,0,0), v(0,1,0),1)
			local reg = get_label()
			set_material('text1', .45,.1,.15,1,.1,.1,.1,10)
			set_material('text2', .15,.1,.55,1,.1,.1,.1,10)
			use_material('text1')
			text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
			use_material('text2')
			text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
			if lod > 1 then
				call_model('squadsign_1', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3)
				call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)
			end
		else
			if select2 < 67 then
				use_material('courier')
				call_model('courier_top_v2', v(0,0,0), v(1,0,0), v(0,1,0),1)
				local reg = get_label()
				set_material('text1', .45,.1,.15,1,.1,.1,.1,10)
				set_material('text2', .15,.1,.55,1,.1,.1,.1,10)
				use_material('text1')
				text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
				use_material('text2')
				text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
				if lod > 1 then
					call_model('squadsign_1', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3)
					call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)
				end
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('courier_top_v2', v(0,0,0), v(1,0,0), v(0,1,0),1)
					local reg = get_label()
					set_material('text1', .63,.7,.83,1,.2,.2,.2,10)
					set_material('text2', .05,.05,.05,1,.3,.3,.3,10)
					use_material('text1')
					text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
					use_material('text2')
					text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
					if lod > 1 then
						call_model('squadsign_4', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3) -- upper side left
						call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)  -- lower side right
					end
				end
			end
		end
	end
})

define_model('trader_body', {    --trader shell
	info =	{
		lod_pixels = { 50, 100, 200, 0 },
		bounding_radius = 32,
		materials = {'courier', 'mercury', 'text1', 'text2'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_bot.obj')
	end,
	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('trader_top_v1', v(0,0,0), v(1,0,0), v(0,1,0),1)
			local reg = get_label()
			set_material('text1', .45,.1,.15,1,.1,.1,.1,10)
			set_material('text2', .15,.1,.55,1,.1,.1,.1,10)
			use_material('text1')
			text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
			use_material('text2')
			text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
			if lod > 1 then
				call_model('squadsign_1', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3)
				call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)
			end
		else
			if select2 < 67 then
				use_material('courier')
				call_model('trader_top_v2', v(0,0,0), v(1,0,0), v(0,1,0),1)
				local reg = get_label()
				set_material('text1', .45,.1,.15,1,.1,.1,.1,10)
				set_material('text2', .15,.1,.55,1,.1,.1,.1,10)
				use_material('text1')
				text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
				use_material('text2')
				text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
				if lod > 1 then
					call_model('squadsign_1', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3)
					call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)
				end
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('trader_top_v2', v(0,0,0), v(1,0,0), v(0,1,0),1)
					local reg = get_label()
					set_material('text1', .63,.7,.83,1,.2,.2,.2,10)
					set_material('text2', .05,.05,.05,1,.3,.3,.3,10)
					use_material('text1')
					text(reg, v(10, 1.746, 3.5), v(0,1,0), v(1,-0.083,0), 1.5, {center=true})
					use_material('text2')
					text(reg, v(-10, .318, 3.5), v(0,-1,0), v(-1,0.047,0), 1.5, {center=true})
					if lod > 1 then
						call_model('squadsign_4', v(-8,1.92,5.5), v(-0.084,1,0), v(0,0,-1),3) -- upper side left
						call_model('squadsign_1', v(8,0.27,5.5), v(0.044,-1,0), v(0,0,-1),3)  -- lower side right
					end
				end
			end
		end
	end
})

define_model('courier_lwp5', {  -- leftwing part 5 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_lwpb_5.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_lwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_lwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_lwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_rwp5', {  -- rightwing part 5 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_rwpb_5.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_rwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_rwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_rwp_top5', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_lwp4', {  -- leftwing part 4 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_lwpb_4.obj')
	end,
	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_lwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_lwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_lwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_rwp4', {  -- rightwing part 4 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_rwpb_4.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_rwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_rwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_rwp_top4', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_lwp3', {  -- leftwing part 3 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_lwpb_3.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_lwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_lwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_lwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_rwp3', {  -- rightwing part 3 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_rwpb_3.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_rwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_rwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_rwp_top3', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_lwp2', {  -- leftwing part 2 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_lwpb_2.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_lwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_lwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_lwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_rwp2', {  -- rightwing part 2 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 5,
		materials = {'courier', 'mercury'},
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_rwpb_2.obj')
	end,

	dynamic = function(lod)
		selector2()
		if select2 < 34 then
			use_material('courier')
			call_model('courier_rwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
		else
			if select2 < 67 then
				use_material('courier')
				call_model('mercury_rwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
			else
				if select2 > 66 then
					set_material('mercury', get_arg_material(0))
					use_material('mercury')
					call_model('mercury_rwp_top2', v(0,0,0), v(1,0,0), v(0,1,0),1)
				end
			end
		end
	end
})

define_model('courier_flap_ll', {  -- left flap1 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'courier'}
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_flap_ll.obj', Matrix.rotate(0.5*math.pi, v(-1,0,0)))
	end
})

define_model('courier_flap_lr', {  -- left flap2 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'courier'}
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_flap_lr.obj', Matrix.rotate(0.5*math.pi, v(1,0,0)))
	end
})

define_model('courier_flap_rr', {  -- right flap1 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'courier'}
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_flap_rr.obj', Matrix.rotate(0.5*math.pi, v(1,0,0)))
	end
})

define_model('courier_flap_rl', {  -- right flap2 all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'courier'}
	},

	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_flap_rl.obj', Matrix.rotate(0.5*math.pi, v(-1,0,0)))
	end
})

define_model('courier_wheels_l', {    -- wheels left all models
	info =	{
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'black', 'courier'},
	},

	static = function(lod)

		texture('c_metal.png', v(.5,.5,0), v(1,0,0), v(0,0,1))
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		load_obj('c_sledge_l.obj')

		set_material('black', .05,.05,.05,1,.3,.3,.3,10)
		use_material('black')
		load_obj('c_wheels_l.obj')
	end
})

define_model('courier_wheels_r', {    -- wheels right all models
	info =	{
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 10,
		materials = {'black', 'courier'},
	},

	static = function(lod)

		texture('c_metal.png')
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		load_obj('c_sledge_r.obj')

		texture(nil)
		set_material('black', .05,.05,.05,1,.3,.3,.3,10)
		use_material('black')
		load_obj('c_wheels_r.obj')
	end
})

define_model('courier_eng_l', {      -- engine part left all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 45,
		materials={'courier', 'e_glow1', 'e_glow2', 'iron', 'nazzle'},
	},
	static = function(lod)

		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_eng_l.obj')

		set_material('iron', .4,.45,.55,1,.2,.2,.2,10)
		use_material('iron')
		texture('c_metal.png')
		load_obj('c_cage_l.obj')

		use_material('e_glow1')
		texture('c_e_inside.png')
		load_obj('c_e_inside_l.obj')
		use_material('e_glow2')
		texture('c_glow.png')
		load_obj('c_e_glow_l.obj')

		if lod > 1 then
			set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('nazzle')
			call_model('nazzle_n', v(-31.58,-8.9,19), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(-31.58,-13.4,19), v(1,0,0), v(0,-1,0), .3)
			call_model('nazzle_n', v(-29.33,-11.16,19), v(0,1,0), v(1,0,0), .3)
			call_model('nazzle_n', v(-33.83,-11.16,19), v(0,1,0), v(-1,0,0), .3)
		end

		local LeftBackTopThrust = v(-31.58, -8.7, 19)
		local LeftBackBottomThrust = v(-31.58, -13.6, 19)

		thruster(LeftBackTopThrust, v(0,1,0), 5)
		thruster(LeftBackBottomThrust, v(0,-1,0), 5)

		local BackLeftThrust = v(-34.03, -11.16, 19)
		local BackRightThrust = v(-29.13, -11.16, 19)

		thruster(BackLeftThrust, v(-1,0,0), 5)
		thruster(BackRightThrust, v(1,0,0), 5)

	end,
	dynamic = function(lod)
		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
		set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }))

		if lod > 1 then
			call_model('posl_red', v(-34,-11.16,7), v(0,0,1), v(-1,0.0),2.5)
			call_model('coll_warn', v(-31.58,-13.6,7), v(1,0,0), v(0,-1,0),2.5)
		end

		local rot = 1.7*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, .3)
		call_model('courier_flap_ll', v(-32.394, -13.175,13.134), v(math.cos(-rot),math.sin(-rot),0), v(-0.024*rot,0.034*rot,1),1)
		call_model('courier_flap_lr', v(-30.756, -13.175,13.134), v(math.cos(rot),math.sin(rot),0), v(-0.024*rot,-0.034*rot,-1),1)

		local factor = get_time('SECONDS')*math.pi
		call_model('courier_tip', v(-31.58, -11.16, 6.1), v(math.sin(factor*1.5),math.cos(factor*1.5),0), v(0,0,-1),1)
	end
})

define_model('courier_eng_r', {      -- engine part right all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 45,
		materials={'courier', 'e_glow1', 'e_glow2', 'iron', 'nazzle'},
	},
	static = function(lod)
		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('c_eng_r.obj')

		set_material('iron', .4,.45,.55,1,.2,.2,.2,10)
		use_material('iron')
		texture('c_metal.png')
		load_obj('c_cage_r.obj')

		use_material('e_glow1')
		texture('c_e_inside.png')
		load_obj('c_e_inside_r.obj')
		use_material('e_glow2')
		texture('c_glow.png')
		load_obj('c_e_glow_r.obj')

		if lod > 1 then
			set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('nazzle')
			call_model('nazzle_n', v(31.58,-8.9,19), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(31.58,-13.4,19), v(1,0,0), v(0,-1,0), .3)
			call_model('nazzle_n', v(29.33,-11.16,19), v(0,1,0), v(-1,0,0), .3)
			call_model('nazzle_n', v(33.83,-11.16,19), v(0,1,0), v(1,0,0), .3)
		end

		local LeftBackTopThrust = v(31.58, -8.7, 19)
		local LeftBackBottomThrust = v(31.58, -13.6, 19)

		thruster(LeftBackTopThrust, v(0,1,0), 5)
		thruster(LeftBackBottomThrust, v(0,-1,0), 5)

		local BackLeftThrust = v(34.03, -11.16, 19)
		local BackRightThrust = v(29.13, -11.16, 19)

		thruster(BackLeftThrust, v(1,0,0), 5)
		thruster(BackRightThrust, v(-1,0,0), 5)

	end,
	dynamic = function(lod)
		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
		set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }))

		if lod > 1 then
			call_model('posl_green', v(34,-11.16,7), v(0,0,1), v(1,0.0),2.5)
			call_model('coll_warn', v(31.58,-13.6,7), v(1,0,0), v(0,-1,0),2.5)
		end

		local rot = 1.7*math.pi*math.clamp(get_animation_position('WHEEL_STATE'), 0, .3)
		call_model('courier_flap_rl', v(30.756, -13.175,13.134), v(math.cos(-rot),math.sin(-rot),0), v(-0.024*rot,0.034*rot,1),1)
		call_model('courier_flap_rr', v(32.394, -13.175,13.134), v(math.cos(rot),math.sin(rot),0), v(-0.024*rot,-0.034*rot,-1),1)

		local factor = get_time('SECONDS')*math.pi
		call_model('courier_tip', v(31.58, -11.16, 6.1), v(math.cos(factor*1.5),math.sin(factor*1.5),0), v(0,0,-1),1)
	end
})

define_model('trader_eng', {   -- trader middle engine part
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 35,
		materials = {'courier', 'e_glow1', 'e_glow2'},
	},
	static = function(lod)

		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('courier')
		texture('c_shell_al.png')
		load_obj('t_eng.obj')

		use_material('e_glow2')
		texture('c_e_inside.png')
		load_obj('t_e_inside.obj')

		use_material('e_glow1')
		texture('c_glow.png')
		load_obj('t_e_glow.obj')

		call_model('courier_tip', v(0,11.466,13.1), v(0,1,0), v(0,0,-1), 1)
	end,
	dynamic = function(lod)
		set_material('e_glow1', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
		set_material('e_glow2', lerp_materials(get_time('SECONDS')*0.5, {0, 0, 0, 1, 0, 0, 0, 0, 1, .7, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }))
	end
})

define_model('courier_sub', {     -- courier sub-model, all models
	info = {
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 46,
		materials = {'courier', 'matte', 'win', 'nazzle'},
	},

	static = function(lod)

		set_material('courier', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('matte', .3, .3, .3,1, .3, .3, .3, 10)
		set_material('win', 0,0,0,1,1,1,1.2,100)

		use_material('courier')

		texture('c_chrome.png')
		load_obj('c_chrome.obj')

		use_material('matte')
		texture('c_back.png')

		load_obj('c_back.obj')

		texture(nil)
		use_material('win')
		load_obj('c_win.obj')

		if lod > 1 then
			set_material('nazzle', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('nazzle')
			call_model('nazzle_n', v(-3.5,1.2,-15), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(-3.5,.5,-15), v(1,0,0), v(0,-1,0), .3)

			call_model('nazzle_n', v(3.5,1.2,-15), v(1,0,0), v(0,1,0), .3)
			call_model('nazzle_n', v(3.5,.5,-15), v(1,0,0), v(0,-1,0), .3)

			call_model('nazzle_n', v(-4,.82,-15), v(0,1,0), v(-1,0,0), .3)
			call_model('nazzle_n', v(4,.82,-15), v(0,1,0), v(1,0,0), .3)
		end

		local LeftFrontTopThrust = v(-3.5,1.4,-15)
		local RightFrontTopThrust = v(3.5,1.4, -15)
		local LeftFrontBottomThrust = v(-3.5,.3, -15)
		local RightFrontBottomThrust = v(3.5,.3, -15)

		thruster(LeftFrontTopThrust, v(0,1,0), 5)
		thruster(RightFrontTopThrust, v(0,1,0), 5)
		thruster(LeftFrontBottomThrust, v(0,-1,0), 5)
		thruster(RightFrontBottomThrust, v(0,-1,0), 5)

		local FrontLeftThrust = v(-4.2,.82, -15)
		local FrontRightThrust = v(4.2,.82, -15)

		thruster(FrontLeftThrust, v(-1,0,0), 5)
		thruster(FrontRightThrust, v(1,0,0), 5)

	end,

	dynamic = function(lod)

		local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
		local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
		local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

		call_model('courier_eng_l', v(trans1*9.5,trans1*8,trans1*-5), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp5', v(trans1*9.5,trans1*7.7,trans1*-4.5), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp4', v(trans1*7.3,trans1*5.9,trans1*-3.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp3', v(trans1*4.95,trans1*4,trans1*-2.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_lwp2', v(trans1*2.4,trans1*1.95,trans1*-1.15), v(1,0,0), v(0,1,0),1)

		call_model('courier_eng_r', v(trans1*-9.5,trans1*8,trans1*-5), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp5', v(trans1*-9.5,trans1*7.7,trans1*-4.5), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp4', v(trans1*-7.3,trans1*5.9,trans1*-3.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp3', v(trans1*-4.95,trans1*4,trans1*-2.4), v(1,0,0), v(0,1,0),1)
		call_model('courier_rwp2', v(trans1*-2.4,trans1*1.95,trans1*-1.15), v(1,0,0), v(0,1,0),1)

		use_material('courier')
		texture('models/ships/courier/c_metal.png')
		ring(3*lod, v(-31.58+trans1*9.5,-11.5+trans1*8,16-trans1*5), v(-31.58+trans1*9.5,-13.5+trans2*3+trans3*5,20.5-trans2*15-trans3*5), v(-trans2*3,1,0),0.2)
		ring(3*lod, v(-31.58+trans1*9.5,-11.5+trans1*8,9-trans1*5), v(-31.58+trans1*9.5,-13.5+trans2*3+trans3*5,20.5-trans2*15-trans3*5), v(trans2*3,1,0),0.2)

		ring(3*lod, v(31.58-trans1*9.5,-11.5+trans1*8,16-trans1*5), v(31.58-trans1*9.5,-13.5+trans2*3+trans3*5,20.5-trans2*15-trans3*5), v(-trans2*3,1,0),0.2)
		ring(3*lod, v(31.58-trans1*9.5,-11.5+trans1*8,9-trans1*5), v(31.58-trans1*9.5,-13.5+trans2*3+trans3*5,20.5-trans2*15-trans3*5), v(trans2*3,1,0),0.2)
		texture(nil)
		call_model('courier_wheels_l', v(trans1*9.5,2.5+trans2*3+trans3*5,14.5-trans2*15-trans3*5), v(1,0,0), v(0,1,0), 1)
		call_model('courier_wheels_r', v(trans1*-9.5,2.5+trans2*3+trans3*5,14.5-trans2*15-trans3*5), v(1,0,0), v(0,1,0), 1)

		if lod > 1 then
			call_model('headlight', v(0,.48,-23), v(1,0,0), v(0,-1,-.15),2)
			call_model('coll_warn', v(0,-1.85,14), v(1,0,0), v(0,-1,0),2)
		end
	end
})

define_model('courier', {
	info = {
		scale = 1.2,   --1.5 = ffed3d export scale, mentioned for trader?
		lod_pixels = {1, 50, 300, 0},
		bounding_radius = 56,
		tags = {'ship'},
		ship_defs = {
			{
				name='Imperial Courier',
				forward_thrust = -50e6,
				reverse_thrust = 15e6,
				up_thrust = 15e6,
				down_thrust = -8e6,
				left_thrust = -8e6,
				right_thrust = 8e6,
				angular_thrust = 110e6,
				gun_mounts =
				{
					{ v(0,0.6,-25), v(0,0,-1) },
					{ v(0,0,16), v(0,0,1) },
				},
				max_cargo = 300,
				max_laser = 2,
				max_missile = 6,
				max_cargoscoop = 0,
				capacity = 300,
				hull_mass = 200,
				fuel_tank_mass = 100,
				thruster_fuel_use = 0.0002,
				price = 611000,
				hyperdrive_class = 4,
			}
		}
	},
	static = function(lod)
		if lod == 1 then
			local  v1 = v(0,1,-24)
			local v1a = v(7,1,-8)
			local v1b = v(-7,1,-8)
			local  v2 = v(16,1,1)
			local  v3 = v(16,1,7)
			local v3a = v(7,2.2,13)
			local  v4 = v(5,-1,14)
			local  v5 = v(-5,-1,14)
			local  v6 = v(-16,1,7)
			local  v7 = v(-16,1,1)
			local  v8 = v(0,0,0)
			local  v9 = v(0,5.5,-4)
			local v9a = v(6,2.2,0)
			local v10 = v(0,6.5,17)

			xref_quad(v8,v7,v1b,v1)
			xref_quad(v8,v5,v6,v7)
			tri(v8,v4,v5)
			tri(v10,v5,v4)
			xref_tri(v1,v9,v9a)
			xref_tri(v1,v9a,v1a)
			xref_quad(v1a,v9a,v2,v1a)
			xref_quad(v9,v10,v3a,v9a)
			xref_quad(v9a,v3a,v3,v2)
			xref_tri(v10,v4,v3a)
			xref_tri(v3a,v4,v3)
		end

		if lod > 1 then
			call_model('courier_body', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('courier_sub', v(0,0,0), v(1,0,0), v(0,1,0),1)
			if lod > 2 then
				call_model('posl_white', v(0,6.6,13), v(1,0,0), v(0,1,0),2.5)
			end
		end
	end,
	dynamic = function(lod)
		if lod == 1 then
			local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
			local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
			local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

			local v20 = v(16,1,1)
			local v21 = v(16,1,7)
			local v22 = v(30-trans1*9.5,-10+trans1*7.7,9-trans1*4.5)
			local v23 = v(30-trans1*9.5,-10+trans1*7.7,12-trans1*4.5)
			local v24 = v(31.6-trans1*9.5,-11.2+trans1*8,-18-trans1*5) -- new
			local v25 = v(31.6-trans1*9.5,-11.2+trans1*8,-7-trans1*5)
			local v26 = v(31.6-trans1*9.5,-11.2+trans1*8,2.7-trans1*5)
			local v27 = v(31.6-trans1*9.5,-11.2+trans1*8,6.3-trans1*5) -- new
			local v28 = v(31.6-trans1*9.5,-11.2+trans1*8,21.5-trans1*5)
			local v29 = v(31-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v30 = v(32-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v31 = v(32-trans1*9.5,-12+trans1*3.5,10-trans1*4)
			local v32 = v(31-trans1*9.5,-12+trans1*3.5,10-trans1*4)

			xref_quad(v20,v21,v23,v22)
			xref_quad(v20,v22,v23,v21)
			xref_tapered_cylinder(3, v24, v25, v(0,1,0), .1, .6)
			xref_tapered_cylinder(3, v25, v26, v(0,1,0), .6, 2)
			xref_tapered_cylinder(3, v26, v27, v(0,1,0), 2, 1.5)
			xref_tapered_cylinder(3, v27, v28, v(0,1,0), 1.3, .9)
			xref_quad(v29,v30,v31,v32)
		end
	end
})

define_model('trader', {
	info = {
		scale = 1.1,
		lod_pixels = { 1, 50, 300, 0 },
		bounding_radius = 55,
		tags = {'ship'},
		ship_defs = {
			{
				name='Imperial Trader',
				forward_thrust = -8e7,
				reverse_thrust = 3e7,
				up_thrust = 3e7,
				down_thrust = -1e7,
				left_thrust = -1e7,
				right_thrust = 1e7,
				angular_thrust = 15e7,
				gun_mounts =
				{
					{ v(0,0.6,-36), v(0,0,-1) },
					{ v(0,0,22), v(0,0,1) },
				},
				max_cargo = 450,
				max_laser = 2,
				max_missile = 6,
				max_cargoscoop = 0,
				capacity = 450,
				hull_mass = 300,
				fuel_tank_mass = 150,
				thruster_fuel_use = 0.0002,
				price = 954000,
				hyperdrive_class = 5,
			}
		}
	},

	static = function(lod)
		if lod == 1 then
			local  v1 = v(0,1,-24)
			local v1a = v(7,1,-8)
			local v1b = v(-7,1,-8)
			local  v2 = v(16,1,1)
			local  v3 = v(16,1,7)
			local v3a = v(7,2.2,13)
			local  v4 = v(5,-1,14)
			local  v5 = v(-5,-1,14)
			local  v6 = v(-16,1,7)
			local  v7 = v(-16,1,1)
			local  v8 = v(0,0,0)
			local  v9 = v(0,5.5,-4)
			local v9a = v(6,2.2,0)
			local v10 = v(0,6.5,17)
			local v11 = v(0,11.5,-9)
			local v12 = v(0,11.5,13)
			local v13 = v(0,11.5,28)
			local v14 = v(0,6,14)  -- trader
			local v15 = v(0,10,18) -- trader
			local v16 = v(0,10,20) -- trader

			xref_quad(v8,v7,v1b,v1)
			xref_quad(v8,v5,v6,v7)
			tri(v8,v4,v5)
			tri(v10,v5,v4)
			xref_tri(v1,v9,v9a)
			xref_tri(v1,v9a,v1a)
			xref_quad(v1a,v9a,v2,v1a)
			xref_quad(v9,v10,v3a,v9a)
			xref_quad(v9a,v3a,v3,v2)
			xref_tri(v10,v4,v3a)
			xref_tri(v3a,v4,v3)
			tapered_cylinder(3, v11, v12, v(0,1,0), 0.5, 3.5)
			tapered_cylinder(3, v12, v13, v(0,1,0), 2.5, 1.5)
			xref_quad(v10,v14,v15,v16)
		end

		if lod > 1 then
			call_model('trader_body', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('trader_eng', v(0,0,0), v(1,0,0), v(0,1,0),1)
			call_model('courier_sub', v(0,0,0), v(1,0,0), v(0,1,0),1)
			if lod > 2 then
				call_model('posl_white', v(0,13.55,20), v(1,0,0), v(0,1,0),2.5)
			end
		end
	end,

	dynamic = function(lod)
		if lod == 1 then
			local trans1 = math.clamp(get_animation_position('WHEEL_STATE'), 0, 1)
			local trans2 = math.clamp(get_animation_position('WHEEL_STATE'), .5, 1)
			local trans3 = math.clamp(get_animation_position('WHEEL_STATE'), 0, .5)

			local v20 = v(16,1,1)
			local v21 = v(16,1,7)
			local v22 = v(30-trans1*9.5,-10+trans1*7.7,9-trans1*4.5)
			local v23 = v(30-trans1*9.5,-10+trans1*7.7,12-trans1*4.5)
			local v24 = v(31.5-trans1*9.5,-11+trans1*8,-17-trans1*5)
			local v25 = v(31.5-trans1*9.5,-11+trans1*8,6-trans1*5)
			local v26 = v(31.5-trans1*9.5,-11+trans1*8,21-trans1*5)
			local v27 = v(31-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v28 = v(32-trans1*9.5,-12+trans1*3.5,2-trans1*4)
			local v29 = v(32-trans1*9.5,-12+trans1*3.5,10-trans1*4)
			local v30 = v(31-trans1*9.5,-12+trans1*3.5,10-trans1*4)

			xref_quad(v20,v21,v23,v22)
			xref_quad(v20,v22,v23,v21)
			xref_tapered_cylinder(3, v24, v25, v(0,1,0), 0.5, 3.5)
			xref_tapered_cylinder(3, v25, v26, v(0,1,0), 2.5, 1.5)
			xref_quad(v27,v28,v29,v30)
		end

	end
})
