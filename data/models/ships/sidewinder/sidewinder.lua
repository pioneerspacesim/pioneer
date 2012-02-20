define_model('rattle_s_gun', {
	info =	{
		lod_pixels = {5,10,20,0},
		bounding_radius = 5,
		materials = {'chrome', 'hole'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('hole', 0,0,0,1,0,0,0,0)

		if lod > 1 then
			use_material('hole')
			zbias(1,v(0,0,-17), v(0,0,-1))
			circle(3*lod, v(0,0,-17), v(0,0,-1), v(0,1,0), .17)
			zbias(0)
			if lod > 2 then
				texture('gun.png',v(.5,0,0),v(2.2,0,0),v(0,0,-.08))
			else
				texture('alu.png')
			end
			use_material('chrome')
		end
		cylinder(3*lod, v(0,0,-11.5), v(0,0,-17), v(0,1,0), .2)

	end
})

define_model('rattle_l_gun', {
	info =	{
		lod_pixels = {5,10,20,0},
		bounding_radius = 5,
		materials = {'chrome', 'hole'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('hole', 0,0,0,1,0,0,0,0)

		if lod > 1 then
			use_material('hole')
			zbias(1,v(0,0,-17), v(0,0,-1))
			circle(3*lod, v(0,0,-17), v(0,0,-1), v(0,1,0), .17)
			zbias(0)
			if lod > 2 then
				texture('gun.png',v(.5,0,0),v(2.2,0,0),v(0,0,-.08))
			else
				texture('alu.png')
			end
			use_material('chrome')
		end
		cylinder(3*lod, v(0,0,-11.5), v(0,0,-17), v(0,1,0), .2)
		xref_cylinder(3*lod, v(3,0,-11.5), v(3,0,-15), v(0,1,0), .1)

	end
})

define_model('rattle_gun', {
	info =	{
		lod_pixels = {5,10,20,0},
		bounding_radius = 5,
		materials = {'chrome', 'hole'},
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		local laser1 = get_equipment('LASER', 1)
		if laser1 then
			if laser1 == 'PULSECANNON_DUAL_1MW' then
				call_model('rattle_s_gun',v(9,0,0),v(1,0,0),v(0,1,0),1)
				call_model('rattle_s_gun',v(-9,0,0),v(1,0,0),v(0,1,0),1)
			elseif laser1 == 'PULSECANNON_1MW' then
				call_model('rattle_s_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				call_model('rattle_l_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
		end

		local laser2 = get_equipment('LASER', 2)
		if laser2 then
			if laser2 == 'PULSECANNON_DUAL_1MW' then
				call_model('rattle_s_gun',v(9,0,0),v(1,0,0),v(0,1,0),1)
				call_model('rattle_s_gun',v(-9,0,0),v(1,0,0),v(0,1,0),1)
			elseif laser2 == 'PULSECANNON_1MW' then
				call_model('rattle_s_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				call_model('rattle_l_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end
		end
	end
})

define_model('rattle', {
	info =	{
		lod_pixels = {5,10,50,0},
		bounding_radius = 20,
		materials = {'top', 'bot', 'black', 'chrome', 'text'},
	},
	static = function(lod)
		local  v0 = v(13,0,-13)
		local  v1 = v(-13,0,-13)
		local  v2 = v(26,0,13)
		local  v4 = v(0,6,13)
		local  v6 = v(0,-6,13)

		local v16 = v(3.5,3.69,3) -- front win
		local v17 = v(-3.5,3.69,3)
		local v18 = v(5,2.998,0)
		local v19 = v(-5,2.998,0)

		local v20 = v(8,3.69,9) -- vent
		local v20l = v(-8,3.69,9)
		local v21 = v(7,3.69,7)
		local v21l = v(-7,3.69,7)
		local v22 = v(10,3.69,8)
		local v22l = v(-10,3.69,8)
		local v23 = v(9,3.69,6)
		local v23l = v(-9,3.69,6)
		local v24 = v(11,2.998,9)
		local v24l = v(-11,2.998,9)
		local v25 = v(8.75,2.998,4.5)
		local v25l = v(-8.75,2.998,4.5)

		local v30 = v(3.7,-2,-4.25)
		local v32 = v(2.7,-3.5,-4.25)
		local v33 = v(-2.7,-3.5,-4.25)
		local v34 = v(3.45,-2,-4.25)
		local v36 = v(2.6,-3.33,-4.25)
		local v37 = v(-2.6,-3.33,-4.25)
		local v38 = v(3.2,-2.15,-3.65)
		local v40 = v(2.4,-3.33,-3.65)
		local v41 = v(-2.4,-3.33,-3.65)
		local v42 = v(.5,-3.5,2.2)
		local v43 = v(-.5,-3.5,2.2)

		local v50 = v(15,-.264,-6.55) -- rft socket
		local v51 = v(15,-.601,-3.671)
		local v52 = v(15,-1.253,-6.012)
		local v53 = v(15,-1.421,-4.519)
		local v54 = v(11,-1.253,-6.012)
		local v55 = v(11,-1.421,-4.519)

		local v60 = v(23,-.278,9.347) -- rbt socket
		local v61 = v(23,-.615,12.332)
		local v62 = v(23,-1.267,9.991)
		local v63 = v(23,-1.435,11.483)
		local v64 = v(19,-1.267,9.991)
		local v65 = v(19,-1.435,11.483)

		local v70 = v(8,.4,-11.2) -- r rev t socket
		local v71 = v(5,.4,-11.2)
		local v72 = v(7.25,1.31,-11.2)
		local v73 = v(5.75,1.31,-11.2)
		local v74 = v(7.25,1.31,-7.25)
		local v75 = v(5.75,1.31,-7.25)

		local v100 = v(23.465,0,13)
		local v102 = v(23.465,0,12.5)
		local v104 = v(0,5.415,13)
		local v106 = v(0,5.415,12.5)

		local v114 = v(0,-5.415,13)
		local v116 = v(0,-5.415,12.5)

		set_material('black', .03,.02,.02,1,.2,.2,.2,30)
		set_material('text',1,1,1,.7,.3,.3,.3,5)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)

		--bottom
		use_material('bot')
		texture('bot1.png',v(.5,.5,0), v(.0192,0,0), v(0,0,-2))
		tri(v0,v6,v1)
		xref_tri(v0,v2,v6)
		xref_quad(v100,v102,v116,v114)

		-- scoop
		quad(v32,v42,v43,v33)
		quad(v36,v37,v41,v40)
		texture('tile0.png',v(-.05,.265,0),v(0,0,.9), v(0,.5,0))
		xref_tri(v30,v42,v32)
		xref_quad(v34,v36,v40,v38)

		texture(nil)
		use_material('black')
		quad(v32,v33,v37,v36)
		xref_quad(v30,v32,v36,v34)

		-- bottom nazzle sockets
		use_material('bot')
		texture('tile0.png', v(.05,.21,0),v(.5,0,0),v(0,0,1.41))
		xref_quad(v52,v53,v55,v54)
		xref_tri(v50,v52,v54)
		xref_tri(v51,v55,v53)
		texture('tile0.png', v(.05,.935,0),v(.5,0,0),v(0,0,1.41))
		xref_quad(v62,v63,v65,v64)
		xref_tri(v60,v62,v64)
		xref_tri(v61,v65,v63)

		--top
		use_material('top')
		texture('top1.png',v(.5,.5,0), v(.0192,0,0), v(0,0,-2))
		tri(v4,v16,v17)
		xref_tri(v0,v4,v2)
		quad(v0,v1,v19,v18)
		xref_quad(v0,v18,v16,v4)
		xref_quad(v100,v104,v106,v102)

		-- back top nazzle sockets
		texture('tile0.png', v(-.65,.51,0), v(.41,-.21,0), v(0,0,1.04))
		quad(v20,v22,v23,v21)
		tri(v20,v24,v22)
		tri(v21,v23,v25)
		texture('tile0.png', v(.65,.51,0), v(.41,.21,0), v(0,0,1.04))
		quad(v20l,v21l,v23l,v22l)
		tri(v22l,v24l,v20l)
		tri(v25l,v23l,v21l)

		-- rev nazzle sockets
		texture('tile0.png', v(0,.48,0),v(.692,0,0),v(0,0,.58))
		xref_quad(v72,v73,v75,v74)
		xref_tri(v70,v72,v74)
		xref_tri(v71,v75,v73)

		-- back
		texture(nil)
		use_material('black')
		xref_quad(v2,v100,v114,v6)
		xref_quad(v2,v4,v104,v100)



		if lod > 1 then
			--gun
			call_model('rattle_gun',v(0,0,0),v(1,0,0),v(0,1,0),1)

			--greebles
			use_material('chrome')
			texture('naz.png', v(0,.99,0), v(.5,0,0), v(0,0,1))
			xref_cylinder(3*lod, v(13,0,12.5), v(13.2,0,12.5), v(0,1,0), 1)
			xref_cylinder(3*lod, v(13.4,0,12.5), v(13.6,0,12.5), v(0,1,0), 1)
			xref_cylinder(3*lod, v(13.8,0,12.5), v(14,0,12.5), v(0,1,0), 1)
			xref_cylinder(3*lod, v(14.2,0,12.5), v(14.4,0,12.5), v(0,1,0), 1)
			xref_cylinder(3*lod, v(14.6,0,12.5), v(14.8,0,12.5), v(0,1,0), 1)
			xref_cylinder(3*lod, v(15,0,12.5), v(15.2,0,12.5), v(0,1,0), 1)

			texture('tex0.png')
			xref_ring(3*lod, v(4,0,12.5), v(15,0,12.5), v(0,1,0), .2)
			xref_cylinder(3*lod, v(4,0,12.71), v(4,0,12), v(1,0,0), .2)

			sphere_slice(3*lod,lod, 0, .3*math.pi, Matrix.translate(v(4,0,11.3)) * Matrix.rotate(.5*math.pi,v(1,0,0)) * Matrix.scale(v(1.1,1.1,1.1)))
			sphere_slice(3*lod,lod, 0, .3*math.pi, Matrix.translate(v(-4,0,11.3)) * Matrix.rotate(.5*math.pi,v(1,0,0)) * Matrix.scale(v(1.1,1.1,1.1)))
		end


	end,

	dynamic = function(lod)
		--select2 = 40
		selector2()
		if select2 < 17 then
			set_material('top', .5,.55,.7,1,.8,.8,1.3,75)
			set_material('bot', .5,.55,.7,1,.8,.8,1.3,75)
		else
			if select2 < 34 then
				set_material('top', get_arg_material(0))
				set_material('bot', .5,.55,.7,1,.8,.8,1.3,75)
			else
				if select2 < 51 then
					set_material('top', get_arg_material(1))
					set_material('bot', .5,.55,.7,1,.8,.8,1.3,75)
				else
					if select2 < 67 then
						set_material('top', get_arg_material(0))
						set_material('bot', get_arg_material(0))
					else
						if select2 < 84 then
							set_material('top', get_arg_material(0))
							set_material('bot', get_arg_material(1))
						else
							if select2 > 83 then
								set_material('top', get_arg_material(1))
								set_material('bot', get_arg_material(1))
							end
						end
					end
				end
			end
		end

		if lod > 1 then
			-- labels
			use_material('text')
			local reg = get_label()
			zbias(1, v(18,.693,3),v(0,1,-.57693))
			text(reg,v(18,.693,3),v(0,1,-.57693),v(-.5,0,-1),3, {center = true})
			zbias(1, v(-18.5,-.578,3),v(0,-1,-.5769))
			text(reg,v(-18.5,-.578,3),v(0,-1,-.5769),v(-.5,0,1),3, {center = true})
			zbias(0)
		end

		if lod > 2 then
			-- specials
			local scanpos_r = v(4.7,4.09,9.9)
			local scanpos_l = v(-4.7,4.09,9.9)
			local ecmpos_r  = v(6.4,2.8,.2)
			local ecmpos_l  = v(-6.4,2.8,.2)
			if get_equipment('SCANNER') == 'SCANNER' then
				use_material('bot')
				selector4()
				if select4 < 51 then
					call_model('scanner_-',scanpos_l,v(1,0,0),v(-.22,1,-.12),2)
				else
					call_model('scanner_+',scanpos_r,v(1,0,0),v(.22,1,-.12),2)
				end
			end

			if get_equipment('ECM') == 'ECM_BASIC' then
				use_material('top')
				selector4()
				if select4 < 51 then
					call_model('ecm_1',ecmpos_r,v(1,0,0),v(0,1,0),1.5)
				else
					call_model('ecm_1',ecmpos_l,v(1,0,0),v(0,1,0),1.5)
				end
			else
				if get_equipment('ECM') == 'ECM_ADVANCED' then
					use_material('top')
					selector4()
					if select4 < 51 then
						call_model('ecm_2',ecmpos_r,v(1,0,0),v(0,1,0),1.5)
					else
						call_model('ecm_2',ecmpos_l,v(1,0,0),v(0,1,0),1.5)
					end
				end
			end

			local mappos_0  = v(0,0,-12.8)
			local mappos_1  = v(-9,0,-12.8)
			if get_equipment('SCANNER') == 'SCANNER' then
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					call_model('antenna_1',mappos_0, v(1,0,0), v(0,1,0), 2)
				else
					if get_equipment('LASER', 2) == 'PULSECANNON_DUAL_1MW' then
						call_model('antenna_1',mappos_0, v(1,0,0), v(0,1,0), 2)
					else
						call_model('antenna_1',mappos_1, v(1,0,0), v(0,1,0), 2)
					end
				end
			end

			--select4 = 60
			selector4()
			if select4 < 51 then
				zbias(1,v(13.43,1.753,3.05),v(0,1,0))
				call_model('decal', v(13.43,1.753,3.05), v(0,1,0), v(-2,.577,1), 2)
				zbias(1,v(-14.36,1.752,4.9), v(0,1,0))
				call_model('squadsign_1',v(-14.36,1.752,4.9), v(0,1,0), v(2,.577,1), 2)
				zbias(0)
			else
				if select4 > 50 then
					zbias(1,v(-14.36,1.752,4.9), v(0,1,0))
					call_model('decal', v(-14.36,1.752,4.9), v(0,1,0), v(2,.577,1), 2)
					zbias(1,v(13.43,1.753,3.05), v(0,1,0))
					call_model('squadsign_1', v(13.43,1.753,3.05), v(0,1,0), v(-2,.577,1), 2)
					zbias(0)
				end
			end
		end

		local M_T1 = v(0,1.1,15)
		local M_T2 = v(0,-1.1,15)
		local M_T3 = v(1.9,0,15)
		local R_T = v(6.5,.8,-12)
		local FT_T = v(6.5,2,-10)
		local BT_T = v(8.5,4.4,7.5)
		local FB_T = v(14.5,-2.1,-5.265)
		local BB_T = v(22.5,-2.1,10.737)
		local FR_T = v(15.7,-.794,-5.204)
		local BR_T = v(23.7,-.808,10.799)
		local FL_T = v(-15.7,-.794,-5.204)
		local BL_T = v(-23.7,-.808,10.799)

		thruster(M_T1,v(0,0,1),20,true)
		thruster(M_T2,v(0,0,1),20,true)
		xref_thruster(M_T3,v(0,0,1),20,true)
		xref_thruster(R_T,v(0,0,-1),10,true)
		xref_thruster(FT_T,v(0,1,0),5)
		xref_thruster(BT_T,v(0,1,0),5)
		xref_thruster(FB_T,v(0,-1,0),5)
		xref_thruster(BB_T,v(0,-1,0),5)
		thruster(FR_T,v(1,0,0),5)
		thruster(BR_T,v(1,0,0),5)
		thruster(FL_T,v(-1,0,0),5)
		thruster(BL_T,v(-1,0,0),5)
	end
})

define_model('rattle_pad_br', {
	info =	{
		lod_pixels = {2,5,10,0},
		bounding_radius = 2,
		materials = {'pad', 'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('chrome')
		texture('tex0.png')
		sphere_slice(3*lod,2*lod, 0, .5*math.pi, Matrix.translate(v(0,.3,0)) * Matrix.scale(v(.4,.4,.4)))
		use_material('pad')
		texture('pad_br.png',v(.49,.5,0), v(.4,-.02,0), v(0,0,-1.15))
		cylinder(6, v(0,0,0), v(0,.3,0), v(.5,0,1), 1.05)
	end,
	dynamic = function(lod)
		--select2 = 40
		selector2()
		if select2 < 51 then
			set_material('pad', .5,.55,.7,1,.8,.8,1.3,75)
		else
			if select2 < 67 then
				set_material('pad', get_arg_material(0))
			else
				if select2 > 66 then
					set_material('pad', get_arg_material(1))
				end
			end
		end
	end
})

define_model('rattle_pad_bl', {
	info =	{
		lod_pixels = {2,5,10,0},
		bounding_radius = 2,
		materials = {'pad', 'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('chrome')
		texture('tex0.png')
		sphere_slice(3*lod,2*lod, 0, .5*math.pi, Matrix.translate(v(0,.3,0)) * Matrix.scale(v(.4,.4,.4)))
		use_material('pad')
		texture('pad_bl.png',v(.49,.5,0), v(.4,-.02,0), v(0,0,-1.15))
		cylinder(6, v(0,0,0), v(0,.3,0), v(.5,0,1), 1.05)
	end,

	dynamic = function(lod)
		--select2 = 40
		selector2()
		if select2 < 51 then
			set_material('pad', .5,.55,.7,1,.8,.8,1.3,75)
		else
			if select2 < 67 then
				set_material('pad', get_arg_material(0))
			else
				if select2 > 66 then
					set_material('pad', get_arg_material(1))
				end
			end
		end
	end
})

define_model('rattle_pad_f', {
	info =	{
		lod_pixels = {2,5,10,0},
		bounding_radius = 2,
		materials = {'pad', 'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		use_material('chrome')
		texture('tex0.png')
		sphere_slice(3*lod,2*lod, 0, .5*math.pi, Matrix.translate(v(0,.3,0)) * Matrix.scale(v(.4,.4,.4)))
		use_material('pad')
		texture('pad_f.png',v(.49,.5,0), v(.4,-.02,0), v(0,0,-1.15))
		cylinder(6, v(0,0,0), v(0,.3,0), v(.5,0,1), 1.05)
	end,

	dynamic = function(lod)
		--select2 = 40
		selector2()
		if select2 < 51 then
			set_material('pad', .5,.55,.7,1,.8,.8,1.3,75)
		else
			if select2 < 67 then
				set_material('pad', get_arg_material(0))
			else
				if select2 > 66 then
					set_material('pad', get_arg_material(1))
				end
			end
		end
	end
})

define_model('rattle_uc_cage', {
	info =	{
		lod_pixels = {5,10,20,0},
		bounding_radius = 5,
		materials = {'d_grey'},
	},
	static = function(lod)
		local v0 = v(18.475,-1.255,8.816) -- back uc cage
		local v1 = v(17.236,-1.528,8.707)
		local v2 = v(19.159,-.978,7.778)
		local v3 = v(16.681,-1.524,7.562)
		local v4 = v(18.604,-.974,6.633)
		local v5 = v(17.365,-1.247,6.524)
		local v10 = v(18.475,-.255,8.816)
		local v11 = v(17.236,-.528,8.707)
		local v12 = v(19.159,-.022,7.778)
		local v13 = v(16.681,-.524,7.562)
		local v14 = v(18.604,-.026,6.633)
		local v15 = v(17.365,-.247,6.524)

		local v20 = v(13.595,-1.255,-.944)  -- front uc cage
		local v21 = v(12.356,-1.528,-1.053)
		local v22 = v(14.279,-.978,-1.982)
		local v23 = v(11.801,-1.524,-2.198)
		local v24 = v(13.724,-.974,-3.127)
		local v25 = v(12.485,-1.247,-3.236)
		local v30 = v(13.595,-.255,-.944)
		local v31 = v(12.356,-.528,-1.053)
		local v32 = v(14.279,-.022,-1.982)
		local v33 = v(11.801,-.524,-2.198)
		local v34 = v(13.724,-.026,-3.127)
		local v35 = v(12.485,-.247,-3.236)

		set_material('d_grey', .1,.1,.11,1,.3,.3,.3,5)
		use_material('d_grey')
		texture('tex0.png')
		-- back uc cage
		xref_quad(v10,v15,v14,v12)
		xref_quad(v15,v10,v11,v13)
		xref_quad(v0,v10,v12,v2)
		xref_quad(v12,v14,v4,v2)
		xref_quad(v14,v15,v5,v4)
		xref_quad(v11,v1,v3,v13)
		xref_quad(v13,v3,v5,v15)
		xref_quad(v10,v0,v1,v11)
		-- front uc cage
		xref_quad(v30,v35,v34,v32)
		xref_quad(v35,v30,v31,v33)
		xref_quad(v20,v30,v32,v22)
		xref_quad(v32,v34,v24,v22)
		xref_quad(v34,v35,v25,v24)
		xref_quad(v31,v21,v23,v33)
		xref_quad(v33,v23,v25,v35)
		xref_quad(v30,v20,v21,v31)
	end
})

define_model('rattle_uc', {
	info =	{
		lod_pixels = {5,50,100,0},
		bounding_radius = 5,
		materials = {'null', 'chrome', 'd_grey'},
	},
	static = function(lod)

		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('d_grey', .1,.1,.11,1,.3,.3,.3,5)  -- .1,.1,.11,1,.3,.3,.3,5)
	end,
	dynamic = function(lod)

		if get_animation_position('WHEEL_STATE') ~= 0 then
			call_model('rattle_uc_cage',v(0,0,0),v(1,0,0),v(0,1,0),1)

			local trans = math.clamp(get_animation_position('WHEEL_STATE'),0,1)
			use_material('chrome')
			texture('models/ships/sidewinder/alu_tiled.png', v(0,.4+2*trans,0), v(.5,0,0),v(0,-.5,0))
			xref_tapered_cylinder(3*lod,v(13.04,-.274,-2.09), v(13.04+trans,-1.2-5*trans,-2.09-2*trans), v(0,0,1), .3,.2)
			xref_tapered_cylinder(3*lod,v(17.92,-.274,7.67), v(17.92+2*trans,-1.2-4.9*trans,7.67+trans), v(0,0,1), .3,.2)
			call_model('rattle_pad_f', v(13.04+trans,-1.251-5*trans,-2.09-2*trans), v(1,0,0), v(-.2307+.2307*trans,1,.11534-.11534*trans), 1.05)
			call_model('rattle_pad_f', v(-13.04-trans,-1.251-5*trans,-2.09-2*trans), v(1,0,0), v(.2307-.2307*trans,1,.11534-.11534*trans), 1.05)
			call_model('rattle_pad_br', v(17.91+2*trans,-1.251-5*trans,7.65+trans), v(1,0,0), v(-.2307+.2307*trans,1,.11534-.11534*trans), 1.05)
			call_model('rattle_pad_bl', v(-17.91-2*trans,-1.251-5*trans,7.65+trans), v(1,0,0), v(.2307-.2307*trans,1,.11534-.11534*trans), 1.05)

			use_material('null')
			xref_circle(6, v(13.04,-1.25,-2.09), v(0,-1,-.1095), v(1,.2307,0), 1.05)
			xref_circle(6, v(17.92,-1.25,7.67), v(0,-1,-.1095), v(1,.2307,0), 1.05)
		end
	end
})

define_model('rattle_dash', {
	info =	{
		bounding_radius = 10,
		materials = {'dashlite'},
	},
	static = function(lod)
		set_material('dashlite', 1,1,1,.9,0,0,0,0,1,1,1)
	end,
	dynamic = function(lod)
		local v0 = v(5,2.999,0)
		local v1 = v(-5,2.999,0)
		local v2 = v(5,2.001,2)
		local v3 = v(-5,2.001,2)
		local timer = math.fmod((get_time('SECONDS')*0.2),1)
		use_material('dashlite')
		if timer < .17 then
			texture('models/ships/sidewinder/dash_lit_01.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
		else
			if timer < .34 then
				texture('models/ships/sidewinder/dash_lit_02.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
			else
				if timer < .51 then
					texture('models/ships/sidewinder/dash_lit_03.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
				else
					if timer < .68 then
						texture('models/ships/sidewinder/dash_lit_04.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
					else
						if timer < .85 then
							texture('models/ships/sidewinder/dash_lit_05.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
						else
							if timer > .84 then
								texture('models/ships/sidewinder/dash_lit_06.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
							end
						end
					end
				end
			end
		end
		quad(v0,v1,v3,v2)
	end
})

define_model('rattle_scoop', {
	info =	{
		lod_pixels = {5,50,100,0},
		bounding_radius = 5,
	},

	static = function(lod)
		local v140 = v(3.2,-2.15,-3.65)
		local v141 = v(-3.2,-2.15,-3.65)
		local v142 = v(2.4,-3.33,-3.65)
		local v143 = v(-2.4,-3.33,-3.65)
		if lod > 1 then
			texture('v_glow.png', v(0,.3,0), v(.5,0,0), v(0,.5,0))
		end
		geomflag(0x100)
		quad(v140,v142,v143,v141)
		geomflag(0)
	end
})

define_model('rattlesnake', {
	info = 	{
		scale = .75,
		lod_pixels = {.1,10,100,0},
		bounding_radius = 32,
		materials = {'chrome', 'cabin', 'metal', 'no_shade', 'text', 'glow', 'e_glow', 'v_glow', 'win', 'black'},
		tags = {'ship'},
		ship_defs = {
			{
				name='Sidewinder',
				forward_thrust = -4e6,
				reverse_thrust = 3e6,
				up_thrust = 2e6,
				down_thrust = -2e6,
				left_thrust = -2e6,
				right_thrust = 2e6,
				angular_thrust = 10e6,
				gun_mounts =
				{
					{ v(0,0,-16), v(0,0,-1) },
					{ v(0,0,15), v(0,0,1) },
				},
				max_cargo = 30,
				max_laser = 2,
				max_missile = 0,
				max_fuelscoop = 1,
				max_cargoscoop = 1,
				capacity = 30,
				hull_mass = 20,
				fuel_tank_mass = 10,
				thruster_fuel_use = 0.00035,
				price = 44000,
				hyperdrive_class = 2,
			}
		}
	},

	static = function(lod)
		local  v0 = v(13,0,-13)
		local  v1 = v(-13,0,-13)
		local  v2 = v(26,0,13)
		local  v3 = v(-26,0,13)
		local  v4 = v(0,6,13)
		local  v6 = v(0,-6,13)
		local  v8 = v(13,0,12.5)
		local  v9 = v(0,3,12.5)
		local v10 = v(0,-3,12.5)
		local v12 = v(8.6,0,12)
		local v13 = v(0,2,12)
		local v14 = v(0,-2,12)
		local v15 = v(-8.6,0,12)

		local v16 = v(3.5,3.69,3) -- front win
		local v17 = v(-3.5,3.69,3)
		local v18 = v(5,2.998,0)
		local v19 = v(-5,2.998,0)

		local v26 = v(-10,3.69,8) -- back top thruster socket
		local v27 = v(-9,3.69,6)
		local v28 = v(-11,2.998,9)
		local v30 = v(-8.75,2.998,4.5)
		local v32 = v(8.5,3.9,7.5) -- rbtt nazzle
		local v33 = v(-8.5,3.9,7.5) -- lbtt nazzle

		local v40 = v(4,0,5) -- cabin
		local v41 = v(-4,0,5)
		local v42 = v(4,4.15,5)
		local v43 = v(-4,4.15,5)
		local v44 = v(5,0,0)
		local v45 = v(-5,0,0)
		local v46 = v(5,2.998,0)
		local v47 = v(-5,2.998,0)
		local v48 = v(5,2,2)
		local v49 = v(-5,2,2)

		local v50 = v(15,-.264,-6.55) -- rft socket
		local v51 = v(15,-.601,-3.671)
		local v52 = v(15,-1.253,-6.012)
		local v53 = v(15,-1.421,-4.519)
		local v56 = v(15.2,-.794,-5.204) -- rft nazzle
		local v57 = v(-15.2,-.794,-5.204) -- lft nazzle
		local v58 = v(14.5,-1.6,-5.265) -- rfbt nazzle
		local v59 = v(-14.5,-1.6,-5.265) -- lfbt nazzle

		local v60 = v(23,-.278,9.347) -- rbt socket
		local v61 = v(23,-.615,12.332)
		local v62 = v(23,-1.267,9.991)
		local v63 = v(23,-1.435,11.483)
		local v66 = v(23.2,-.808,10.799) -- rbt nazzle
		local v67 = v(-23.2,-.808,10.799) -- lbt nazzle
		local v68 = v(22.5,-1.6,10.737) -- rbbt nazzle
		local v69 = v(-22.5,-1.6,10.737) -- lbbt nazzle

		local v70 = v(8,.4,-11.2) -- r rev t socket
		local v71 = v(5,.4,-11.2)
		local v72 = v(7.25,1.31,-11.2)
		local v73 = v(5.75,1.31,-11.2)
		local v76 = v(6.5,.8,-11.5) -- r rev nazzle
		local v77 = v(-6.5,.8,-11.5) -- l rev nazzle
		local v78 = v(6.5,1.5,-10) -- ftt nazzles
		local v79 = v(-6.5,1.5,-10)

		local v100 = v(23.465,0,12.5) -- back
		local v102 = v(0,5.415,12.5)
		local v104 = v(0,-5.415,12.5)

		local v130 = v(0,1.1,13.3)  -- MT nazzles
		local v131 = v(0,-1.1,13.3)
		local v132 = v(1.9,0,13.3)
		local v133 = v(-1.9,0,13.3)

		local v140 = v(3.2,-2.15,-3.65) -- scoop
		local v141 = v(-3.2,-2.15,-3.65)
		local v142 = v(2.4,-3.33,-3.65)
		local v143 = v(-2.4,-3.33,-3.65)

		set_material('metal', .11,.1,.15,1,.3,.3,.35,30)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('cabin', .15,.2,.21,1,.3,.3,.3,10)
		set_material('black', 0,0,0,1,0,0,0,0)
		set_material('no_shade', .1,.1,.1,1,0,0,0,0)
		set_material('glow', .5,.5,.5,1,.5,.5,.5,100,2,1.5,1)
		set_material('win', 1.2,1,2,.4,1.2,1,2,100)

		--collision mesh
		if lod == 1 then
			texture(nil)
			tri(v0,v1,v4)
			xref_tri(v0,v4,v2)
			tri(v0,v6,v1)
			xref_tri(v0,v2,v6)
			quad(v2,v4,v3,v6)
		end

		if lod > 1 then
			use_material('metal')
			--back
			texture('tex1.png',v(0,.5,0), v(.05,0,0), v(0,.08,0))
			xref_quad(v100,v102,v9,v8)
			xref_quad(v100,v8,v10,v104)
			-- thruster excavation
			xref_quad(v8,v9,v13,v12)
			xref_quad(v8,v12,v14,v10)
			quad(v12,v13,v15,v14)

			-- thruster sockets
			texture('socket_l.png', v(0,.1,0), v(0,0,-1), v(-.05,.5,0))
			xref_quad(v50,v51,v53,v52)
			texture('socket_l.png', v(0,.9,0), v(0,0,-1), v(-.05,.5,0))
			xref_quad(v60,v61,v63,v62)
			texture('socket_l.png', v(0,0,0), v(.385,0,0), v(0,1,0))
			xref_quad(v70,v71,v73,v72)

			-- cabin
			use_material('cabin')
			texture('socket_l.png', v(0,0,0), v(.5,0,0), v(0,-.5,0))
			quad(v40,v41,v43,v42)
			texture('socket_l.png', v(0,0,0), v(0,0,1), v(0,-.5,0))
			xref_quad(v40,v42,v46,v44)
			texture(nil)
			use_material('no_shade')
			quad(v40,v44,v45,v41)

			if lod > 2 then
				-- dash
				texture('dash0.png', v(.5,.02,0), v(.11,0,0), v(0,0,-4.5))
				quad(v46,v47,v49,v48)
				zbias(1,v(0,2.499,1),v(0,1,0))
				call_model('rattle_dash',v(0,0,0),v(1,0,0),v(0,1,0),1)
				zbias(0)
				-- pilot
				--call_model('pilot1',v(0,2.7,3.1),v(1,0,0),v(0,1,0),.33)
				call_model('pilot_3_m',v(0,1.4,3.1),v(1,0,0),v(0,1,0),2)

			end
			texture(nil)
			use_material('glow')
			xref_ring(2*lod, v(2.5,0,5), v(2.5,4,5), v(0,0,1), .5)
			billboard('smoke.png',7,v(1.2,1,.8), {v(2.5,.75,4.49), v(-2.5,.75,4.49), v(2.5,1.75,4.49), v(-2.5,1.75,4.49),
			v(2.5,2.75,4.49), v(-2.5,2.75,4.49)})

			-- MT nazzles
			if lod > 2 then
				use_material('chrome')
				call_model('nazzle2_l', v132,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_l', v133,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_l', v130,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_l', v131,v(0,1,0),v(0,0,1),1)
			else
				use_material('chrome')
				call_model('nazzle2_s', v132,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_s', v133,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_s', v130,v(0,1,0),v(0,0,1),1)
				use_material('chrome')
				call_model('nazzle2_s', v131,v(0,1,0),v(0,0,1),1)
			end

			-- top nazzles
			if lod > 2 then
				use_material('chrome')
				call_model('nazzle2_n',v32,v(1,0,0),v(0,1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v33,v(1,0,0),v(0,1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v78,v(1,0,0),v(0,1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v79,v(1,0,0),v(0,1,0),.3)
				if lod > 3 then
					use_material('chrome')
					call_model('nazzle2_l',v76,v(0,1,0),v(0,0,-1),.4)
					use_material('chrome')
					call_model('nazzle2_l',v77,v(0,1,0),v(0,0,-1),.4)
				else
					use_material('chrome')
					call_model('nazzle2_s',v76,v(0,1,0),v(0,0,-1),.4)
					use_material('chrome')
					call_model('nazzle2_s',v77,v(0,1,0),v(0,0,-1),.4)
				end
				-- bottom nazzles
				use_material('chrome')
				call_model('nazzle2_n',v58,v(0,0,1),v(0,-1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v59,v(0,0,1),v(0,-1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v68,v(0,0,1),v(0,-1,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v69,v(0,0,1),v(0,-1,0),.3)
				-- left/right nazzles
				use_material('chrome')
				call_model('nazzle2_n',v56,v(0,0,1),v(1,0,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v57,v(0,0,1),v(-1,0,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v66,v(0,0,1),v(1,0,0),.3)
				use_material('chrome')
				call_model('nazzle2_n',v67,v(0,0,1),v(-1,0,0),.3)
			end

			-- eye (tt nazzle socket)
			use_material('v_glow')
			texture('v_glow.png', v(0,.6,0), v(0,0,1), v(0,-.9,0))
			xref_quad(v26,v27,v30,v28)



			-- uc
			call_model('rattle_uc',v(0,0,0),v(1,0,0),v(0,1,0),1)
			-- main
			call_model('rattle',v(0,0,0),v(1,0,0),v(0,1,0),1)
			-- win
			texture(nil)
			use_material('win')
			quad(v16,v18,v19,v17)

			call_model('posl_white',v(0,5.87,12.6),v(1,0,0),v(0,1,-.3),3)
			call_model('coll_warn',v(0,-5.87,12.6),v(1,0,0),v(0,-1,-.3),3)
			call_model('posl_green',v(25,.1,12),v(1,0,0),v(.3,1,-.11),3)
			call_model('coll_warn',v(25,-.1,12),v(1,0,0),v(.3,-1,-.11),3)
			call_model('posl_red',v(-25,.1,12),v(1,0,0),v(-.3,1,-.11),3)
			call_model('coll_warn',v(-25,-.1,12),v(1,0,0),v(-.3,-1,-.11),3)
		end
	end,

	dynamic = function(lod)

		set_material('e_glow', lerp_materials(get_time('SECONDS')*0.5, 	{.3, .3, .3, 1, 0, 0, 0, 0, .7, 1, 1.5 },
		{.3, .3, .3, 1, 0, 0, 0, 0, 1, .7, 1.5 }))
		set_material('v_glow', lerp_materials(get_time('SECONDS')*0.5, 	{0, 0, 0, 1, 0, 0, 0, 0, 1.2, 1.5, 0 },
		{0, 0, 0, 1, 0, 0, 0, 0, 1, 2, .5 }))

		-- scoop
		if lod > 1 then
			if get_equipment('FUELSCOOP') == 'FUEL_SCOOP' then
				use_material('v_glow')
			else
				use_material('black')
			end
		end
		call_model('rattle_scoop',v(0,0,0),v(1,0,0),v(0,1,0),1)


		-- collision mesh uc
		if lod == 1 then
			texture(nil)
			if get_animation_position('WHEEL_STATE') ~=0 then
				local trans = math.clamp(get_animation_position('WHEEL_STATE'),0,1)
				xref_cylinder(4, v(13.04+trans,-1.251-5*trans,-2.09-2*trans), v(13.04+trans,-.951-5*trans,-2.09-2*trans), v(0,0,1),1)
				xref_cylinder(4, v(17.92+2*trans,-1.251-5*trans,7.67+trans), v(17.92+2*trans,-.951-5*trans,7.67+trans), v(0,0,1),1)
				xref_ring(3,v(13.04,-.274,-2.09), v(13.04+trans,-1.2-5*trans,-2.09-2*trans), v(0,0,1), .15)
				xref_ring(3,v(17.92,-.274,7.67), v(17.92+2*trans,-1.2-5*trans,7.67+trans), v(0,0,1), .15)
			end
		else
			-- posl
			call_model('headlight',v(0,-1.38,-7),v(1,0,0),v(0,-1,-.15),3)
		end
	end
})
