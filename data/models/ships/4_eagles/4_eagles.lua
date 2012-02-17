define_model('eagle_gun', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 11,
		materials = {'d_red', 'black', 'chrome'},
	},
	static = function(lod)
		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('d_red', .3,0,0,1,.3,.3,.3,50)

		use_material('chrome')
		texture('tex8.png')
		tapered_cylinder(4*lod,v(0,0,0),v(0,0,-9),v(0,1,0),.3,.2)
		use_material('d_red')
		sphere_slice(3*lod,2*lod, 0, .5*math.pi, Matrix.rotate(math.pi,v(1,0,0))*Matrix.scale(v(1,1,1.5)))
		if lod > 2 then
			use_material('black')
			texture(nil)
			zbias(1,v(0,0,-9), v(0,0,-1))
			circle(4*lod,v(0,0,-9),v(0,0,-1),v(0,1,0),.15)
			zbias(0)
		end
	end
})

define_model('eagle_lrf_v0', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 51,
		materials = {'top'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v2 = v(0,6,25)
		local v6 = v(31,0,-9)
		local v8 = v(3,0,12)
		local v10 = v(9,0,-37)
		local v14 = v(31,1,9)
		texture('tex1.png', v(.5,.222,0), v(.0165,0,0), v(0,0,-1.2))
		xref_flat(6*lod, v(0,1,0), {v2}, {v14}, {v6}, {v8, v10, v0})  -- top
	end
})

define_model('eagle_lrf_v1', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 51,
		materials = {'top'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v2 = v(0,6,25)
		local v6 = v(31,0,-9)
		local v8 = v(3,0,12)
		local v10 = v(9,0,-37)
		local v14 = v(31,1,9)
		texture('tex1c.png', v(.5,.222,0), v(.0165,0,0), v(0,0,-1.2))
		xref_flat(6*lod, v(0,1,0), {v2}, {v14}, {v6}, {v8, v10, v0})  -- top
	end
})

