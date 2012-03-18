-- Wave manned heavy hypersonic fighter
-- Design inspirations from blackswift project and some decalling inspired by f-22 and a-10 aircraft

-- Large wave compresssion riding aircraft for edge of space operations engaging lower altitude and near orbit tiers, loses large 3d thrust vectoring of space fighters but gains from massive position change ability
-- (depending on speed and altitude and atmosphere) and extended high speed runs in atmosphere from low drag shape.
-- ++ Large g ability at hypersonic speeds ++ High speed in atmosphere runs due to low drag, can outmatch space designed fighters in atmosphere which are bogged due to thermal and friction factors
-- ++ operates at atmosphere boundary for ability against space and atmospheric threats
-- -- Not a space fighter, without atmosphere it is vulnerable -- Reduced sensor area due to sleek design, -- Ability dependent on pilot skill to use atmospheric lift to best ability

-- scale is size of a "heavy fighter" (my definition is not cockpit only) though it is very streamlined and loses any cargo carrying capability.
-- Crew cabin is between the engines (wear earmuffs!) with escape hatch ontop, it is quite a big fighter compared to a person 1.8m tall so could carry perhaps 2 in close proximity.
-- cabin is cramped, maybe like a stepped apache layout with 2 members and windows replaced by screens (no windows sry), 2 small passages either side merge to one to rear
-- 30m by 30m footprint.
-- made in wings3d (z front, y up, x left) textured in gimp (textures flipped vertically in game), exported to blender via 3ds then to game via obj due to wings 3d obj texture coords export mess up,
-- minimal triangles, no nonplanar 4 vertice surfaces, close to complete edge looping
-- gear attempt loads seperate gear objects first

define_model('1bogey', {
	info = {
		lod_pixels = { 5, 15, 30, 0 },
		bounding_radius = 7,
		materials={'leg','tyre'}
	},
	static = function(lod)
		set_material('leg', .02,.02,.02,1, 0,0,0,1, 0,0,0)
		set_material('tyre', .01,.01,.01,1, 0,0,0, 1, 0,0,0)
		use_material('leg')
		local v6 = v(0, 0, 0)
		local v7 = v(0, 6, 0)
		local v8 = v(0, 10, 0)
		local divs = lod*4
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		use_material('tyre')
		xref_cylinder(divs, v(.5,10,0), v(1,10,0), v(0,0,1), 1.0)
	end
})

define_model('3bogey', {
	info = {
		lod_pixels = { 5, 15, 30, 0 },
		bounding_radius = 8,
		materials = {'leg', 'tyre'}
	},
	static = function(lod)
		set_material('leg', .02,.02,.02,1, 0,0,0,1, 0,0,0)
		set_material('tyre', .01,.01,.01,1, 0,0,0, 1, 0,0,0)
		local v6 = v(0,0,0)
		local v7 = v(0,6,0)
		local v8 = v(0,10,0)
		-- crossbar
		local v13 = v(0, 10, 1.5)
		local v14 = v(0, 10, -2.0)
		local divs = 4*lod
		use_material('leg')
		cylinder(divs, v6, v8, v(0,0,1), .4)
		cylinder(divs, v7, v8, v(0,0,1), .5)
		cylinder(4, v13, v14, v(1,0,0), .5)
		use_material('tyre')
		xref_cylinder(divs, v(.5, 10, 1.7), v(1, 10, 1.7), v(0,0,1), 0.9)
		xref_cylinder(divs, v(.5, 10, -0.3), v(1, 10, -0.3), v(0,0,1), 0.9)
		xref_cylinder(divs, v(.5, 10, -2.5), v(1, 10, -2.5), v(0,0,1), 0.9)
	end
})

