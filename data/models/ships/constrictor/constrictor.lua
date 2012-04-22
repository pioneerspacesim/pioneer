define_model('conny_scoop', {
	info = {
		bounding_radius = 11,
		materials = { 'ncv', 'scoop' }
	},
	static = function(lod)
		set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
		use_material('ncv')

		texture('con_sc_b.png')
		load_obj('con_scoop.obj')

		texture('scoop.png')
		use_material('scoop')
		load_obj('con_sc_glow.obj')
	end,
	dynamic = function(lod)
		set_material('scoop', lerp_materials(get_time('SECONDS')*.3, {0, 0, 0, 1, 0, 0, 0, 1, 1, 2, 2.5 }, {0, 0, 0, 1, 0, 0, 0, 1, 1.5, 2.5, 2.5 }))
	end
})

define_model('conny_flap_fr', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_f.png',v(.0125,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,-10,0),v(0,10,0),v(-3.5,10,0),v(-3.5,-10,0))
	end
})

define_model('conny_flap_fl', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_f.png',v(.005,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,10,0),v(0,-10,0),v(3.5,-10,0),v(3.5,10,0))
	end
})

define_model('conny_flap_rr_r', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_rr.png',v(.0125,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,-10,0),v(0,10,0),v(-3.5,10,0),v(-3.5,-10,0))
	end
})

define_model('conny_flap_rr_l', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_rr.png',v(.005,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,10,0),v(0,-10,0),v(3.5,-10,0),v(3.5,10,0))
	end
})

define_model('conny_flap_rl_r', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_rl.png',v(.0125,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,-10,0),v(0,10,0),v(-3.5,10,0),v(-3.5,-10,0))
	end
})

define_model('conny_flap_rl_l', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
	},
	static = function(lod)
		texture('con_flap_rl.png',v(.005,.5,0),v(.142,0,0),v(0,-.0494,0))
		extrusion(v(0,0,0),v(0,0,.3),v(0,1,0),1,v(0,10,0),v(0,-10,0),v(3.5,-10,0),v(3.5,10,0))
	end
})

define_model('conny_piston_f', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 11,
		materials={'chrome'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
	end,
	dynamic = function(lod)
		local trans = 0.5*math.pi*math.clamp(1.5*(get_animation_position('WHEEL_STATE')-0.3), 0, 1)
		if lod > 1 then
			texture('models/ships/constrictor/metal.png')
			use_material('chrome')
		end
		ring(3*lod,v(0,0,0),v(0,0,-1.1-2*trans),v(0,1,0),.3)
		ring(3*lod,v(0,0,0),v(0,0,-1.3-4*trans),v(0,1,0),.25)
		ring(3*lod,v(0,0,0),v(0,0,-1.5-6.5*trans),v(0,1,0),.2)
	end
})

define_model('conny_w_front_0', {
	info = {
		lod_pixels={.1,10,30,0},
		bounding_radius = 13,
		materials={'chrome'}
	},
	static = function(lod)
		local divs = lod*3

		if lod > 1 then
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			use_material('chrome')
			texture('metal.png',v(.5,.1,0),v(.25,0,0),v(0,-.18,0))
		end
		ring(divs,v(3.5,0,0),v(-3.5,0,0),v(0,1,0),.6)
		ring(divs, v(0,0,0),v(0,10.2,0),v(0,0,1),.4)
		ring(divs, v(.5,10.2,0),v(-.5,10.2,0),v(0,1,0),.4)
		if lod > 1 then
			texture('tire.png',v(.51,.08,0),v(0,0,.95),v(0,.35,0)) --v(0,0,.95), v(0,.35,0))
		end
		xref_cylinder(lod*4,v(.5,10.2,0),v(2,10.2,0),v(0,0,1),1.8)
	end
})