define_model('eagle_lrf_body', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 51,
		materials = {'top', 'bot', 'black', 'steel'},
	},

	static = function(lod)
		local v0 = v(0,0,-40)
		local v4 = v(0,-6,25)
		local v7 = v(-31,0,-9)
		local v9 = v(-3,0,12)
		local v11 = v(-9,0,-37)
		local v25 = v(-31,-1,9)
		local v26 = v(14,1.5,5)
		local v28 = v(14,3,9)
		local v30 = v(17,1.5,4)
		local v32 = v(17,2.7,8)
		local v34 = v(20,1.8,3)
		local v36 = v(20,2.3,7)

		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('steel', .2,.23,.25,1,.35,.38,.4,10)

		if lod > 1 then
			use_material('bot')
			texture('tex8.png', v(0,0,0), v(0,.2,0), v(0,0,1))
			xref_tapered_cylinder(3*lod, v26, v28, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v30, v32, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v34, v36, v(0,1,-.1), 0.2, 1)
			texture(nil)
			use_material('black')
			xref_circle(3*lod, v28+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v32+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v36+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
		end

		texture('tex0.png', v(.5,.655,0), v(.0165,0,0), v(0,0,1.05))
		use_material('bot')
		xref_flat(6*lod, v(0,-1,0), {v4}, {v25}, {v7}, {v9, v11, v0}) -- bottom

	end,

	dynamic = function(lod)
		--select2 = 20
		selector2()
		if select2 < 26 then  -- plain green with light blue bottom
			set_material('top', .1,.3,.05,1,.4,.4,.4,50)
			set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('top')
			call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			if select2 < 51 then
				set_material('top', .1,.3,.05,1,.4,.4,.4,50)
				set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
				use_material('top')
				call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if select2 < 76 then -- cv
					set_material('top', get_arg_material(1))
					set_material('bot', get_arg_material(0))
					use_material('top')
					call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
				else
					if select2 > 75 then
						set_material('top', get_arg_material(1))
						set_material('bot', get_arg_material(0))
						use_material('top')
						call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
					end
				end
			end
		end
		if lod > 1 then
			use_material('steel')
			selector3()
			if select3 < 51 then
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_-', v(9,1.2,2), v(1,0,0), v(.1,1,-.2), 3)
					call_model('antenna_1', v(-18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(-6,.407,-20), v(0,1,0))
					call_model('decal', v(-6,.407,-20), v(0,1,-.07), v(1,.349,0), 3)
					zbias(0)
				end
			else
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_+', v(-9,1.2,2), v(1,0,0), v(-.1,1,-.2), 3)
					call_model('antenna_1', v(18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(6,.407,-20), v(0,1,0))
					call_model('decal', v(6,.407,-20), v(0,1,-.07), v(-1,.349,0), 3)
					zbias(0)
				end
			end
		end

		local  R_T = v(31,0,-25.6)
		local RF_T = v(10.1,0,-14)
		local LF_T = v(-10.1,0,-14)
		local TF_T = v(8.5,1.6,-14)
		local BF_T = v(8.5,-1.6,-14)
		local RB_T = v(32.5,0,16)
		local LB_T = v(-32.5,0,16)
		local TB_T = v(31,1.8,16)
		local BB_T = v(31,-1.8,16)

		xref_thruster(R_T, v(0,0,-1), 10, true)
		thruster(RF_T, v(1,0,0), 8)
		thruster(LF_T, v(-1,0,0), 8)
		xref_thruster(TF_T, v(0,1,0), 8)
		xref_thruster(BF_T, v(0,-1,0), 8)
		thruster(RB_T, v(1,0,0), 8)
		thruster(LB_T, v(-1,0,0), 8)
		xref_thruster(TB_T, v(0,1,0), 8)
		xref_thruster(BB_T, v(0,-1,0), 8)

	end
})

define_model('eagle_mk2_body', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 51,
		materials = {'top', 'bot', 'black', 'steel'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v4 = v(0,-6,25)
		local v7 = v(-31,0,-9)
		local v9 = v(-3,0,12)
		local v11 = v(-9,0,-37)
		local v14 = v(31,1,9)
		local v25 = v(-31,-1,9)
		local v26 = v(14,1.5,5)
		local v28 = v(14,3,9)
		local v30 = v(17,1.5,4)
		local v32 = v(17,2.7,8)
		local v34 = v(20,1.8,3)
		local v36 = v(20,2.3,7)

		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('steel', .2,.23,.25,1,.35,.38,.4,10)

		if lod > 1 then
			use_material('bot')
			texture('tex8.png', v(0,0,0), v(0,.2,0), v(0,0,1))
			xref_tapered_cylinder(3*lod, v26, v28, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v30, v32, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v34, v36, v(0,1,-.1), 0.2, 1)
			use_material('black')
			texture(nil)
			xref_circle(3*lod, v28+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v32+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v36+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
		end

		texture('tex0.png', v(.5,.655,0), v(.0165,0,0), v(0,0,1.05))
		use_material('bot')
		xref_flat(6*lod, v(0,-1,0), {v4}, {v25}, {v7}, {v9, v11, v0}) -- bottom
	end,

	dynamic = function(lod)
		--select2 = 10
		selector2()
		if select2 < 34 then  -- plain silver blue
			set_material('top', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('top')
			call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			if select2 < 67  then -- cv
				set_material('top', get_arg_material(0))
				set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
				use_material('top')
				call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if select2 > 66  then -- cv
					set_material('top', get_arg_material(0))
					set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
					use_material('top')
					call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
			end
		end

		if lod > 1 then
			use_material('steel')
			selector3()
			if select3 < 51 then
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_-', v(9,1.2,2), v(1,0,0), v(.1,1,-.2), 3)
					call_model('antenna_1', v(-18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(-6,.407,-20), v(0,1,0))
					call_model('decal', v(-6,.407,-20), v(0,1,-.07), v(1,.349,0), 3)
					zbias(0)
				end
			else
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_+', v(-9,1.2,2), v(1,0,0), v(-.1,1,-.2), 3)
					call_model('antenna_1', v(18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(6,.407,-20), v(0,1,0))
					call_model('decal', v(6,.407,-20), v(0,1,-.07), v(-1,.349,0), 3)
					zbias(0)
				end
			end
		end

		local R_T = v(8.8,0,-15.8)
		local RF_T = v(10.1,0,-14)
		local LF_T = v(-10.1,0,-14)
		local TF_T = v(0,3.6,-14)
		local BF_T = v(0,-3.6,-14)
		local RB_T = v(33.8,0,8)
		local LB_T = v(-33.8,0,8)
		local TB_T = v(32,1.8,8)
		local BB_T = v(32,-1.8,8)

		xref_thruster(R_T, v(0,0,-1), 8, true)
		thruster(RF_T, v(1,0,0), 8)
		thruster(LF_T, v(-1,0,0), 8)
		thruster(TF_T, v(0,1,0), 8)
		thruster(BF_T, v(0,-1,0), 8)
		thruster(RB_T, v(1,0,0), 8)
		thruster(LB_T, v(-1,0,0), 8)
		xref_thruster(TB_T, v(0,1,0), 8)
		xref_thruster(BB_T, v(0,-1,0), 8)
	end
})

define_model('eagle_mk3_body', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 51,
		materials = {'top', 'black', 'steel'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v4 = v(0,-6,25)
		local v7 = v(-31,0,-9)
		local v9 = v(-3,0,12)
		local v11 = v(-9,0,-37)
		local v14 = v(31,1,9)
		local v25 = v(-31,-1,9)
		local v26 = v(14,1.5,5)
		local v28 = v(14,3,9)
		local v30 = v(17,1.5,4)
		local v32 = v(17,2.7,8)
		local v34 = v(20,1.8,3)
		local v36 = v(20,2.3,7)

		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('steel', .2,.23,.25,1,.35,.38,.4,10)

		use_material('top')
		if lod > 1 then
			texture('tex8.png', v(0,0,0), v(0,.2,0), v(0,0,1))
			xref_tapered_cylinder(3*lod, v26, v28, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v30, v32, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v34, v36, v(0,1,-.1), 0.2, 1)
			use_material('black')
			texture(nil)
			xref_circle(3*lod, v28+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v32+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v36+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
		end

		use_material('top')
		texture('tex0.png', v(.5,.655,0), v(.0165,0,0), v(0,0,1.05))
		xref_flat(6*lod, v(0,-1,0), {v4}, {v25}, {v7}, {v9, v11, v0}) -- bottom
	end,

	dynamic = function(lod)
		--select2 = 30
		selector2()
		if select2 < 34 then  -- plain night black
			set_material('top', .02,0,.1,1,.2,.2,.4,50)
			use_material('top')
			call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			if select2 < 67 then -- cv
				set_material('top', get_arg_material(0))
				use_material('top')
				call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if select2 > 66 then -- cv
					set_material('top', get_arg_material(0))
					use_material('top')
					call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				end
			end
		end

		if lod > 1 then
			use_material('steel')
			selector3()
			if select3 < 51 then
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_-', v(9,1.2,2), v(1,0,0), v(.1,1,-.2), 3)
					call_model('antenna_1', v(-18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(-6,.407,-20), v(0,1,0))
					call_model('decal', v(-6,.407,-20), v(0,1,-.07), v(1,.349,0), 3)
					zbias(0)
				end
			else
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_+', v(-9,1.2,2), v(1,0,0), v(-.1,1,-.2), 3)
					call_model('antenna_1', v(18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(6,.407,-20), v(0,1,0))
					call_model('decal', v(6,.407,-20), v(0,1,-.07), v(-1,.349,0), 3)
					zbias(0)
				end
			end
		end

		local M_T2 = v(33,0,18.2)
		local R_T  = v(33,0,-25.2)
		local RF_T = v(35.6,0,-22)
		local LF_T = v(-35.6,0,-22)
		local TF_T = v(33,2.6,-22)
		local BF_T = v(33,-2.6,-22)
		local RB_T = v(35.6,0,15)
		local LB_T = v(-35.6,0,15)
		local TB_T = v(33,2.6,15)
		local BB_T = v(33,-2.6,15)

		xref_thruster(M_T2, v(0,0,1), 15, true)
		xref_thruster(R_T, v(0,0,-1), 15, true)
		thruster(RF_T, v(1,0,0), 9)
		thruster(LF_T, v(-1,0,0), 9)
		xref_thruster(TF_T, v(0,1,0), 9)
		xref_thruster(BF_T, v(0,-1,0), 9)
		thruster(RB_T, v(1,0,0), 9)
		thruster(LB_T, v(-1,0,0), 9)
		xref_thruster(TB_T, v(0,1,0), 9)
		xref_thruster(BB_T, v(0,-1,0), 9)
	end
})

define_model('emk4_extras_v0', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 46,
		materials = {'top'},
	},
	static = function(lod)
		local v40 = v(8,4,20)
		local v42 = v(8,2.5,7)
		local v44 = v(10,10,20)
		local v46 = v(10,10,12)
		local v48 = v(8.5,4,20)
		local v50 = v(8.5,2.5,7)
		local v52 = v(10.5,10,20)
		local v54 = v(10.5,10,12)
		-- mk4 fin
		texture('tex12.png', v(.5,.5,0), v(0,.1,0), v(0,0,.5))
		xref_quad(v42,v40,v44,v46)
		xref_quad(v50,v54,v52,v48)
		xref_quad(v40,v48,v52,v44)
		xref_quad(v42,v46,v54,v50)
		-- mk4 tank tips
		texture('tex12.png', v(.5,.5,0), v(.05,0,0), v(0,.05,0))
		sphere_slice(4*lod,3*lod, .4, .5*math.pi, Matrix.translate(v(34.9,0,-15.8))
		* Matrix.rotate(0.5*math.pi, v(-1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, .4, .5*math.pi, Matrix.translate(v(-34.9,0,-15.8))
		* Matrix.rotate(0.5*math.pi, v(-1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, 0, .5*math.pi, Matrix.translate(v(34.9,0,15.8))
		* Matrix.rotate(0.5*math.pi, v(1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, 0, .5*math.pi, Matrix.translate(v(-34.9,0,15.8))
		* Matrix.rotate(0.5*math.pi, v(1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))
	end
})

define_model('emk4_extras_v1', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 46,
		materials = {'top'},
	},
	static = function(lod)
		local v40 = v(8,4,20)
		local v42 = v(8,2.5,7)
		local v44 = v(10,10,20)
		local v46 = v(10,10,12)
		local v48 = v(8.5,4,20)
		local v50 = v(8.5,2.5,7)
		local v52 = v(10.5,10,20)
		local v54 = v(10.5,10,12)
		-- mk4 fin
		texture('tex1e.png', v(.5,.5,0), v(0,.1,0), v(0,0,.5))
		xref_quad(v42,v40,v44,v46)
		xref_quad(v50,v54,v52,v48)
		xref_quad(v40,v48,v52,v44)
		xref_quad(v42,v46,v54,v50)
		-- mk4 tank tips
		texture('tex1e.png', v(.5,.5,0), v(.05,0,0), v(0,.05,0))
		sphere_slice(4*lod,3*lod, .4, .5*math.pi, Matrix.translate(v(34.9,0,-15.8))
		* Matrix.rotate(0.5*math.pi, v(-1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, .4, .5*math.pi, Matrix.translate(v(-34.9,0,-15.8))
		* Matrix.rotate(0.5*math.pi, v(-1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, 0, .5*math.pi, Matrix.translate(v(34.9,0,15.8))
		* Matrix.rotate(0.5*math.pi, v(1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))

		sphere_slice(4*lod,3*lod, 0, .5*math.pi, Matrix.translate(v(-34.9,0,15.8))
		* Matrix.rotate(0.5*math.pi, v(1,0,0)) * Matrix.rotate(math.pi/16, v(0,1,0)) * Matrix.scale(v(4.1,5,4.1)))
	end
})

define_model('eagle_mk4_body', {
	info = {
		lod_pixels = {5,10,100,0},
		bounding_radius = 57,
		materials = {'top', 'bot', 'tank', 'black', 'steel'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v4 = v(0,-6,25)
		local v7 = v(-31,0,-9)
		local v9 = v(-3,0,12)
		local v11 = v(-9,0,-37)
		local v14 = v(31,1,9)
		local v25 = v(-31,-1,9)
		local v26 = v(14,1.5,5)
		local v28 = v(14,3,9)
		local v30 = v(17,1.5,4)
		local v32 = v(17,2.7,8)
		local v34 = v(20,1.8,3)
		local v36 = v(20,2.3,7)
		local v81 = v(34.9,0,-16)
		local v83 = v(34.9,0,16)

		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('steel', .2,.23,.25,1,.35,.38,.4,10)

		use_material('tank')
		if lod > 1 then
			texture('tex8.png', v(0,0,0), v(0,.2,0), v(0,0,1))
			xref_tapered_cylinder(3*lod, v26, v28, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v30, v32, v(0,1,-.1), 0.2, 1)
			xref_tapered_cylinder(3*lod, v34, v36, v(0,1,-.1), 0.2, 1)
			use_material('black')
			texture(nil)
			xref_circle(3*lod, v28+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v32+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
			xref_circle(3*lod, v36+v(0,0,.001), v(0,0,1), v(0,1,-.1), .8)
		end

		-- mk4 scoop
		use_material('top')
		texture('tex12.png', v(.5,.5,0), v(.1,0,0), v(0,0,.5))
		cubic_bezier_quad(2*lod,2*lod,  v(5,-1.5,-8), v(5,-2,-4), v(3,-2,5), v(1,-4,10),
		v(3,-6,-8), v(4,-8,-4), v(2,-8,5), v(0.5,-4,10),
		v(-3,-6,-8), v(-4,-8,-4), v(-2,-8,5), v(-0.5,-4,10),
		v(-5,-1.5,-8), v(-5,-2,-4), v(-3,-2,5), v(-1,-4,10))
		if lod > 2 then
			cubic_bezier_quad(2*lod,2*lod,  v(-5,-1.5,-8), v(-5,-2,-4), v(-3,-2,5), v(-1,-4,10),
			v(-3,-6,-8), v(-4,-7,-4), v(-2,-7,5), v(-0.5,-3,10),
			v(3,-6,-8), v(4,-7,-4), v(2,-7,5), v(0.5,-3,10),
			v(5,-1.5,-8), v(5,-2,-4), v(3,-2,5), v(1,-4,10))
		end
		-- mk4 side tanks
		use_material('tank')
		texture('tex1.png', v(.5,.17,0), v(.0165,0,0), v(0,0,-.75))
		xref_ring(4*lod,v83, v81, v(0,1,0), 4)

		-- eagle body
		texture('tex0.png', v(.5,.655,0), v(.0165,0,0), v(0,0,1.05))
		use_material('bot')
		xref_flat(6*lod, v(0,-1,0), {v4}, {v25}, {v7}, {v9, v11, v0}) -- bottom
	end,

	dynamic = function(lod)
		select2 = 90
		--selector2()
		if select2 < 17 then  -- dark green with black bottom
			set_material('top', .04,.2,.02,1,.3,.3,.4,50)
			set_material('bot', .02,0,.1,1,.2,.2,.3,50)
			set_material('tank', .02,0,.1,1,.2,.2,.3,50)
			use_material('top')
			call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
			call_model('emk4_extras_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			if select2 < 34 then  -- dark green with black bottom
				set_material('top', .05,.3,.02,1,.3,.3,.4,50)
				set_material('bot', .02,0,.1,1,.2,.2,.3,50)
				set_material('tank', .02,0,.1,1,.2,.2,.3,50)
				use_material('top')
				call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
				call_model('emk4_extras_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if select2 < 51 then   -- chrome with green top
					set_material('top', .1,.3,.05,1,.4,.4,.4,50)
					set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
					set_material('tank', .63,.7,.83,1,1.26,1.4,1.66,30)
					use_material('top')
					call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
					call_model('emk4_extras_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
				else
					if select2 < 67 then   -- chrome with green top
						set_material('top', .1,.3,.05,1,.4,.4,.4,50)
						set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
						set_material('tank', .63,.7,.83,1,1.26,1.4,1.66,30)
						use_material('top')
						call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
						call_model('emk4_extras_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
					else
						if select2 < 83  then -- cv
							set_material('top', get_arg_material(0))
							set_material('tank', get_arg_material(1))
							set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)
							use_material('top')
							call_model('eagle_lrf_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
							call_model('emk4_extras_v0',v(0,0,0),v(1,0,0),v(0,1,0),1)
						else
							if select2 > 83  then -- cv
								set_material('top', get_arg_material(0))
								set_material('tank', get_arg_material(1))
								set_material('bot', .63,.7,.83,1,1.26,1.4,1.66,30)   -- old light blue  .4,.55,.6,1,.4,.55,.6,50)
								use_material('top')
								call_model('eagle_lrf_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
								call_model('emk4_extras_v1',v(0,0,0),v(1,0,0),v(0,1,0),1)
							end
						end
					end
				end
			end
		end

		if lod > 1 then
			use_material('steel')
			selector3()
			if select3 < 51 then
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_-', v(9,1.2,2), v(1,0,0), v(.1,1,-.2), 3)
					call_model('antenna_1', v(-18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(-6,.407,-20), v(0,1,0))
					call_model('decal', v(-6,.407,-20), v(0,1,-.07), v(1,.349,0), 3)
					zbias(0)
				end
			else
				if get_equipment('SCANNER') == 'SCANNER' then
					call_model('scanner_+', v(-9,1.2,2), v(1,0,0), v(-.1,1,-.2), 3)
					call_model('antenna_1', v(18,0,-4), v(1,0,0), v(0,1,0), 4)
				end

				if lod > 2 then
					if get_equipment('ECM') == 'ECM_BASIC' then
						call_model('ecm_1', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					if get_equipment('ECM') == 'ECM_ADVANCED' then
						call_model('ecm_2', v(-18,-.1,-2), v(-1,0,0), v(0,-1,0), 2)
					end

					zbias(1,v(6,.407,-20), v(0,1,0))
					call_model('decal', v(6,.407,-20), v(0,1,-.07), v(-1,.349,0), 3)
					zbias(0)
				end
			end
		end

		local R_T  = v(34.9,0,-21.6)
		local RF_T = v(39.7,0,-13)
		local LF_T = v(-39.7,0,-13)
		local TF_T = v(34.9,4.9,-13)
		local BF_T = v(34.9,-4.9,-13)
		local RB_T = v(39.7,0,13)
		local LB_T = v(-39.7,0,13)
		local TB_T = v(34.9,4.9,13)
		local BB_T = v(34.9,-4.9,13)

		xref_thruster(R_T, v(0,0,-1), 15, true)
		thruster(RF_T, v(1,0,0), 9)
		thruster(LF_T, v(-1,0,0), 9)
		xref_thruster(TF_T, v(0,1,0), 9)
		xref_thruster(BF_T, v(0,-1,0), 9)
		thruster(RB_T, v(1,0,0), 9)
		thruster(LB_T, v(-1,0,0), 9)
		xref_thruster(TB_T, v(0,1,0), 9)
		xref_thruster(BB_T, v(0,-1,0), 9)

	end
})

define_model('eagle_cyl', {
	info = {
		lod_pixels = { 10, 30 ,100, 0 },
		bounding_radius = 14,
		materials = {'chrome'},
	},
	static = function(lod)
		set_material('chrome', .43,.5,.63,1,1,1.2,1.4,30)
		use_material('chrome')
		texture('tex8.png')
		sphere_slice(3*lod, 2*lod, 0, math.pi, Matrix.translate(v(9,0,0)) * Matrix.scale(v(0.75,0.75,0.75)))
		sphere_slice(3*lod, 2*lod, 0, math.pi, Matrix.translate(v(-9,0,0)) * Matrix.scale(v(0.75,0.75,0.75)))
	end,
	dynamic = function(lod)
		use_material('chrome')
		texture('models/ships/4_eagles/tex8.png', v(0,0,0), v(0,1,0), v(1,0,0))
		local uc_trans = math.clamp(get_animation_position('WHEEL_STATE'), 0.3, 1)
		xref_ring(3*lod, v(9,0,0), v(9,(7*-uc_trans)+1.7,0), v(0,0,1), .25)
		xref_ring(3*lod, v(9,0,0), v(9,(9*-uc_trans)+1.7,0), v(0,0,1), .2)
		xref_ring(3*lod, v(9,0,0), v(9,(11*-uc_trans)+1.7,0), v(0,0,1), .15)
	end
})

define_model('eagle_wheels', {
	info = {
		lod_pixels = {10,30,100,0},
		bounding_radius = 12,
		materials = {'metal', 'wheels'},
	},
	static = function(lod)
		set_material('metal', .2,.23,.25,1,.35,.38,.4,10)
		set_material('wheels', .5,.5,.5,1,.3,.3,.3,5)

		use_material('metal')
		texture('tex8.png')
		xref_cylinder(4, v(9,0,4.5), v(9,0,-4.5), v(0,1,0), .5)

		use_material('wheels')
		texture('tex9.png', v(.5,.5,0), v(0,0,.95), v(0,.26,0))
		xref_cylinder(4*lod, v(9.5,0,4), v(10.5,0,4), v(0,1,0), 1.6)
		xref_cylinder(4*lod, v(9.5,0,0), v(10.5,0,0), v(0,1,0), 1.6)
		xref_cylinder(4*lod, v(9.5,0,-4), v(10.5,0,-4), v(0,1,0), 1.6)

		xref_cylinder(4*lod, v(8.5,0,4), v(7.5,0,4), v(0,1,0), 1.6)
		xref_cylinder(4*lod, v(8.5,0,0), v(7.5,0,0), v(0,1,0), 1.6)
		xref_cylinder(4*lod, v(8.5,0,-4), v(7.5,0,-4), v(0,1,0), 1.6)
	end
})

define_model('eagle_all', {
	info = {
		lod_pixels = {.1,10,100,0},
		bounding_radius = 52,
		materials = {'d_grey', 'steel', 'chrome', 'e_glow', 'cutout', 'win', 'd_red', 'text', 'null', 'grey', 'desk', 'black'},
	},
	static = function(lod)
		local v0 = v(0,0,-40)
		local v2 = v(0,6,25)
		local v4 = v(0,-6,25)
		local v6 = v(31,0,-9)
		local v7 = v(-31,0,-9)
		local v8 = v(3,0,12)
		local v9 = v(-3,0,12)
		local v10 = v(9,0,-37)
		local v11 = v(-9,0,-37)
		local v14 = v(31,1,9)
		local v15 = v(-31,1,9)
		local v24 = v(31,-1,9)
		local v25 = v(-31,-1,9)
		local v28 = v(1,5.03,17)
		local v29 = v(-1,5.03,17)
		local v30 = v(6,3.95,15)
		local v31 = v(-6,3.95,15)
		local v32 = v(1,4,4)
		local v33 = v(-1,4,4)
		local v34 = v(6,2.3,2)
		local v35 = v(-6,2.3,2)
		local v36 = v(15,0,17.263)
		local v37 = v(-15,0,17.263)
		local v38 = v(3,2,23.45)
		local v39 = v(-3,2,23.45)
		local v40 = v(3,-3,23.45)
		local v41 = v(-3,-3,23.45)
		local v42 = v(15,0,15.763)
		local v43 = v(-15,0,15.763)
		local v44 = v(3,2,21.95)
		local v45 = v(-3,2,21.95)
		local v46 = v(3,-3,21.95)
		local v47 = v(-3,-3,21.95)
		local v52 = v36+v(4,0,-1)
		local v54 = v38+v(-.5,1,1.25)
		local v56 = v40+v(-.5,-1,1.25)
		local v58 = v36+v(8,0,-4.25)
		local v60 = v38+v(-2,2,1)
		local v62 = v40+v(-2,-2,1)
		local v100 = v(0,1.72,-22)
		local v101 = v(0,1.31,-26)
		local v102 = v(1.5,1.55,-22)
		local v103 = v(-1.5,1.55,-22)
		local v104 = v(1.5,1.2,-26)
		local v105 = v(-1.5,1.2,-26)
		local v106 = v(1.5,-1.4,-22)
		local v107 = v(-1.5,-1.4,-22)
		local v108 = v(1.5,-1.1,-26)
		local v109 = v(-1.5,-1.1,-26)
		local v110 = v(1.5,.7,-25.75)
		local v111 = v(-1.5,.7,-25.75)

		-- collision mesh
		if lod == 1 then
			xref_flat(4, v(0,1,0), {v2}, {v14}, {v6}, {v8, v10, v0})
			xref_flat(4, v(0,-1,0), {v4}, {v25}, {v7}, {v9, v11, v0})
			xref_quad(v2,v4,v24,v14)
		end

		if lod > 1 then
			set_material('text', .7,.7,.7,1,.3,.3,.3,5)
			set_material('grey', .05,.15,.12,1,.1,.1,.1,5)
			set_material('desk', 1,.98,.9,1,.5,.5,.5,10)
			set_material('black', 0,0,0,1,0,0,0,0)
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('steel', .2,.23,.25,1,.35,.38,.4,10)
			set_material('d_grey', .2,.2,.2,1,.3,.3,.3,5)
			set_material('d_red', .3,0,0,1,.3,.3,.3,50)

			-- back right
			use_material('steel')
			texture('tex2.png', v(0,.55,0), v(.03,0,0), v(0,.085,0))
			quad(v2,v4,v40,v38)
			quad(v4,v24,v36,v40)
			tri(v24,v14,v36)
			quad(v14,v2,v38,v36)

			-- back left
			texture('tex2.png', v(0,.55,0), v(-.03,0,0), v(0,.085,0))
			quad(v4,v2,v39,v41)
			quad(v4,v41,v37,v25)
			tri(v25,v37,v15)
			quad(v15,v37,v39,v2)

			-- engine inside
			use_material('e_glow')
			texture('tex10.png', v(.25,.5,0), v(-.04,0,0), v(0,.08,0))
			tri(v42,v44,v46)
			texture('tex10.png', v(.25,.5,0), v(.04,0,0), v(0,.08,0))
			tri(v43,v47,v45)

			-- main thruster "nazzle"
			use_material('chrome')
			texture('tex4.png', v(0,.55,0), v(.045,0,0), v(0,-.11,0))
			xref_quad(v36,v40,v56,v52)
			xref_quad(v36,v52,v54,v38)
			xref_quad(v38,v54,v56,v40)
			xref_quad(v56,v62,v58,v52)
			xref_quad(v52,v58,v60,v54)
			xref_quad(v60,v62,v56,v54)
			xref_quad(v38,v40,v46,v44)
			xref_quad(v40,v36,v42,v46)
			xref_quad(v36,v38,v44,v42)

			-- top cooler
			use_material('d_grey')
			texture('tex7.png', v(0,.07,0), v(.1,.04,0), v(0,0,-1))
			quad(v28,v30,v34,v32)
			texture('tex7.png', v(0,.07,0), v(.1,-.04,0), v(0,0,-1))
			quad(v29,v33,v35,v31)
		end

		if lod > 2 then
			-- pilot
			call_model('pilot2', v(0,1.3,-23), v(1,0,0), v(0,1,0), .6)

			-- cockpit
			use_material('black')
			quad(v107,v106,v108,v109)
			quad(v104,v105,v109,v108)
			use_material('grey')
			xref_quad(v102,v104,v108,v106)
			quad(v102,v106,v107,v103)

			use_material('desk')
			texture('tex6.png', v(.5,0,0), v(.32,0,0), v(0,0,12.4))
			quad(v104,v105,v111,v110)
			texture('tex5.png', v(.5,.35,0), v(.335,0,0), v(0,1.95,0))
			quad(v110,v111,v111+v(0,-.5,1), v110+v(0,-.5,1))

			use_material('null')
			xref_quad(v100,v102,v104,v101)
		end
	end,

	dynamic = function(lod)
		local trans1 = 6.667*math.clamp(get_animation_position('WHEEL_STATE'),0,.3)
		local trans2 = 12*math.clamp(get_animation_position('WHEEL_STATE'),.3,1)
		local rot = 2.4*math.clamp(get_animation_position('WHEEL_STATE'),.3,1)

		if lod > 1 then

			set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5,  {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .9, .8, 1.5 }))

			if get_animation_position('WHEEL_STATE') ~= 0 then
				local v64 = v(9-trans1,-1.55-(trans1/5),1)
				local v66 = v(9+trans1,-1.55+(trans1/8),1)
				local v68 = v(9+trans1,-3.53-(trans1/6),13)
				local v70 = v(9-trans1,-3.53,13)
				local v72 = v(7,-1.94,1)
				local v74 = v(11,-1.29,1)
				local v76 = v(11,-3.86,13)
				local v78 = v(7,-3.53,13)

				use_material('d_grey')
				texture('models/ships/4_eagles/tex8.png')
				xref_quad(v72+v(0,3.24,0),v74+v(0,2.59,0),v76+v(0,4.86,0),v78+v(0,4.53,0))
				xref_quad(v78,v72,v72+v(0,3.24,0),v78+v(0,4.53,0))
				xref_quad(v74,v76,v76+v(0,4.86,0),v74+v(0,2.59,0))
				xref_quad(v72,v74,v74+v(0,2.59,0),v72+v(0,3.24,0))
				xref_quad(v76,v78,v78+v(0,4.53,0),v76+v(0,4.86,0))

				call_model('eagle_wheels', v(0,3.6-trans2,7), v(1,0,0), v(0,1,0), 1)
				call_model('eagle_cyl', v(0,1.3,5), v(1,0,0), v(0,1.7-math.cos(rot),.3-math.sin(rot)), 1)
				call_model('eagle_cyl', v(0,1.3,11), v(1,0,0), v(0,1.5-math.cos(rot),.3+math.sin(rot)), 1)

				use_material('null')
				texture(nil)
				xref_quad(v64, v66, v68, v70)
			end
		end

		texture(nil)
		if lod == 1 then
			if get_animation_position('WHEEL_STATE') ~= 0 then
				xref_cylinder(4, v(9,3.6-trans2,1), v(9,3.6-trans2,13), v(0,1,0), 2)
			end
		end

		if lod > 1 then
			local vTXT1 = v(19,2.759,12)
			local vTXT2 = v(-19,-2.759,12)
			local reg = get_label()
			use_material('text')
			zbias(1,v(19,2.759,12),v(0,1,.0105))
			text(reg, vTXT1, v(0,1,.0105), v(1,-.1604,-.5), 5, {center = true})
			zbias(1,v(-19,-2.759,12),v(0,-1,.0105))
			text(reg, vTXT2, v(0,-1,.0105), v(-1,.1604,-.5), 5, {center = true})
			zbias(0)

			if get_equipment('LASER', 1) then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					call_model('eagle_gun',v(20,-.5,0),v(1,0,0),v(0,1,0),1)
					call_model('eagle_gun',v(-20,-.5,0),v(1,0,0),v(0,1,0),1)
				else
					call_model('eagle_gun',v(0,-.7,-30),v(1,0,0),v(0,1,0),1)
				end
			end

			if get_equipment('LASER', 2) then
				if get_equipment('LASER', 2) == 'PULSECANNON_DUAL_1MW' then
					call_model('eagle_gun',v(20,-.5,0),v(1,0,0),v(0,1,0),1)
					call_model('eagle_gun',v(-20,-.5,0),v(1,0,0),v(0,1,0),1)
				else
					call_model('eagle_gun',v(0,-.7,-30),v(1,0,0),v(0,1,0),1)
				end
			end

			-- missiles
			local M_0 = v(24,-3.5,3)
			local M_1 = v(-24,-3.5,3)

			if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
				call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
				call_model('d_unguided',M_0,v(1,0,0),v(0,1,0),3.5)
			else
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
					call_model('d_guided',M_0,v(1,0,0),v(0,1,0),3.5)
				else
					if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
						call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
						call_model('d_smart',M_0,v(1,0,0),v(0,1,0),3.5)
					else
						if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
							call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
							call_model('d_naval',M_0,v(1,0,0),v(0,1,0),3.5)
						end
					end
				end
			end

			if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
				call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
				call_model('d_unguided',M_1,v(1,0,0),v(0,1,0),3.5)
			else
				if get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
					call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
					call_model('d_guided',M_1,v(1,0,0),v(0,1,0),3.831)
				else
					if get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
						call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
						call_model('d_smart',M_1,v(1,0,0),v(0,1,0),3.5)
					else
						if get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
							call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
							call_model('d_naval',M_1,v(1,0,0),v(0,1,0),3.5)
						end
					end
				end
			end
		end

		-- main thrusters
		local M_T = v(6.5,-.35,24)
		xref_thruster(M_T, v(0,0,1), 40, true)
	end
})

define_model('eagle_lrf', {
	info = {
		--			scale = 0.261, -- FE2 to Pioneer vector spacing scale for the eagle, each ship is different!
		scale = 0.45,
		lod_pixels = {.1,10,50,0},
		bounding_radius = 30,
		materials={'steel', 'cutout', 'win'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Eagle Long Range Fighter',
				forward_thrust = -34e5,
				reverse_thrust = 17e5,
				up_thrust = 8e5,
				down_thrust = -8e5,
				left_thrust = -8e5,
				right_thrust = 8e5,
				angular_thrust = 64e5,
				gun_mounts =
				{
					{ v(0,-.7,-40), v(0,0,-1) },
					{ v(0,-.7,25), v(0,0,1) },
				},
				max_cargo = 20,
				max_missile = 2,
				max_fuelscoop = 0,
				max_cargoscoop = 0,
				capacity = 20,
				hull_mass = 10,
				fuel_tank_mass = 5,
				thruster_fuel_use = 0.0001,
				price = 38000,
				hyperdrive_class = 1,
			}
		}
	},

	static = function(lod)
		local v48 = v(8.5,1,-14)
		local v49 = v(8.5,-1,-14)
		local v50 = v(-8.5,-1,-14)
		local v51 = v(-8.5,1,-14)
		local v80 = v(34,0,0)
		local v81 = v(31,0,-25)
		local v82 = v(31,6,3)
		local v83 = v(31,0,18)
		local v84 = v(31,-6,3)
		local v85 = v(-31,0,-25)
		local v86 = v(-31,0,18)

		set_material('cutout', .6,.6,.6,.9,.3,.3,.3,50)
		set_material('steel', .2,.23,.25,1,.35,.38,.4,10)
		-- collision mesh lrf tanks
		if lod == 1 then
			xref_tri(v80,v81,v82)
			xref_tri(v80,v82,v83)
			xref_tri(v80,v83,v84)
			xref_tri(v80,v84,v81)
			xref_quad(v84,v83,v82,v81)
		end
		-- lrf side tanks outside
		if lod > 1 then
			use_material('steel')
			texture('tex2.png', v(.5,.55,0), v(0,0,.3), v(0,.085,0))
			xref_tri(v80,v81,v82)
			xref_tri(v80,v82,v83)
			xref_tri(v80,v83,v84)
			xref_tri(v80,v84,v81)
		end
		if lod > 2 then
			-- nazzles
			call_model('nazzle_n', v81+v(0,0,.2), v(1,0,0), v(0,0,-1), .7)  -- retro
			call_model('nazzle_n', v85+v(0,0,.2), v(1,0,0), v(0,0,-1), .7)
			call_model('nazzle_n', v(9.5,0,-14), v(0,1,0), v(1,0,0), .6)    -- rft
			call_model('nazzle_n', v(-9.5,0,-14), v(0,1,0), v(-1,0,0), .6)  -- lft
			call_model('nazzle_n', v83+v(1,0,-2), v(0,1,0), v(1,0,0), .6)   -- rbt
			call_model('nazzle_n', v86+v(-1,0,-2), v(0,1,0), v(-1,0,0), .6) -- lbt
			call_model('nazzle_n', v83+v(0,1.3,-2), v(1,0,0), v(0,1,0), .6) -- tbt
			call_model('nazzle_n', v86+v(0,1.3,-2), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v83+v(0,-1.3,-2), v(1,0,0), v(0,-1,0), .6) -- bbt
			call_model('nazzle_n', v86+v(0,-1.3,-2), v(1,0,0), v(0,-1,0), .6)
			call_model('nazzle_n', v48, v(1,0,0), v(0,1,0), .6)  -- rtft
			call_model('nazzle_n', v49, v(1,0,0), v(0,-1,0), .6) -- rbft
			call_model('nazzle_n', v50, v(1,0,0), v(0,-1,0), .6) -- lbft
			call_model('nazzle_n', v51, v(1,0,0), v(0,1,0), .6)  -- lbtt
		end

		if lod > 1 then
			-- poslights
			call_model('posl_green', v(33.5,.7,-.5), v(0,1,0), v(1,.6,0), 6)
			call_model('coll_warn', v(33.5,-.7,-.5), v(0,1,0), v(1,-.6,0), 6)
			call_model('posl_red', v(-33.5,.7,-.5), v(0,1,0), v(-1,.6,0), 6)
			call_model('coll_warn', v(-33.5,-.7,-.5), v(0,1,0), v(-1,-.6,0), 6)
		end

		-- lrf side tanks inside
		if lod > 1 then
			use_material('steel')
			texture('tex2.png', v(.5,.55,0), v(0,0,.3), v(0,.085,0))
			xref_quad(v84,v83,v82,v81)
		end
		texture(nil)
		-- eagle mainpart
		call_model('eagle_all', v(0,0,0), v(1,0,0), v(0,1,0), 1)

		-- eagle cv body
		if lod > 1 then
			call_model('eagle_lrf_body', v(0,0,0), v(1,0,0), v(0,1,0), 1)
			-- details
			zbias(1,v(-15,3.4,14), v(-.13,1,0))
			call_model('squadsign_1', v(-15,3.4,14), v(-.13,1,0), v(0,-.0555,-1), 4)
			zbias(1,v(15,-3.4,14), v(.13,-1,0))
			call_model('squadsign_1', v(15,-3.4,14), v(.13,-1,0), v(0,.0555,-1), 4)
			zbias(0)
		end

		-- rostrum
		if lod > 3 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(1,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(-1.999,2.499,3.999)))
			zbias(0)
		end
		texture(nil)
		use_material('win')
		zbias(2,v(0,1,-24),v(0,1,0))
		sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2,2.5,4)))
		zbias(0)
		if lod > 2 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(3,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2.001,2.501,4.001)))
			zbias(0)
		end

		if lod > 1 then
			-- poslights
			call_model('posl_white', v(0,5.8,24), v(1,0,0), v(0,1,0), 6)
			call_model('coll_warn', v(0,-5.8,24), v(1,0,0), v(0,-1,0), 6)
		end
	end,

	dynamic = function(lod)
		if lod > 2 then
			set_material('win', .5,.5,5,.2,1,1,1,100)
		else
			set_material('win', .2,.2,.5,1,1,1,1,100)
		end
	end
})

define_model('eagle_mk2', {
	info = {
		scale = 0.45,
		lod_pixels = {.1,10,50,0},
		bounding_radius = 30,
		materials={'chrome', 'cutout', 'win'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Eagle MK-II',
				forward_thrust = -34e5,
				reverse_thrust = 17e5,
				up_thrust = 8e5,
				down_thrust = -8e5,
				left_thrust = -8e5,
				right_thrust = 8e5,
				angular_thrust = 64e5,
				gun_mounts =
				{
					{ v(0,-.7,-40), v(0,0,-1) },
					{ v(0,-.7,25), v(0,0,1) },
				},
				max_cargo = 22,
				max_missile = 2,
				max_fuelscoop = 0,
				max_cargoscoop = 0,
				capacity = 22,
				hull_mass = 9,
				fuel_tank_mass = 6,
				thruster_fuel_use = 0.0001,
				price = 41000,
				hyperdrive_class = 1,
			}
		}
	},

	static = function(lod)
		local v81 = v(31,0,-25)
		local v83 = v(31,0,9)
		local v85 = v(-31,0,-25)
		local v86 = v(-31,0,9)
		local v50 = v(0,-3,-14)
		local v51 = v(0,3,-14)

		set_material('cutout', .6,.6,.6,.9,.3,.3,.3,50)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		-- collision mesh mk2 wingtips
		if lod == 1 then
			tapered_cylinder(3,v83+v(.5,0,0),v81, v(0,1,0), .6,.1)
			tapered_cylinder(3,v86+v(-.5,0,0),v85, v(0,-1,0), .6,.1)
		end
		-- mk2 wingtips
		if lod > 1 then
			use_material('chrome')
			texture('tex8.png')
			tapered_cylinder(3,v83+v(.5,0,0),v81, v(0,1,0), .6,.1)
			tapered_cylinder(3,v86+v(-.5,0,0),v85, v(0,-1,0), .6,.1)
		end
		texture(nil)
		if lod > 2 then
			-- nazzles
			call_model('nazzle_n', v(8.8,0,-15.2), v(1,0,0), v(0,0,-1), .6)  -- retro
			call_model('nazzle_n', v(-8.8,0,-15.2), v(1,0,0), v(0,0,-1), .6)
			call_model('nazzle_n', v(9.5,0,-14), v(0,1,0), v(1,0,0), .6)    -- rft
			call_model('nazzle_n', v(-9.5,0,-14), v(0,1,0), v(-1,0,0), .6)  -- lft
			call_model('nazzle_n', v83+v(2.2,0,-1), v(0,1,0), v(1,0,0), .6)   -- rbt
			call_model('nazzle_n', v86+v(-2.2,0,-1), v(0,1,0), v(-1,0,0), .6) -- lbt
			call_model('nazzle_n', v83+v(1,1.3,-1), v(1,0,0), v(0,1,0), .6) -- tbt
			call_model('nazzle_n', v86+v(-1,1.3,-1), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v83+v(1,-1.3,-1), v(1,0,0), v(0,-1,0), .6) -- bbt
			call_model('nazzle_n', v86+v(-1,-1.3,-1), v(1,0,0), v(0,-1,0), .6)
			call_model('nazzle_n', v50, v(1,0,0), v(0,-1,0), .6) -- bft
			call_model('nazzle_n', v51, v(1,0,0), v(0,1,0), .6)  -- tft

		end

		-- eagle mainpart
		call_model('eagle_all', v(0,0,0), v(1,0,0), v(0,1,0), 1)

		--eagle cv body
		if lod > 1 then
			call_model('eagle_mk2_body', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		end

		if lod > 1 then
			-- details
			zbias(1,v(-15,3.4,14), v(-.13,1,0))
			call_model('squadsign_1', v(-15,3.4,14), v(-.13,1,0), v(0,-.0555,-1), 4)
			zbias(1,v(15,-3.4,14), v(.13,-1,0))
			call_model('squadsign_1', v(15,-3.4,14), v(.13,-1,0), v(0,.0555,-1), 4)
			zbias(0)
		end

		-- rostrum
		if lod > 3 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(1,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(-1.999,2.499,3.999)))
			zbias(0)
		end
		texture(nil)
		use_material('win')
		zbias(2,v(0,1,-24),v(0,1,0))
		sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2,2.5,4)))
		zbias(0)
		if lod > 2 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(3,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2.001,2.501,4.001)))
			zbias(0)
		end

		if lod > 1 then
			-- poslights
			call_model('posl_white', v(0,5.8,24), v(1,0,0), v(0,1,0), 6)
			call_model('coll_warn', v(0,-5.8,24), v(1,0,0), v(0,-1,0), 6)
			call_model('posl_green', v(31.6,.3,-.5), v(0,1,0), v(.5,1,-.05), 6)
			call_model('coll_warn', v(31.6,-.3,-.5), v(0,1,0), v(.5,-1,-.05), 6)
			call_model('posl_red', v(-31.6,.3,-.5), v(0,1,0), v(-.5,1,-.05), 6)
			call_model('coll_warn', v(-31.6,-.3,-.5), v(0,1,0), v(-.5,-1,-.05), 6)
		end
	end,

	dynamic = function(lod)

		if lod > 2 then
			set_material('win', .5,.5,5,.2,1,1,1,100)
		else
			set_material('win', .2,.2,.5,1,1,1,1,100)
		end
	end
})

define_model('eagle_mk3', {
	info = {
		scale = 0.45,
		lod_pixels = {.1,10,50,0},
		bounding_radius = 30,
		materials={'chrome', 'e_glow', 'win', 'cutout'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Eagle MK-III',
				forward_thrust = -36e5,
				reverse_thrust = 25e5,
				up_thrust = 8e5,
				down_thrust = -8e5,
				left_thrust = -8e5,
				right_thrust = 8e5,
				angular_thrust = 64e5,
				gun_mounts =
				{
					{ v(0,-.7,-40), v(0,0,-1) },
					{ v(0,-.7,25), v(0,0,1) },
				},
				max_cargo = 22,
				max_missile = 2,
				max_fuelscoop = 0,
				max_cargoscoop = 0,
				capacity = 22,
				hull_mass = 10,
				fuel_tank_mass = 5,
				thruster_fuel_use = 0.00015,
				price = 43000,
				hyperdrive_class = 1,
			}
		}
	},

	static = function(lod)
		local v81 = v(30.5,0,-25)
		local v83 = v(30.5,0,18)
		local v85 = v(-30.5,0,-25)
		local v86 = v(-30.5,0,18)

		set_material('cutout', .6,.6,.6,.9,.3,.3,.3,50)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		-- collision mesh mk3 side engines
		if lod == 1 then
			xref_cylinder(3,v83+v(2,0,0), v81+v(2,0,0), v(0,1,0), 2)
		end
		-- mk3 side engines
		if lod > 1 then
			use_material('chrome')
			texture('tex8.png', v(0,0,0), v(.5,0,0), v(0,.2,0))
			xref_ring(4*lod,v83+v(2,0,-2), v81+v(2,0,2), v(0,1,0), 2)
			texture('tex4.png', v(.5,.35,0), v(0,0,-2), v(.24,0,0))
			xref_tube(4*lod, v83+v(2,0,-2), v83+v(2,0,0), v(0,1,0), 1.6,2)
			texture('tex4.png', v(.5,0,0), v(0,0,2), v(.24,0,0))
			xref_tube(4*lod, v81+v(2,0,2), v81+v(2,0,0), v(0,1,0), 1.6,2)
		end
		if lod > 1 then
			use_material('e_glow')
			texture('tex10.png', v(.77,.5,0), v(.19,0,0), v(0,.15,0))
			circle(3*lod, v83+v(2,0,-2), v(0,0,1), v(0,1,0), 1.9)
			circle(3*lod, v81+v(2,0,2), v(0,0,-1), v(0,1,0), 1.9)
			texture('tex10.png', v(.23,.5,0), v(.19,0,0), v(0,.15,0))
			circle(3*lod, v86+v(-2,0,-2), v(0,0,1), v(0,1,0), 1.9)
			circle(3*lod, v85+v(-2,0,2), v(0,0,-1), v(0,1,0), 1.9)
		end

		texture(nil)
		if lod > 2 then
			-- nazzles
			call_model('nazzle_n', v81+v(4.1,0,3), v(0,1,0), v(1,0,0), .6)    -- rft
			call_model('nazzle_n', v85+v(-4.1,0,3), v(0,1,0), v(-1,0,0), .6)  -- lft
			call_model('nazzle_n', v81+v(2,2.1,3), v(1,0,0), v(0,1,0), .6)  -- tft
			call_model('nazzle_n', v85+v(-2,2.1,3), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v81+v(2,-2.1,3), v(1,0,0), v(0,-1,0), .6) -- bft
			call_model('nazzle_n', v85+v(-2,-2.1,3), v(1,0,0), v(0,-1,0), .6)
			call_model('nazzle_n', v83+v(4.1,0,-3), v(0,1,0), v(1,0,0), .6)   -- rbt
			call_model('nazzle_n', v86+v(-4.1,0,-3), v(0,1,0), v(-1,0,0), .6) -- lbt
			call_model('nazzle_n', v83+v(2,2.1,-3), v(1,0,0), v(0,1,0), .6) -- tbt
			call_model('nazzle_n', v86+v(-2,2.1,-3), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v83+v(2,-2.1,-3), v(1,0,0), v(0,-1,0), .6) -- bbt
			call_model('nazzle_n', v86+v(-2,-2.1,-3), v(1,0,0), v(0,-1,0), .6)

		end

		-- eagle mainpart
		call_model('eagle_all', v(0,0,0), v(1,0,0), v(0,1,0), 1)

		-- eagle cv body
		if lod > 1 then
			call_model('eagle_mk3_body', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		end

		if lod > 1 then
			-- details
			zbias(1,v(-15,3.4,14), v(-.13,1,0))
			call_model('squadsign_1', v(-15,3.4,14), v(-.13,1,0), v(0,-.0555,-1), 4)
			zbias(1,v(15,-3.4,14), v(.13,-1,0))
			call_model('squadsign_1', v(15,-3.4,14), v(.13,-1,0), v(0,.0555,-1), 4)
			zbias(0)
		end

		-- rostrum
		if lod > 3 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(1,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(-1.999,2.499,3.999)))
			zbias(0)
		end
		texture(nil)
		use_material('win')
		zbias(2,v(0,1,-24),v(0,1,0))
		sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2,2.5,4)))
		zbias(0)
		if lod > 2 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(3,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2.001,2.501,4.001)))
			zbias(0)
		end

		if lod > 1 then
			-- poslights
			call_model('posl_green', v(34.8,.7,-.5), v(0,1,0), v(1,.5,0), 6)
			call_model('coll_warn', v(34.8,-.7,-.5), v(0,1,0), v(1,-.5,0), 6)
			call_model('posl_red', v(-34.8,.7,-.5), v(0,1,0), v(-1,.5,0), 6)
			call_model('coll_warn', v(-34.8,-.7,-.5), v(0,1,0), v(-1,-.5,0), 6)
			call_model('posl_white', v(0,5.8,24), v(1,0,0), v(0,1,0), 6)
			call_model('coll_warn', v(0,-5.8,24), v(1,0,0), v(0,-1,0), 6)
		end
	end,

	dynamic = function(lod)
		set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5,  {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .9, .8, 1.5 }))
		if lod > 2 then
			set_material('win', .5,.5,5,.2,1,1,1,100)
		else
			set_material('win', .2,.2,.5,1,1,1,1,100)
		end
	end
})

define_model('eagle_mk4', {
	info = {
		scale = 0.45,
		lod_pixels = {.1,10,50,0},
		bounding_radius = 30,
		materials={'chrome', 'cv2', 'e_glow', 'win', 'cutout'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Eagle MK-IV "Bomber"',
				forward_thrust = -50e5,
				reverse_thrust = 25e5,
				up_thrust = 14e5,
				down_thrust = -12e5,
				left_thrust = -12e5,
				right_thrust = 12e5,
				angular_thrust = 90e5,
				gun_mounts =
				{
					{ v(0,-.7,-40), v(0,0,-1) },
					{ v(0,-.7,25), v(0,0,1) },
				},
				max_cargo = 36,
				max_laser = 2,
				max_missile = 6,
				max_fuelscoop = 1,
				max_cargoscoop = 1,
				capacity = 36,
				hull_mass = 15,
				fuel_tank_mass = 5,
				thruster_fuel_use = 0.00015,
				price = 56000,
				hyperdrive_class = 2,
			}
		}
	},

	static = function(lod)
		local v40 = v(8,4,20)
		local v42 = v(8,2.5,7)
		local v44 = v(10,10,20)
		local v46 = v(10,10,12)
		local v48 = v(8.5,4,20)
		local v50 = v(8.5,2.5,7)
		local v52 = v(10.5,10,20)
		local v54 = v(10.5,10,12)
		local v81 = v(34.9,0,-16)
		local v83 = v(34.9,0,16)
		local v85 = v(-34.9,0,-16)
		local v86 = v(-34.9,0,16)

		set_material('cutout', .6,.6,.6,.9,.3,.3,.3,50)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)

		-- collision mesh mk4
		if lod == 1 then
			texture(nil)
			xref_quad(v42,v40,v44,v46)
			xref_quad(v50,v54,v52,v48)
			xref_quad(v40,v48,v52,v44)
			xref_quad(v44,v52,v54,v46)
			xref_quad(v42,v46,v54,v50)
			cylinder(3,v81+v(0,0,-4), v83+v(0,0,4), v(0,1,0), 2)
			cylinder(3,v86+v(0,0,4), v85+v(0,0,-4), v(0,1,0), 2)
			cubic_bezier_quad(2,2,  v(5,-1.5,-8), v(5,-2,-4), v(3,-2,5), v(1,-4,10),
			v(3,-6,-8), v(4,-8,-4), v(2,-8,5), v(0.5,-4,10),
			v(-3,-6,-8), v(-4,-8,-4), v(-2,-8,5), v(-0.5,-4,10),
			v(-5,-1.5,-8), v(-5,-2,-4), v(-3,-2,5), v(-1,-4,10))
			geomflag(0x100)
			flat(3, v(0,0,-1), {v(4.5,-2,-8)}, {v(4,-6.2,-8), v(-4,-6.2,-8), v(-4.5,-2,-8)})
			geomflag(0)
		end

		-- mk4 fin
		if lod > 1 then
			use_material('chrome')
			texture('tex8.png')
			xref_cylinder(3*lod, v(10.25,10,20), v(10.25,10,8), v(0,1,0), .25)

			-- mk4 scoop
			use_material('e_glow')
			texture('tex10.png', v(.5,.3,0), v(.09,0,0), v(0,.1,0))
			flat(3*lod, v(0,0,-1), {v(4.5,-2,-6)}, {v(4,-6.2,-6), v(-4,-6.2,-6), v(-4.5,-2,-6)})
		end

		texture(nil)
		if lod > 2 then
			-- nazzles
			call_model('nazzle_l', v81+v(0,0,-4.8), v(1,0,0), v(0,0,-1), 1)  -- retro
			call_model('nazzle_l', v85+v(0,0,-4.8), v(1,0,0), v(0,0,-1), 1)
			call_model('nazzle_n', v81+v(4.2,0,3), v(0,1,0), v(1,0,0), .6)    -- rft
			call_model('nazzle_n', v85+v(-4.2,0,3), v(0,1,0), v(-1,0,0), .6)  -- lft
			call_model('nazzle_n', v81+v(0,4.3,3), v(1,0,0), v(0,1,0), .6)  -- tft
			call_model('nazzle_n', v85+v(0,4.3,3), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v81+v(0,-4.3,3), v(1,0,0), v(0,-1,0), .6) -- bft
			call_model('nazzle_n', v85+v(0,-4.3,3), v(1,0,0), v(0,-1,0), .6)
			call_model('nazzle_n', v83+v(4.2,0,-3), v(0,1,0), v(1,0,0), .6)   -- rbt
			call_model('nazzle_n', v86+v(-4.2,0,-3), v(0,1,0), v(-1,0,0), .6) -- lbt
			call_model('nazzle_n', v83+v(0,4.3,-3), v(1,0,0), v(0,1,0), .6) -- tbt
			call_model('nazzle_n', v86+v(0,4.3,-3), v(1,0,0), v(0,1,0), .6)
			call_model('nazzle_n', v83+v(0,-4.3,-3), v(1,0,0), v(0,-1,0), .6) -- bbt
			call_model('nazzle_n', v86+v(0,-4.3,-3), v(1,0,0), v(0,-1,0), .6)

		end

		-- eagle mainpart
		call_model('eagle_all', v(0,0,0), v(1,0,0), v(0,1,0), 1)

		if lod > 1 then
			-- eagle cv body
			call_model('eagle_mk4_body', v(0,0,0), v(1,0,0), v(0,1,0), 1)
			-- details
			zbias(1,v(-15,3.4,14), v(-.13,1,0))
			call_model('squadsign_1', v(-15,3.4,14), v(-.13,1,0), v(0,-.0555,-1), 4)
			zbias(1,v(15,-3.4,14), v(.13,-1,0))
			call_model('squadsign_1', v(15,-3.4,14), v(.13,-1,0), v(0,.0555,-1), 4)
			zbias(0)
		end

		-- rostrum
		if lod > 3 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(1,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(-1.999,2.499,3.999)))
			zbias(0)
		end
		texture(nil)
		use_material('win')
		zbias(2,v(0,1,-24),v(0,1,0))
		sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2,2.5,4)))
		zbias(0)
		if lod > 2 then
			use_material('cutout')
			texture('tex11.png', v(.5,.4,0), v(.18,0,0), v(0,0,.9))
			zbias(3,v(0,1,-24),v(0,1,0))
			sphere_slice(4*lod,2*lod, 0, 0.5*math.pi, Matrix.translate(v(0,1,-24)) * Matrix.scale(v(2.001,2.501,4.001)))
			zbias(0)
		end
		if lod > 1 then
			-- poslights
			call_model('posl_green', v(38.7,.7,-.5), v(0,1,0), v(1,.3,0), 6)
			call_model('coll_warn', v(38.7,-.7,-.5), v(0,1,0), v(1,-.3,0), 6)
			call_model('posl_red', v(-38.7,.7,-.5), v(0,1,0), v(-1,.3,0), 6)
			call_model('coll_warn', v(-38.7,-.7,-.5), v(0,1,0), v(-1,-.3,0), 6)
			call_model('posl_white', v(0,5.8,24), v(1,0,0), v(0,1,0), 6)
			call_model('coll_warn', v(0,-5.8,24), v(1,0,0), v(0,-1,0), 6)
		end
	end,

	dynamic = function(lod)
		set_material('cv2', get_arg_material(1))
		set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5,  {0, 0, 0, 1, 0, 0, 0, 0, .7, 1, 1.5 }, {0, 0, 0, 1, 0, 0, 0, 0, .9, .8, 1.5 }))
		if lod > 2 then
			set_material('win', .5,.5,5,.2,1,1,1,100)
		else
			set_material('win', .2,.2,.5,1,1,1,1,100)
		end

		if lod > 1 then
			-- missiles
			local M_0 = v(28,-3,3)
			local M_1 = v(-28,-3,3)

			if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
				call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
				call_model('d_unguided',M_0,v(1,0,0),v(0,1,0),3.5)
			else
				if get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
					call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
					call_model('d_guided',M_0,v(1,0,0),v(0,1,0),3.5)
				else
					if get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
						call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
						call_model('d_smart',M_0,v(1,0,0),v(0,1,0),3.5)
					else
						if get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
							call_model('m_pod',M_0+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
							call_model('d_naval',M_0,v(1,0,0),v(0,1,0),3.5)
						end
					end
				end
			end

			if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
				call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
				call_model('d_unguided',M_1,v(1,0,0),v(0,1,0),3.5)
			else
				if get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
					call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
					call_model('d_guided',M_1,v(1,0,0),v(0,1,0),3.831)
				else
					if get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
						call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
						call_model('d_smart',M_1,v(1,0,0),v(0,1,0),3.5)
					else
						if get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
							call_model('m_pod',M_1+v(0,.3,0),v(1,0,0),v(0,1,0),3.5)
							call_model('d_naval',M_1,v(1,0,0),v(0,1,0),3.5)
						end
					end
				end
			end
		end
	end
})