define_model('gearfront', {
	info = {
		scale = 1,
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 7,
		materials={'wave'}
	},
	static = function(lod)
		if lod > 1 then
			set_material('wave', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
			use_material('wave')
			texture('wave.png')
			load_obj('gearf.obj')
			texture(nil)
		end
	end
})

define_model('gearleft', {
	info = {
		scale = 1,
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 7,
		materials={'wave'}
	},
	static = function(lod)
		if lod > 1 then
			set_material('wave', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
			use_material('wave')
			texture('wave.png')
			load_obj('gearl.obj')
			texture(nil)
		end
	end
})

define_model('gearright', {
	info = {
		scale = 1,
		lod_pixels = { 20, 50, 100, 0 },
		bounding_radius = 7,
		materials={'wave'}
	},
	static = function(lod)
		if lod > 1 then
			set_material('wave', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
			use_material('wave')
			texture('wave.png')
			load_obj('gearr.obj')
			texture(nil)
		end
	end
})

define_model('wave', {
	info = {
		scale = 1,
		-- collision mesh pops in at 3 pixels, dark material with a lot of specular to glow at distance, then lod 2 at 12 pixels, change to 20 gives a closer popping in of high res mesh, 10 almost entirely visually avoids it (tested @1080p)
		lod_pixels={3,12,50,0},
		bounding_radius = 31,
		materials = {'wave', 'distant', 'text', 'glow'},
		tags = {'ship'},
	},
	static = function(lod)
		-- material specifications; diffuse r,g,b, alpha trans, specular r,g,b, shinyness, environmental r,g,b
		set_material('distant',0.15,0.15,0.15,1,1,1,1,0,0.15,0.15,0.15)
		set_material('wave', 1,1,1, 1, 0.6,0.6,0.6,100,0,0,0)
		set_material('text', .6,.6,.6,1,.3,.3,.3,5)
		if lod == 1 then
			use_material('distant')
			load_obj('wavelod1.obj')
		elseif lod == 2 then
			use_material('wave')
			texture('wave.png')
			load_obj('wavelod2.obj')
			texture(nil)
		else
			use_material('wave')
			texture('wave.png')
			load_obj('wave.obj')
			use_material('glow')
			texture('glow.png')
			load_obj('glow.obj')
			texture(nil)
		end
	end,
	dynamic = function(lod)
		if lod > 1 then
			if lod > 2 then
				-- glowing parts thanks to s2odan
				-- had to export glowing.obj from blender without mtl file to get it to work
				set_material('glow',lerp_materials(get_time('SECONDS'),{0,0,0,.8,0,0,0,0,10,10,10},{0,0,0,.4,0,0,0,0,9,9,9}))
				-- text on ship L first
				-- first number id, then center, then normal, then vector direction
				set_material('text', .6,.6,.6,1,.3,.3,.3,5)
				use_material('text')

				local reg = get_label()
				text(reg, v(-4.9,-2.7,4.7), v(-0.2,-1,0), v(0,0,1), 1, {center = true})
				text(reg, v(4.9,-2.7,4.7), v(0.2,-1,0), v(0,0,-1), 1, {center = true})

				-- missiles
				-- missile bays L2 L1 R1 L2 1 inside 2 outside
				local L1 = v(-6.9, -0.8, 5.3)
				local L2 = v(-6.9, -1.3, 5.3)
				local R1 = v(6.9, -0.8, 5.3)
				local R2 = v(6.9, -1.3, 5.3)

				-- unguided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_unguided',L1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 2) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_unguided',R1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 3) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_unguided',L2,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 4) == 'MISSILE_UNGUIDED' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_unguided',R2,v(1,0,0),v(0,1,0),1)
				end

				-- guided missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_GUIDED' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_guided',L1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 2) == 'MISSILE_GUIDED' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_guided',R1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 3) == 'MISSILE_GUIDED' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_guided',L2,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 4) == 'MISSILE_GUIDED' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_guided',R2,v(1,0,0),v(0,1,0),1)
				end

				-- smart missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_SMART' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_smart',L1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 2) == 'MISSILE_SMART' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_smart',R1,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 3) == 'MISSILE_SMART' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_smart',L2,v(1,0,0),v(0,1,0),1)
				end

				if get_equipment('MISSILE', 4) == 'MISSILE_SMART' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_smart',R2,v(1,0,0),v(0,1,0),1)
				end

				-- naval missiles loading
				if get_equipment('MISSILE', 1) == 'MISSILE_NAVAL' then
					call_model('m_pod',L1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_naval',L1,v(1,0,0),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 2) == 'MISSILE_NAVAL' then
					call_model('m_pod',R1+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_naval',R1,v(1,0,0),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 3) == 'MISSILE_NAVAL' then
					call_model('m_pod',L2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_naval',L2,v(1,0,0),v(0,1,0),1)
				end
				if get_equipment('MISSILE', 4) == 'MISSILE_NAVAL' then
					call_model('m_pod',R2+v(0,.3,0),v(1,0,0),v(0,1,0),1)
					call_model('d_naval',R2,v(1,0,0),v(0,1,0),1)
				end
			end
			-- lights
			call_model('posl_red', v(-14.85,-3.05, 13.7), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_green', v(14.85,-3.05, 13.7), v(0,1,0), v(1,0,0), 1)
			call_model('posl_red', v(-14.85,-3.05, 12.6), v(0,1,0), v(-1,0,0), 1)
			call_model('posl_green', v(14.85,-3.05, 12.6), v(0,1,0), v(1,0,0), 1)

			call_model('coll_warn', v(-3.0,-3.0, 1.2), v(-1,0,0), v(0,-1,0), 1)
			call_model('coll_warn', v(3.0,-3.0, 1.2), v(-1,0,0), v(0,-1,0), 1)
			call_model('coll_warn', v(-3.0,-3.0, 7.3), v(-1,0,0), v(0,-1,0), 1)
			call_model('coll_warn', v(3.0,-3.0, 7.3), v(-1,0,0), v(0,-1,0), 1)

			-- Thrusters, x and z coords are reversed from my wings 3d file
			-- 6 back facing thrusters
			local back1 = v(-5.2, -1.7, 6.6)
			local back2 = v(-3.8, -1.8, 6.6)
			local back3 = v(-2.4, -1.9, 6.6)
			xref_thruster(back1, v(0,0,1), 9, true)
			xref_thruster(back2, v(0,0,1), 9, true)
			xref_thruster(back3, v(0,0,1), 9, true)

			-- 2 front facing thrusters
			local front1 = v(2.2, -0.9, -12.2)
			xref_thruster(front1, v(0,0,-1), 4, true)

			-- 4 down facing thrusters
			local downl1 = v(3.7, -1.1, -8.7)
			xref_thruster(downl1, v(0,-1,0), 3)
			local downl2 = v(3.7, 0.1, 13.0)
			xref_thruster(downl2, v(0,-1,0), 3)

			-- 4 up facing thrusters
			local upl1 = v(-3.7, 0.5, -8.7)
			xref_thruster(upl1, v(0,1,0), 3)
			local upl2 = v(-3.7, 0.5, 11.2)
			xref_thruster(upl2, v(0,1,0), 3)

			--  4 side facing thrusters
			local sidel1 = v(-5.8, -2.3, 2.6)
			local sidel2 = v(-5.8, -2.3, 7.3)
			thruster(sidel1, v(-1,0,0), 5)
			thruster(sidel2, v(-1,0,0), 5)
			local sider1 = v(5.8, -2.3, 2.6)
			local sider2 = v(5.8, -2.3, 7.3)
			thruster(sider1, v(1,0,0), 5)
			thruster(sider2, v(1,0,0), 5)
		end

		-- landing gear
		local wheel_state = get_animation_position('WHEEL_STATE')
		if wheel_state ~= 0 then
			-- sliding front well covers and front gear
			-- sliders (seasons 1-2) rules!
			local sliders1 = 2.3 * wheel_state
			local sliders2 = 0.4*math.pi*math.clamp(1.5*(wheel_state-0.4), 0, 1)

			call_model('gearfront', v(0,0,sliders1), v(1,0,0), v(0,1,0), 1)
			call_model('gearleft', v(sliders1,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('gearright', v(-sliders1,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('1bogey', v(0,-1.7+2.2*math.cos(sliders2),1.2), v(-1,0,0), v(0,-1,0), 0.3)
			call_model('3bogey', v(3.7,-1.7+2.2*math.cos(sliders2),4.5), v(-1,0,0), v(0,-1,0), 0.3)
			call_model('3bogey', v(-3.7,-1.7+2.2*math.cos(sliders2),4.5), v(-1,0,0), v(0,-1,0), 0.3)
		else
			call_model('gearfront', v(0,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('gearleft', v(0,0,0), v(1,0,0), v(0,1,0), 1)
			call_model('gearright', v(0,0,0), v(1,0,0), v(0,1,0), 1)
		end
	end
})