define_model('conny_w_front', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 15,
		materials={'chrome', 'inside', 'hole', 'ncv'},
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1)
		set_material('hole', 0,0,0,0,0,0,0,0)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
	end,
	dynamic = function(lod)
		if get_animation_position('WHEEL_STATE') ~= 0 then
			local v0 = v(3.5,-4,10)
			local v1 = v(-3.5,-4,10)
			local v2 = v(3.5,0,10)
			local v3 = v(-3.5,0,10)

			local v6 = v2
			local v7 = v(3.5,0,-10)
			local v8 = v(0,0,10)
			local v9 = v(0,0,-10)

			local frot = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
			local wrot = 0.5*math.pi*math.clamp(1.5*(get_animation_position('WHEEL_STATE')-0.3), 0, 1)

			if lod > 1 then
				use_material('inside')
				texture('models/ships/constrictor/iron.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				extrusion(v(0,0,-10),v(0,0,10),v(0,1,0),1,v2,v0,v1,v3)

				use_material('chrome')
				texture('models/ships/constrictor/metal.png')
				sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-4,-1)))
			end

			call_model('conny_w_front_0',v(0,-2.2,-8),v(1,0,0),v(0,math.sin(wrot),math.cos(wrot)), 1.0)

			call_model('conny_piston_f',v(0,-4,-1),v(-1,0,0),v(0,-math.sin(.415*wrot),-math.cos(.415*wrot)),1)

			if lod > 1 then
				use_material('ncv')
			end
			call_model('conny_flap_fr',v(-3.5,0,0),v(-math.cos(frot),-math.sin(frot),0),v(0,0,-1),1)
			call_model('conny_flap_fl',v(3.5,0,0),v(-math.cos(frot),math.sin(frot),0),v(0,0,-1),1)

			-- cutout
			if lod > 1 then
				use_material('hole')
				zbias(1, v(0,0,0), v(0,1,0))
				xref_quad(v8, v6, v7, v9)
				zbias(0)
			end
		end
	end
})

define_model('conny_w_rear_0', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 9,
		materials = {'metal', 'chrome'},
	},
	static = function(lod)
		set_material('metal', .2,.23,.25,1,.35,.38,.4,10)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)

		use_material('metal')
		texture('metal.png',v(.5,.5,0),v(0,0,1),v(0,.5,0))
		extrusion(v(0,0,7),v(0,0,-7),v(0,1,0),1,v(.5,-.5,7),v(.5,.5,7),v(-.5,.5,7),v(-.5,-.5,7))

		use_material('chrome')
		sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-.5,0))*Matrix.rotate(math.pi,v(1,0,0))*Matrix.scale(v(.5,.5,.5)))

		texture('tire.png', v(.495,.515,0), v(0,0,.95), v(0,.35,0))
		cylinder(4*lod, v(.5,0,6), v(2,0,6), v(0,1,0), 1.8)
		cylinder(4*lod, v(.5,0,0), v(2,0,0), v(0,1,0), 1.8)
		cylinder(4*lod, v(.5,0,-6), v(2,0,-6), v(0,1,0), 1.8)

		cylinder(4*lod, v(-.5,0,6), v(-2,0,6), v(0,1,0), 1.8)
		cylinder(4*lod, v(-.5,0,0), v(-2,0,0), v(0,1,0), 1.8)
		cylinder(4*lod, v(-.5,0,-6), v(-2,0,-6), v(0,1,0), 1.8)
	end
})

define_model('conny_w_rear_r', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 14,
		materials={'chrome', 'inside', 'hole', 'ncv'},
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1)
		set_material('hole', 0,0,0,0,0,0,0,0)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
	end,
	dynamic = function(lod)
		if get_animation_position('WHEEL_STATE') ~= 0 then
			local v0 = v(3.5,-4,10)
			local v1 = v(-3.5,-4,10)
			local v2 = v(3.5,0,10)
			local v3 = v(-3.5,0,10)

			local v6 = v2
			local v7 = v(3.5,0,-10)
			local v8 = v(0,0,10)
			local v9 = v(0,0,-10)

			local frot = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
			local rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE')-.3 ,0,1)
			local trans = math.clamp(get_animation_position('WHEEL_STATE')-.3 ,0,1)

			if lod > 1 then
				use_material('inside')
				texture('models/ships/constrictor/iron.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				extrusion(v(0,0,-10),v(0,0,10),v(0,1,0),1,v2,v0,v1,v3)

				use_material('chrome')
				texture('models/ships/constrictor/metal.png',v(.5,.5,0),v(0,0,.5),v(0,1,0))
				sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-4,8)))
				sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-4,-8)))
			end

			tapered_cylinder(3*lod,v(0,-2.7+14.6*trans,0),v(0,-4,8),v(1,0,0),.3,.5)
			tapered_cylinder(3*lod,v(0,-2.7+14.6*trans,0),v(0,-4,-8),v(1,0,0),.3,.5)

			call_model('conny_w_rear_0',v(0,-2.2+14.6*trans,0),v(1,0,0),v(0,1,0),1)

			if lod > 1 then
				use_material('ncv')
			end
			call_model('conny_flap_rr_r',v(-3.5,0,0),v(-math.cos(frot),-math.sin(frot),0),v(0,0,-1),1)
			call_model('conny_flap_rr_l',v(3.5,0,0),v(-math.cos(frot),math.sin(frot),0),v(0,0,-1),1)

			-- cutout
			if lod > 1 then
				use_material('hole')
				zbias(1, v(0,0,0), v(0,1,0))
				xref_quad(v8, v6, v7, v9)
				zbias(0)
			end
		end
	end
})

define_model('conny_w_rear_l', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 14,
		materials={'chrome', 'inside', 'hole', 'ncv'},
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1)
		set_material('hole', 0,0,0,0,0,0,0,0)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
	end,
	dynamic = function(lod)
		if get_animation_position('WHEEL_STATE') ~= 0 then
			local v0 = v(3.5,-4,10)
			local v1 = v(-3.5,-4,10)
			local v2 = v(3.5,0,10)
			local v3 = v(-3.5,0,10)

			local v6 = v2
			local v7 = v(3.5,0,-10)
			local v8 = v(0,0,10)
			local v9 = v(0,0,-10)

			local frot = math.pi*math.clamp(get_animation_position('WHEEL_STATE'),0,.5)
			local rot = 0.5*math.pi*math.clamp(get_animation_position('WHEEL_STATE')-.3 ,0,1)
			local trans = math.clamp(get_animation_position('WHEEL_STATE')-.3 ,0,1)

			if lod > 1 then
				use_material('inside')
				texture('models/ships/constrictor/iron.png',v(.5,.5,0),v(.1,0,0),v(0,0,1))
				extrusion(v(0,0,-10),v(0,0,10),v(0,1,0),1,v2,v0,v1,v3)

				use_material('chrome')
				texture('models/ships/constrictor/metal.png',v(.5,.5,0),v(0,0,.5),v(0,1,0))
				sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-4,8)))
				sphere_slice(3*lod,lod,0,.5*math.pi, Matrix.translate(v(0,-4,-8)))
			end

			tapered_cylinder(3*lod,v(0,-2.7+14.6*trans,0),v(0,-4,8),v(1,0,0),.3,.5)
			tapered_cylinder(3*lod,v(0,-2.7+14.6*trans,0),v(0,-4,-8),v(1,0,0),.3,.5)

			call_model('conny_w_rear_0',v(0,-2.2+14.6*trans,0),v(1,0,0),v(0,1,0),1)

			if lod > 1 then
				use_material('ncv')
			end
			call_model('conny_flap_rl_r',v(-3.5,0,0),v(-math.cos(frot),-math.sin(frot),0),v(0,0,-1),1)
			call_model('conny_flap_rl_l',v(3.5,0,0),v(-math.cos(frot),math.sin(frot),0),v(0,0,-1),1)

			-- cutout
			if lod > 1 then
				use_material('hole')
				zbias(1, v(0,0,0), v(0,1,0))
				xref_quad(v8, v6, v7, v9)
				zbias(0)
			end
		end
	end
})

define_model('conny_pyl', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 3,
		materials={'inside'},
	},
	static = function(lod)
		set_material('inside', .2,.2,.2,1, 0,0,0, 1)
		use_material('inside')
		cylinder(6,v(0,0,0),v(0,0,-2),v(0,1,0),.41)
	end
})

define_model('conny_gun', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 5,
		materials={'chrome', 'matte', 'black'},
	},
	static = function(lod)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('black',0,0,0,1,0,0,0,1)
		set_material('matte', .2,.22,.25,1,.4,.42,.45,10)

		use_material('chrome')
		texture('gun.png',v(.5,.78,0),v(3,0,0),v(0,0,-.09))
		cylinder(3*lod,v(0,0,0),v(0,0,-4.5),v(0,1,0),.15)

		use_material('matte')
		texture('grill.png',v(.5,.995,0),v(.5,0,0),v(0,0,-.8))
		cylinder(3*lod,v(0,0,0),v(0,0,-2.5),v(0,1,0),.3)

		sphere_slice(4*lod,2*lod,0,.5*math.pi,Matrix.rotate(math.pi,v(1,0,0))*Matrix.scale(v(1,.5,1)))

		texture(nil)
		use_material('black')
		zbias(1,v(0,0,-4.5),v(0,0,-1))
		circle(3*lod,v(0,0,-4.5),v(0,0,-1),v(0,1,0),.12)
		zbias(0)
	end
})

local LASER_SCALE = {
	--PULSECANNON_1MW       = ??,
	--PULSECANNON_DUAL_1MW  = ??,
	PULSECANNON_2MW       = 0.1,
	PULSECANNON_RAPID_2MW = 0.2,
	PULSECANNON_4MW       = 0.3,
	PULSECANNON_10MW      = 0.4,
	PULSECANNON_20MW      = 0.5,
	MININGCANNON_17MW     = 0.6,
	SMALL_PLASMA_ACCEL    = 0.7,
	LARGE_PLASMA_ACCEL    = 0.8,
}

define_model('conny_equipment', {
	info = {
		lod_pixels = {.1,10,30,0},
		bounding_radius = 10,
		materials={'chrome', 'matte', 'ncv', 'hole', 'scoop'},
	},
	static = function(lod)
		set_material('matte', .2,.22,.25,1,.4,.42,.45,10)
		set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
		set_material('hole', 0,0,0,0,0,0,0,0)

		call_model('blank',v(0,0,0),v(1,0,0),v(0,1,0),0)
	end,
	dynamic = function(lod)

		if get_equipment('FUELSCOOP') or get_equipment('CARGOSCOOP') then
			call_model('conny_scoop', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		end

		if lod > 2 then
			local v1 = v(3,4,8.8) -- ecm
			local v2 = v(0,4,6.5) -- scanner

			if get_equipment('ECM') == 'ECM_BASIC' then
				use_material('matte')
				call_model('ecm_1',v1,v(1,0,0),v(0,1,0),.8)
			else
				if get_equipment('ECM') == 'ECM_ADVANCED' then
					use_material('matte')
					call_model('ecm_2',v1,v(1,0,0),v(0,1,0),.8)
				end
			end

			if get_equipment('SCANNER') == 'SCANNER' then
				use_material('matte')
				call_model('scanner_-',v2,v(1,0,0),v(0,1,0),1)
				call_model('antenna_1',v(-2,-1.8,-22),v(1,0,0),v(0,1,0),.8)
			end

			if get_equipment('LASER', 1) then
				local scale = LASER_SCALE[get_equipment('LASER',1)] or 0.1
				use_material('chrome')
				if get_equipment('LASER', 1) == 'PULSECANNON_DUAL_1MW' then
					texture('models/ships/constrictor/iron.png')
					call_model('conny_gun',v(7,-2,-15),v(1,0,0),v(0,1,0),.8)
					call_model('conny_gun',v(-7,-2,-15),v(1,0,0),v(0,1,0),.8)
				else
					texture('models/ships/constrictor/iron.png')
					call_model('conny_gun',v(0,-2,-22),v(1,0,0),v(0,1,0),.7+scale)
				end
			end

			if get_equipment('LASER', 2) then
				local scale = LASER_SCALE[get_equipment('LASER',2)] or 0.1
				use_material('chrome')
				if get_equipment('LASER', 2) == 'PULSECANNON_DUAL_1MW' then
					texture('models/ships/constrictor/iron.png')
					call_model('conny_gun',v(7,-2,13),v(-1,0,0),v(0,1,0),.8)
					call_model('conny_gun',v(-7,-2,13),v(-1,0,0),v(0,1,0),.8)
				else
					texture('models/ships/constrictor/iron.png')
					call_model('conny_gun',v(0,-2,15.5),v(-1,0,0),v(0,1,0),.7+scale)
				end
			end

			if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
				call_model('conny_pyl',v(-3.935,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
				call_model('d_unguided',v(-3.935,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)

				use_material('hole')
				zbias(2,v(-3.935,-2.5,3.4),v(0,1,0))
				circle(3*lod,v(-3.935,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
				zbias(0)
			else
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('conny_pyl',v(-3.935,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
					call_model('d_guided',v(-3.935,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
					use_material('hole')
					zbias(2,v(-3.935,-2.5,3.4),v(0,1,0))
					circle(3*lod,v(-3.935,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
					zbias(0)
				else
					if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
						call_model('conny_pyl',v(-3.935,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
						call_model('d_smart',v(-3.935,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
						use_material('hole')
						zbias(2,v(-3.935,-2.5,3.4),v(0,1,0))
						circle(3*lod,v(-3.935,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
						zbias(0)
					else
						if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
							call_model('conny_pyl',v(-3.935,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
							call_model('d_naval',v(-3.935,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
							use_material('hole')
							zbias(2,v(-3.935,-2.5,3.4),v(0,1,0))
							circle(3*lod,v(-3.935,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
							zbias(0)
						end
					end
				end
			end

			if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
				call_model('conny_pyl',v(-5.506,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
				call_model('d_unguided',v(-5.506,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
				use_material('hole')
				zbias(2,v(-5.506,-2.5,3.4),v(0,1,0))
				circle(3*lod,v(-5.506,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
				zbias(0)
			else
				if get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
					call_model('conny_pyl',v(-5.506,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
					call_model('d_guided',v(-5.506,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
					use_material('hole')
					zbias(2,v(-5.506,-2.5,3.4),v(0,1,0))
					circle(3*lod,v(-5.506,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
					zbias(0)
				else
					if get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
						call_model('conny_pyl',v(-5.506,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
						call_model('d_smart',v(-5.506,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
						use_material('hole')
						zbias(2,v(-5.506,-2.5,3.4),v(0,1,0))
						circle(3*lod,v(-5.506,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
						zbias(0)
					else
						if get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
							call_model('conny_pyl',v(-5.506,-2.5,3.4),v(1,0,0),v(0,1,0),-1)
							call_model('d_naval',v(-5.506,-2.5,4.9),v(1,0,0),v(0,1,0),.6666)
							use_material('hole')
							zbias(2,v(-5.506,-2.5,3.4),v(0,1,0))
							circle(3*lod,v(-5.506,-2.5,3.4),v(0,0,-1),v(0,1,0),.4)
							zbias(0)
						end
					end
				end
			end
		end
	end
})

define_model('conny_extra_0', {
	info = {
		bounding_radius = 22,
		materials={'glow'},
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		-- quads back r & l
		local v0 = v(8.720,2.526,8.2)
		local v1 = v(7.429,3.632,8.2)
		local v2 = v(8.720,2.526,14.2)
		local v3 = v(7.429,3.632,14.2)
		-- quad mid r
		local v4 = v(13.317,-1.415,-5.838)
		local v5 = v(12.132,-.399,-5.838)
		local v6 = v(13.317,-1.415,-4.36)
		local v7 = v(12.132,-.399,-4.36)
		-- quad mid l
		local v8 = v(-12.254,-.503,-5.891)
		local v9 = v(-13.383,-1.471,-5.891)
		local v10 = v(-12.254,-.503,-4.413)
		local v11 = v(-13.383,-1.471,-4.413)
		-- tri front r
		local v12 = v(9.664,.08,-9.313)
		local v13 = v(6.815,.889,-10.68)
		local v14 = v(7.751,1.68,-8.029)
		-- tri front l
		local v15 = v(-9.549,.218,-9.148)
		local v16 = v(-7.677,1.733,-7.997)
		local v17 = v(-6.528,1.047,-10.657)
		-- quad bot rear
		local v18 = v(1.78,-2,14.3)
		local v19 = v(-1.8,-2,14.3)
		local v20 = v(1.78,-2,9.65)
		local v21 = v(-1.8,-2,9.65)
		-- quads bot mid
		local v22 = v(3.356,-2,-1.218)
		local v23 = v(.459,-2,-1.218)
		local v24 = v(2.678,-2,-.582)
		local v25 = v(1.093,-2,-.582)

		local v26 = v(2.678,-2,1.06)
		local v27 = v(1.093,-2,1.06)
		local v28 = v(3.336,-2,1.685)
		local v29 = v(.459,-2,1.685)
		-- quads bot front
		local v30 = v(10.268,-2,-8.297)
		local v31 = v(9.158,-2,-8.297)
		local v32 = v(10.268,-2,-3.693)
		local v33 = v(9.158,-2,-3.693)

		local v34 = v(7.963,-2,-8.297)
		local v35 = v(6.853,-2,-8.297)
		local v36 = v(7.963,-2,-3.693)
		local v37 = v(6.853,-2,-3.693)
		-- back
		local v38 = v(11.077,-1.118,11.69)
		local v39 = v(6.45,2.9,17)
		local v40 = v(4.44,-1.118,17)
		local v41 = v(0,2.9,17)
		local v42 = v(0,-1.118,17)

		local v44 = v(-11.077,-1.118,11.69)
		local v45 = v(-6.45,2.9,17)
		local v46 = v(-4.44,-1.118,17)

		local trans = get_time('SECONDS')*.05

		set_material('glow',0, 0, 0, 1, 0, 0, 0, 1, 1, 1.5, 0)

		use_material('glow')
		texture('models/ships/constrictor/flow_1.png',v(0,trans,0),v(0,0,.05),v(1,0,0))
		zbias(3,v(0,0,0),v(0,1,0))
		-- back
		xref_quad(v0,v1,v3,v2)
		-- mid
		quad(v4,v5,v7,v6)
		quad(v8,v9,v11,v10)
		-- front
		tri(v12,v13,v14)
		tri(v15,v16,v17)
		zbias(0)
		zbias(3,v(0,0,0),v(0,-1,0))
		-- bot rear
		quad(v20,v18,v19,v21)
		-- bot mid
		quad(v22,v24,v25,v23)
		quad(v26,v28,v29,v27)
		quad(v22,v28,v26,v24)
		quad(v23,v25,v27,v29)
		-- bot front
		quad(v30,v32,v33,v31)
		quad(v34,v36,v37,v35)
		zbias(0)

		--texture('models/ships/constrictor/flow_2.png',v(0,-trans,0),v(0,.1,0),v(1,0,0))
		zbias(3,v(0,0,0),v(0,0,1))
		tri(v38,v39,v40)
		quad(v40,v39,v41,v42)

		--texture('models/ships/constrictor/flow_2.png',v(0,trans,0),v(0,.1,0),v(1,0,0))
		tri(v44,v46,v45)
		quad(v45,v46,v42,v41)
		zbias(0)
	end
})

define_model('conny_teeth', {
	info = {
		bounding_radius = 26,
		materials = {'teeth'},
	},
	static = function(lod)
		set_material('teeth', .6, .6, .6,.99, .5, .5, .5, 30)
		use_material('teeth')
		texture('teeth.png')
		zbias(5,v(0,0,0),v(0,1,0))
		load_obj('con_teeth.obj')
		zbias(0)
	end
})

define_model('conny_ver0', {
	info = {
		lod_pixels = { .1, 30, 120, 0 },
		bounding_radius = 28,
		materials = {'ncv'},
	},
	static = function(lod)
		set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
		texture('con_top.png')
		use_material('ncv')
		zbias(2,v(0,0,0),v(0,0,0))
		load_obj('con_top_cv1.obj')
		load_obj('con_top_cv0.obj')
		zbias(0)
		texture('con_back.png')
		load_obj('con_back.obj')

	end
})

define_model('conny_ver1', {
	info = {
		lod_pixels = { .1, 30, 120, 0 },
		bounding_radius = 28,
		materials = {'cv0'},
	},
	static = function(lod)
		texture('con_top2.png')
		use_material('cv0')
		zbias(2,v(0,0,0),v(0,0,0))
		load_obj('con_top_cv1.obj')
		load_obj('con_top_cv0.obj')
		zbias(0)

		texture('con_back.png')
		load_obj('con_back.obj')
	end,
	dynamic = function(lod)
		set_material('cv0', get_arg_material(0))
	end
})

define_model('conny_ver2', {
	info = {
		lod_pixels = { .1, 30, 50, 0 },
		bounding_radius = 28,
		materials = {'cv0', 'cv1'},
	},
	static = function(lod)
		texture('con_top.png')
		zbias(2,v(0,0,0),v(0,0,0))
		use_material('cv1')
		load_obj('con_top_cv1.obj')

		use_material('cv0')
		load_obj('con_top_cv0.obj')

		zbias(3,v(0,0,0),v(0,1,0))
		load_obj('con_tip_cv0.obj')
		zbias(0)

		texture('con_back.png')
		load_obj('con_back.obj')
	end,
	dynamic = function(lod)
		set_material('cv0', get_arg_material(0))
		set_material('cv1', get_arg_material(1))
	end
})

define_model('conny_ver3', {
	info = {
		lod_pixels = { .1, 30, 120, 0 },
		bounding_radius = 28,
		materials = {'cv0', 'cv1'},
	},
	static = function(lod)
		texture('con_top.png')
		zbias(2,v(0,0,0),v(0,1,0))
		use_material('cv0')
		load_obj('con_top_cv1.obj')
		load_obj('con_top_cv0.obj')

		use_material('cv1')
		zbias(3,v(0,0,0),v(0,1,0))
		load_obj('con_tip_cv0.obj')
		zbias(0)

		texture('con_back.png')
		use_material('cv0')
		load_obj('con_back.obj')
	end,
	dynamic = function(lod)
		set_material('cv0', get_arg_material(0))
		set_material('cv1', get_arg_material(1))
	end
})

define_model('conny_top', {
	info = {
		bounding_radius = 28,
	},
	static = function(lod)
	end,
	dynamic = function(lod)
		selector2()
		if select2 < 26 then
			call_model('conny_ver0',v(0,0,0),v(1,0,0),v(0,1,0),1)
		else
			if select2 < 51 then
				call_model('conny_ver1',v(0,0,0),v(1,0,0),v(0,1,0),1)
			else
				if select2 < 76 then
					call_model('conny_ver2',v(0,0,0),v(1,0,0),v(0,1,0),1)
				else
					if select2 > 75 then
						call_model('conny_ver3',v(0,0,0),v(1,0,0),v(0,1,0),1)
					end
				end
			end
		end
		selector3()
		if select3 >= 50 then
			call_model('conny_teeth',v(0,0,0),v(1,0,0),v(0,1,0),1)
		end
	end
})

define_model('conny', {
	info = {
		scale = 1.4, --1.5, -- should be 2.08, but didn't fits?
		lod_pixels = { .1, 30, 120, 0 },
		bounding_radius = 45,
		materials = {'text', 'cv0', 'cv1', 'ncv', 'chrome', 'matte', 'pit', 'pit_0', 'layer',
		'radio', 'glass', 'win', 'black', 'glow', 'e_glow', 'null'},
		tags = {'ship'},
	},
	static = function(lod)
		local rwhl_r = v(9.902,-2,6.19)
		local rwhl_l = v(-9.902,-2,6.19)
		local fwhl = v(0,-2,-11.2)

		zbias(2,v(0,0,0),v(0,-1,0))
		call_model('conny_w_front',fwhl,v(-1,0,0),v(0,-1,0),.4)
		call_model('conny_w_rear_r',rwhl_r,v(-1,0,0),v(0,-1,0),.4)
		call_model('conny_w_rear_l',rwhl_l,v(-1,0,0),v(0,-1,0),.4)


		if lod == 1 then
			load_obj('con_coll.obj')
			geomflag(0x100)
			quad(v(2.479,-2,2.9), v(7.046,-2,2.9), v(6.048,-4,2.9), v(3.477,-4,2.9))
			geomflag(0)
		else
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,30)
			set_material('text', .6, .6, .6,.99, .5, .5, .5, 30)
			set_material('black', .2,.2,.2,1,.1,.1,.1,5)
			set_material('null',0,0,0,0,0,0,0,0)
			set_material('ncv', .33,.35,.3,1,.63,.7,.83,30)
			set_material('layer', .4,.45,.5,.999,1.26,1.4,1.66,30)
			set_material('win', 0,0,.05,.4,1,1,1.5,100)
			set_material('matte', .2,.2,.2,1,.4,.42,.45,10)
			set_material('pit', .33,.35,.3,1,.63,.7,.83,10)
			set_material('pit_0', .33,.35,.3,.99,.63,.7,.83,10)
			--set_material('glass', 0,0,.05,.4,1,1,1.5,100)
			--set_material('radio', .3,.32,.35,1,.5,.55,.6,5)
			set_material('glow', 0,0,0,.99,.5,.5,.5,100,1.7,1.7,1.5)

			set_light(1, 0.05, v(0,7,-9), v(6,6,6))
			set_light(2, 0.05, v(0,4,-8), v(3,3,3))


			set_local_lighting(true)
			use_light(1)

			zbias(0)
			use_material('pit')
			texture('con_pit_f.png')
			load_obj('con_pit_f.obj')

			texture('con_pit_s.png')
			load_obj('con_pit_s.obj')

			use_light(2)
			texture('con_pit_b.png')
			load_obj('con_pit_b.obj')

			use_material('glow')
			zbias(1,v(0,0,0),v(0,0,-1))
			load_obj('con_pit_b.obj')
			zbias(0)

			if lod > 2 then
				call_model('pilot_3_m',v(.6,1.5,-9.1),v(1,0,0),v(0,1,0),.8) -- for scale 1.5
				call_model('pilot_4_m',v(-.6,1.5,-9.1),v(1,0,0),v(0,1,0),.8)
			end

			set_local_lighting(true)
			use_light(1)

			use_material('pit_0')
			texture('con_pit_d0.png')
			load_obj('con_pit_d.obj')

			use_material('glow')
			texture('con_pit_d1.png')
			zbias(1,v(0,0,0),v(0,1,0))
			load_obj('con_pit_d.obj')
			zbias(0)
			set_local_lighting(false)

			zbias(3,v(0,0,0),v(0,0,0))
			texture('con_top.png')
			use_material('ncv')
			load_obj('con_thrust_s1.obj')

			texture('con_back.png')
			load_obj('con_thrust_tb.obj')

			call_model('conny_equipment',v(0,0,0),v(1,0,0),v(0,1,0),1)

			if lod > 2 then
				texture(nil)
				use_material('null')
				zbias(4,v(0,0,0),v(0,0,0))
				load_obj('con_thrust_s0.obj')
				zbias(0)
			end

			use_material('ncv')
			texture('con_pyl.png')
			load_obj('con_pyl.obj')

			texture('con_bot.png')
			load_obj('con_bot.obj')

			call_model('posl_green',v(13.5,-2,0),v(1,0,0),v(0,-1,0),1.5)
			call_model('posl_red',v(-13.5,-2,0),v(1,0,0),v(0,-1,0),1.5)
			call_model('headlight',v(0,-2,-19),v(1,0,0),v(0,-1,0),1.5)
			call_model('blank',v(0,0,0),v(1,0,0),v(0,1,0),0)

			texture('scoop.png')
			use_material('black')
			load_obj('con_engine.obj')

			zbias(3,v(0,0,0),v(0,0,1))
			use_material('e_glow')
			load_obj('con_e_glow.obj')

			texture(nil)
			use_material('null')
			zbias(5,v(0,0,0),v(0,0,1))
			load_obj('con_e_cut.obj')
			zbias(0)

			texture(nil)
			zbias(4,v(0,0,0),v(0,1,0))
			use_material('win')
			load_obj('con_win_0.obj')

			call_model('conny_top',v(0,0,0),v(1,0,0),v(0,1,0),1)

			if lod > 2 then
				call_model('conny_extra_0',v(0,0,0),v(1,0,0),v(0,1,0),1)
			end

			texture('con_bot.png')
			use_material('layer')
			zbias(4,v(0,0,0),v(0,-1,0))
			load_obj('con_bot_layer.obj')
			zbias(0)

			texture('con_back.png')
			zbias(4,v(0,0,0),v(0,0,1))
			load_obj('con_back.obj')
			zbias(0)

			texture('con_top.png')
			zbias(4,v(0,0,0),v(0,1,0))
			load_obj('con_top_layer.obj')


			texture('con_win.png')
			zbias(5,v(0,0,0),v(0,1,0))
			load_obj('con_win_f.obj')
			zbias(0)

			call_model('coll_warn',v(0,4,16.7),v(1,0,0),v(0,1,0),1.5)

			if lod > 2 then
				zbias(5,v(0,0,0),v(-1,.87,0))    --v(-12.32,-.5,-1)
				call_model('squadsign_1',v(-12.2,-.5,-1.5),v(0,1,0),v(1,.87,0),1.5)
				zbias(5,v(0,0,0),v(1,.87,0))
				call_model('decal',v(12.2,-.5,-3),v(0,1,0),v(-1,.87,0),1.5)
				zbias(0)
			end
		end
		texture(nil)
	end,

	dynamic = function(lod)
		set_material('cv0', get_arg_material(0))
		set_material('cv1', get_arg_material(1))
		set_material('e_glow', lerp_materials(get_time('SECONDS')*.3, {0, 0, 0, 1, 0, 0, 0, 1, 1, 2, 2.5 }, {0, 0, 0, 1, 0, 0, 0, 1, 1.5, 2.5, 2.5 }))

		if lod > 2 then
			local reg = get_label()
			texture('models/ships/constrictor/washed.png',v(.5,.5,0),v(.5,0,0),v(0,0,.5))
			use_material('text')
			zbias(5,v(11.667,0,4), v(.85,1,0))
			text(reg,v(11.667,0,4), v(.85,1,0), v(0,0,-1),2, {center = true})
			zbias(5,v(-11.667,0,4.3), v(-.85,1,0))
			text(reg,v(-11.667,0,4.3), v(-.85,1,0), v(0,0,1),2, {center = true})
			zbias(0)
		end

		if lod > 1 then
			local M_T = v(0,1.5,17.2)
			local R_T = v(10.9,-1.7,-11.3)

			local RF_T = v(13.5,-1.5,-7)
			local RR_T = v(13.5,-1.5,7.5)
			local LF_T = v(-13.5,-1.5,-7)
			local LR_T = v(-13.5,-1.5,7.5)

			local TF_T = v(6,4.3,-3)
			local TR_T = v(6,4.3,12)
			local BF_T = v(8,-2.3,-10)
			local BR_T = v(8,-2.3,12)

			thruster(M_T,v(0,0,1),20,true)
			xref_thruster(R_T,v(0,0,-1),5,true)
			thruster(RF_T,v(1,0,0),5)
			thruster(RR_T,v(1,0,0),5)
			thruster(LF_T,v(-1,0,0),5)
			thruster(LR_T,v(-1,0,0),5)
			xref_thruster(TF_T,v(0,1,0),5)
			xref_thruster(TR_T,v(0,1,0),5)
			xref_thruster(BF_T,v(0,-1,0),5)
			xref_thruster(BR_T,v(0,-1,0),5)
		end
	end
})
