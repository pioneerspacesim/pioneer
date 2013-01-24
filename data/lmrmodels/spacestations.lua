-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt


-- NOTE
-- info->ship_dock_anim function's last docking anim ship location will be
-- used to place the ship when docked.

define_model('spacestation_door', {
	info = {
			lod_pixels = { 10, 0},
			bounding_radius = 200.0,
			materials = {'stripes','walls'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		set_material('walls', .7,.7,.7,1)
		set_material('stripes', .9,.9,0,1)
		use_material('walls')
		if lod > 1 then
		texture('ships/4_eagles/tex8.png', v(.5,.5,0), v(.085,0,0), v(0,.111,0))
		end
		quad(a,b,c,d)
		quad(d,c,b,a)
		if lod > 1 then
			use_material('stripes')
			zbias(1, v(0,0,0), v(0,1,0))
			-- diagonal stripes on front
			quad(v(-10,0,-20), v(-30,0,-20), v(-70,0,20), v(-50,0,20))
			quad(v(30,0,-20), v(10,0,-20), v(-30,0,20), v(-10,0,20))
			quad(v(70,0,-20), v(50,0,-20), v(10,0,20), v(30,0,20))
			-- on back
			zbias(1, v(0,0,0), v(0,-1,0))
			quad(v(10,0,-20), v(30,0,-20), v(70,0,20), v(50,0,20))
			quad(v(-30,0,-20), v(-10,0,-20), v(30,0,20), v(10,0,20))
			quad(v(-70,0,-20), v(-50,0,-20), v(-10,0,20), v(-30,0,20))
			zbias(0)
		end
	end
})

-- final resting place of docked ship
define_model('spacestation_entry1_stage3', {
	info = {
			lod_pixels = { 50, 0 },
			bounding_radius = 300,
			materials = {'wall1','wall2','text','red_glow','green_glow'}
		},
	static = function(lod)
		-- mark as non-colliding (all >= 0x8000 is)
		geomflag(0x9000)
		local a = v(-100,200,50)
		local b = v(100,200,50)
		local c = v(100,200,-50)
		local d = v(-100,200,-50)
		local a2 = v(-100,0,50)
		local b2 = v(100,0,50)
		local c2 = v(100,0,-50)
		local d2 = v(-100,0,-50)

		set_material('wall1', .2,.2,.2,1)
		set_material('wall2', .8,0,.8,1)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		set_material('red_glow', 1,0,0,.8,1,0,0,5,1,.4,0)
		set_material('green_glow', 0,1,0,.8,0,1,0,5,0,1,0)
		--set interior lighting
		set_local_lighting(true)
		--define light
		set_light(1, 0.000009, v(0,50,0), v(1,0,0))
		set_light(2, 0.0000000001, v(-99,100,49), v(0,.1,.1))

		if lod > 1 then
		use_light(1)
		use_material('wall2')
		texture('ships/4_eagles/tex1e.png', v(.5,1,0), v(-.005,0,0), v(0,-.005,0)) --floor
		quad(a,b,b2,a2)
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --top
		quad(c,d,d2,c2)
		texture('ships/4_eagles/tex1.png', v(.5,.55,0), v(.01,0,0), v(0,0,-.9)) --back
		quad(d,c,b,a)
		texture('ships/4_eagles/tex1.png', v(.5,.55,0), v(0,0,.9), v(0.01,0,0)) --l/r
		xref_quad(b,c,c2,b2)
		geomflag(0x8020)
		invisible_tri(v(0,100,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
		--struts and girders---------------
			--use_material('wall1')
			texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,.005,0))
			tube(8, v(0,4,0), v(0,0.1,0), v(0,0,1), 96, 102)
			tube(8, v(0,23,0), v(0,27,0), v(0,0,1), 96, 102)
			tube(8, v(0,48,0), v(0,52,0), v(0,0,1), 96, 102)
			tube(8, v(0,73,0), v(0,77,0), v(0,0,1), 96, 102)
			tube(8, v(0,98,0), v(0,102,0), v(0,0,1), 96, 102)
			tube(8, v(0,123,0), v(0,127,0), v(0,0,1), 96, 102)
			tube(8, v(0,148,0), v(0,152,0), v(0,0,1), 96, 102)
			tube(8, v(0,173,0), v(0,177,0), v(0,0,1), 96, 102)
			tube(8, v(0,198,0), v(0,202,0), v(0,0,1), 96, 102)
			cuboid(v(-90,0.1,-50),v(180,4,4))
			cuboid(v(-90,23,-50),v(180,4,4))
			cuboid(v(-90,48,-50),v(180,4,4))
			cuboid(v(-90,73,-50),v(180,4,4))
			cuboid(v(-90,98,-50),v(180,4,4))
			cuboid(v(-90,123,-50),v(180,4,4))
			cuboid(v(-90,148,-50),v(180,4,4))
			cuboid(v(-90,173,-50),v(180,4,4))
			cuboid(v(-90,198,-50),v(180,4,4))
			texture(nil)
			--ceiling lights-----------
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,25,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,25,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,50,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,50,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,75,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,75,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,100,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,100,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,125,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,125,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,150,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,150,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(-50,175,-45)})
			billboard('smoke.png', 10.0, v(1,0,0), {v(50,175,-45)})
			--lamp housings
			use_material('text')
			cylinder(8,v(-50,25,-46),v(-50,25,-45.5),v(0,1,0),1)
			cylinder(8,v(50,25,-46),v(50,25,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,50,-46),v(-50,50,-45.5),v(0,1,0),1)
			cylinder(8,v(50,50,-46),v(50,50,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,75,-46),v(-50,75,-45.5),v(0,1,0),1)
			cylinder(8,v(50,75,-46),v(50,75,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,100,-46),v(-50,100,-45.5),v(0,1,0),1)
			cylinder(8,v(50,100,-46),v(50,100,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,125,-46),v(-50,125,-45.5),v(0,1,0),1)
			cylinder(8,v(50,125,-46),v(50,125,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,150,-46),v(-50,150,-45.5),v(0,1,0),1)
			cylinder(8,v(50,150,-46),v(50,150,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,175,-46),v(-50,175,-45.5),v(0,1,0),1)
			cylinder(8,v(50,175,-46),v(50,175,-45.5),v(0,1,0),1)
			--lamp lens
			use_material('red_glow')
			cylinder(8,v(-50,25,-45.5),v(-50,25,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,25,-45.5),v(50,25,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,50,-45.5),v(-50,50,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,50,-45.5),v(50,50,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,75,-45.5),v(-50,75,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,75,-45.5),v(50,75,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,100,-45.5),v(-50,100,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,100,-45.5),v(50,100,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,125,-45.5),v(-50,125,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,125,-45.5),v(50,125,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,150,-45.5),v(-50,150,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,150,-45.5),v(50,150,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,175,-45.5),v(-50,175,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,175,-45.5),v(50,175,-44.5),v(0,1,0),.75)
			--TVs
			--power leds
			use_material('green_glow')
			cuboid(v(37,197.85,22),v(1,1,.5))--rear
			cuboid(v(94.85,63,22),v(.5,1,.5))--left
			cuboid(v(-94.85,137,22),v(.5,1,.5))--right
			--screens
			use_material('wall1')
			texture('grav.png', v(.5,.5,0), v(.01,0,0), v(0,0,-1.25))
			cuboid(v(-42.5,198,-22.75),v(85,2,47))--rear
			texture('grav.png', v(.5,.5,0), v(0,0,1.25), v(.01,0,0))
			cuboid(v(95,57.5,-22.75),v(2,85,47))--left
			cuboid(v(-97,57.5,-22.75),v(2,85,47))--right
			end

	end,
	dynamic = function(lod)
		if lod > 1 then
			use_material('text')
			zbias(1, v(0,200,0), v(0,-1,0))
			-- starport name
			text(get_label(), v(86,200,-35), v(0,-1,0), v(-1,0,0), 5.0)
			-- docking bay number
			text("DOCKING BAY 1", v(-60,200,-35), v(0,-1,0), v(-1,0,0), 7.0, {center=true})
			zbias(0)
			--cargo back wall
			call_model('cargo', v(21,195,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(65,195,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(95,193,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-12,197,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-32,195,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-72,197,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-95,190,49), v(-1,0,0), v(0,0,-1), 4)
			--cargo left wall
			call_model('cargo', v(96,183,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(98,180,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,178,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(98,173,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,170,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(95,168,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,148,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,118,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(96,116,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(98,110,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(95,104,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(95,97,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,95,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(94,90,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(96,85,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(96,70,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(98,68,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(95,22,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(97,19,49), v(-1,0,0), v(0,0,-1), 4)
			--cargo right wall
			call_model('cargo', v(-90,196,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-92,182,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-95,171,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-98,168,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-97,147,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-91,121,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-95,119,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-96,115,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-94,112,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-90,107,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-95,97,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-93,94,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-97,90,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-92,87,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-90,79,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-96,72,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-98,69,49), v(-1,0,0), v(0,0,-1), 4)
			call_model('cargo', v(-94,21,49), v(-1,0,0), v(0,0,-1), 4)

			-- adverts
			call_model('station_splash',v(0,197.85,0),v(-1,0,0),v(0,0,-1),40)
			call_model('ad_sirius_0', v(-94.85,100,20), v(0,-1,0), v(0,0,-1), 40.0)
			call_model('ad_sirius_0', v(94.85,100,20), v(0,1,0), v(0,0,-1), 40.0)
			set_local_lighting(false)
		end
	end
})

define_model('spacestation_entry1_stage2', {
	info = {
			bounding_radius = 300,
			materials = {'wall1', 'wall2','text','orange_glow'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		local a2 = v(-100,-200,50)
		local b2 = v(100,-200,50)
		local c2 = v(100,-200,-50)
		local d2 = v(-100,-200,-50)
		set_material('wall1', .8,.8,.8,1)
		set_material('wall2', .8,.8,.5,1.0)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		set_material('orange_glow', 1,.5,0,.8,1,.5,0,5,1,.5,0)
		use_material('wall1')
		--set interior lighting
		set_local_lighting(true)
		set_light(1, 0.00001, v(0,-100,0), v(.5,.25,0))
		use_light(1)
		texture('ships/4_eagles/tex7.png', v(.5,.5,0), v(.01,0,0), v(0,.02,0)) --t/b
		--if lod > 1 then
		quad(a,b,b2,a2)
		quad(c,d,d2,c2)
		texture('ships/4_eagles/tex7.png', v(.5,.5,0), v(0,0,.033), v(1,0,0)) --back
		quad(a2,b2,c2,d2)
		use_material('wall2')
		texture('ships/4_eagles/tex7.png', v(.5,.5,0), v(.01,0,0), v(0,.02,0)) --sides
		xref_quad(b,c,c2,b2)
		--struts and girders---------------
			--use_material('wall1')
			texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,.005,0))
			tube(8, v(0,-.1,0), v(0,-4,0), v(0,0,1), 96, 102)
			tube(8, v(0,-23,0), v(0,-27,0), v(0,0,1), 96, 102)
			tube(8, v(0,-48,0), v(0,-52,0), v(0,0,1), 96, 102)
			tube(8, v(0,-73,0), v(0,-77,0), v(0,0,1), 96, 102)
			tube(8, v(0,-98,0), v(0,-102,0), v(0,0,1), 96, 102)
			tube(8, v(0,-123,0), v(0,-127,0), v(0,0,1), 96, 102)
			tube(8, v(0,-148,0), v(0,-152,0), v(0,0,1), 96, 102)
			tube(8, v(0,-173,0), v(0,-177,0), v(0,0,1), 96, 102)
			tube(8, v(0,-198,0), v(0,-202,0), v(0,0,1), 96, 102)
			--top
			cuboid(v(-90,-4.1,-50),v(180,4,4))
			cuboid(v(-90,-27,-50),v(180,4,4))
			cuboid(v(-90,-52,-50),v(180,4,4))
			cuboid(v(-90,-77,-50),v(180,4,4))
			cuboid(v(-90,-102,-50),v(180,4,4))
			cuboid(v(-90,-127,-50),v(180,4,4))
			cuboid(v(-90,-152,-50),v(180,4,4))
			cuboid(v(-90,-177,-50),v(180,4,4))
			cuboid(v(-90,-202,-50),v(180,4,4))
			--bottom
			cuboid(v(-90,-4.1,46),v(180,4,4))
			cuboid(v(-90,-27,46),v(180,4,4))
			cuboid(v(-90,-52,46),v(180,4,4))
			cuboid(v(-90,-77,46),v(180,4,4))
			cuboid(v(-90,-102,46),v(180,4,4))
			cuboid(v(-90,-127,46),v(180,4,4))
			cuboid(v(-90,-152,46),v(180,4,4))
			cuboid(v(-90,-177,46),v(180,4,4))
			cuboid(v(-90,-202,46),v(180,4,4))
			texture(nil)
			--ceiling lights-----------
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-25,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-25,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-50,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-50,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-75,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-75,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-100,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-100,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-125,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-125,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-150,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-150,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(-50,-175,-45)})
			billboard('smoke.png', 10.0, v(1,.5,0), {v(50,-175,-45)})
			--lamp housings
			use_material('text')
			cylinder(8,v(-50,-25,-46),v(-50,-25,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-25,-46),v(50,-25,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-50,-46),v(-50,-50,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-50,-46),v(50,-50,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-75,-46),v(-50,-75,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-75,-46),v(50,-75,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-100,-46),v(-50,-100,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-100,-46),v(50,-100,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-125,-46),v(-50,-125,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-125,-46),v(50,-125,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-150,-46),v(-50,-150,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-150,-46),v(50,-150,-45.5),v(0,1,0),1)
			cylinder(8,v(-50,-175,-46),v(-50,-175,-45.5),v(0,1,0),1)
			cylinder(8,v(50,-175,-46),v(50,-175,-45.5),v(0,1,0),1)
			--lamp lens
			use_material('orange_glow')
			cylinder(8,v(-50,-25,-45.5),v(-50,-25,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-25,-45.5),v(50,-25,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-50,-45.5),v(-50,-50,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-50,-45.5),v(50,-50,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-75,-45.5),v(-50,-75,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-75,-45.5),v(50,-75,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-100,-45.5),v(-50,-100,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-100,-45.5),v(50,-100,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-125,-45.5),v(-50,-125,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-125,-45.5),v(50,-125,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-150,-45.5),v(-50,-150,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-150,-45.5),v(50,-150,-44.5),v(0,1,0),.75)
			cylinder(8,v(-50,-175,-45.5),v(-50,-175,-44.5),v(0,1,0),.75)
			cylinder(8,v(50,-175,-45.5),v(50,-175,-44.5),v(0,1,0),.75)
			--end
		geomflag(0x8010)
		invisible_tri(v(0,-100,0), v(0,0,0), v(0,0,0))
		invisible_tri(v(0,0,0), v(-1,0,0), v(0,0,-1))
		geomflag(0)
	end,
	dynamic = function(lod)
--		material(.8,0,0, 0,0,0, 0, 0,0,0)
		local stage = get_animation_stage('DOCKING_BAY_1')
		local pos = get_animation_position('DOCKING_BAY_1')
		if stage == 3 or stage == 7 or stage == -1 or stage == -5 then
			-- good, door is opening
		elseif stage == 5 or stage == 9 or stage == -3 or stage == -7 then -- door is closing
			pos = 1.0 - pos
		elseif stage == 4 or stage == 8 or stage == -2 or stage == -6 then -- door is open
			pos = 1
		else -- door is closed
			pos = 0
		end
		door2 = vlerp(pos, v(0,0,0), v(0,0,-100))
		call_model('spacestation_door', door2, v(1,0,0), v(0,1,0), 1.0)
		if stage >= 7 or (stage >= -4 and stage <= -1) then
			call_model('spacestation_entry1_stage3', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end
	end
})

define_model('spacestation_entry1_stage1', {
	info = {
			lod_pixels = { 20, 0 },
			bounding_radius = 400,
			materials = {'wall1', 'green_glow', 'text'}
		},
	static = function(lod)
		local a = v(-100,0,50)
		local b = v(100,0,50)
		local c = v(100,0,-50)
		local d = v(-100,0,-50)
		local a2 = v(-100,-300,50)
		local b2 = v(100,-300,50)
		local c2 = v(100,-300,-50)
		local d2 = v(-100,-300,-50)
		set_material('green_glow', 0,1,0,.8,0,1,0,5,0,1,0)
		set_material('wall1', .5,.5,.5,1)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		use_material('wall1')
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(0,0,1), v(0,.005,0)) --l/rv(.5,.5,0), v(0,0,.6), v(0,.005,0))
		xref_quad(b,c,c2,b2)
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --bottom
		quad(a,b,b2,a2)
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0)) --top
		quad(c,d,d2,c2)
		--struts and girders---------------
			texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,.005,0))
			tube(8, v(0,-20.5,0), v(0,-24.5,0), v(0,0,1), 96, 102)
			tube(8, v(0,-48,0), v(0,-52,0), v(0,0,1), 96, 102)
			tube(8, v(0,-73,0), v(0,-77,0), v(0,0,1), 96, 102)
			tube(8, v(0,-98,0), v(0,-102,0), v(0,0,1), 96, 102)
			tube(8, v(0,-123,0), v(0,-127,0), v(0,0,1), 96, 102)
			tube(8, v(0,-148,0), v(0,-152,0), v(0,0,1), 96, 102)
			tube(8, v(0,-173,0), v(0,-177,0), v(0,0,1), 96, 102)
			tube(8, v(0,-198,0), v(0,-202,0), v(0,0,1), 96, 102)
			tube(8, v(0,-223,0), v(0,-227,0), v(0,0,1), 96, 102)
			tube(8, v(0,-248,0), v(0,-252,0), v(0,0,1), 96, 102)
			tube(8, v(0,-273,0), v(0,-277,0), v(0,0,1), 96, 102)
			tube(8, v(0,-298,0), v(0,-300,0), v(0,0,1), 96, 102)
			--top
			cuboid(v(-90,-24.5,-50),v(180,4,4))
			cuboid(v(-90,-52,-50),v(180,4,4))
			cuboid(v(-90,-77,-50),v(180,4,4))
			cuboid(v(-90,-102,-50),v(180,4,4))
			cuboid(v(-90,-127,-50),v(180,4,4))
			cuboid(v(-90,-152,-50),v(180,4,4))
			cuboid(v(-90,-177,-50),v(180,4,4))
			cuboid(v(-90,-202,-50),v(180,4,4))
			cuboid(v(-90,-227,-50),v(180,4,4))
			cuboid(v(-90,-252,-50),v(180,4,4))
			cuboid(v(-90,-277,-50),v(180,4,4))
			cuboid(v(-90,-300,-50),v(180,2,4))
			--bottom
			cuboid(v(-90,-24.5,46),v(180,4,4))
			cuboid(v(-90,-52,46),v(180,4,4))
			cuboid(v(-90,-77,46),v(180,4,4))
			cuboid(v(-90,-102,46),v(180,4,4))
			cuboid(v(-90,-127,46),v(180,4,4))
			cuboid(v(-90,-152,46),v(180,4,4))
			cuboid(v(-90,-177,46),v(180,4,4))
			cuboid(v(-90,-202,46),v(180,4,4))
			cuboid(v(-90,-227,46),v(180,4,4))
			cuboid(v(-90,-252,46),v(180,4,4))
			cuboid(v(-90,-277,46),v(180,4,4))
			cuboid(v(-90,-300,46),v(180,2,4))
			--TVs
			--power leds
			use_material('green_glow')
			cuboid(v(94.8,-138.25,22),v(.5,1,.5))--left
			--cuboid(v(94.85,63,22),v(.5,1,.5))--left
			cuboid(v(-94.8,-63.25,22),v(.5,1,.5))--right
			--cuboid(v(-94.85,137,22),v(.5,1,.5))--right
			--screens
			use_material('text')--wall1
			texture('grav.png', v(.5,.5,0), v(0,0,1.25), v(.01,0,0))
			cuboid(v(94.85,-142.5,-22.75),v(2,85,47))--left
			--cuboid(v(95,57.5,-22.75),v(2,85,47))--left
			use_material('text')
			cuboid(v(-96.85,-142.5,-22.75),v(2,85,47))--right
			texture(nil)

	end,
	dynamic = function(lod)
		-- adverts
		if lod > 1 then
			call_model('ad_cola_1', v(94.7,-100,20), v(0,1,0), v(0,0,-1), 40.0)--'ad_sirius_2'
			call_model('ad_pioneer_0', v(-94.7,-100,20), v(0,-1,0), v(0,0,-1), 40.0)

		end
	end
})

define_model('spacestation_entry1', {
	info = {
			bounding_radius = 200,
			materials = {'wall'}
		},
	static = function(lod)
		set_material('wall', .2,.2,.6,1)
		-- docking surface
		geomflag(0x10)
		invisible_tri(v(-100,-100,50), v(100,-100,50), v(100,-100,-50))
		invisible_tri(v(-100,-100,50), v(100,-100,-50), v(-100,-100,-50))
		geomflag(0)
	end,
	dynamic = function(lod)
		local stage = get_animation_stage('DOCKING_BAY_1')
		local pos = get_animation_position('DOCKING_BAY_1')
		if stage == 1 then
			-- open door at start of docking permission
			pos = math.min(pos*50, 1.0)
		elseif stage == -8 then
			-- open door
		elseif stage == 2 then
			-- close door
			pos = 1.0 - pos
		elseif stage == -9 then
			-- launch: close door after a little while
			pos = 1.0 - pos
		else
			pos = 0
		end
		local door1 = vlerp(pos, v(0,-20,0), v(0,-20,-100))
		call_model('spacestation_door', door1, v(1,0,0), v(0,1,0), 1.0)

		if (stage >= 1 and stage <= 5) or (stage >= -9 and stage <= -5) then
			call_model('spacestation_entry1_stage1', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		end

		call_model('spacestation_entry1_stage2', v(0,-300,0), v(1,0,0), v(0,1,0), 1.0)
	end
})

function simple_lift_docking_port(baynum, pos, lod)
	local bayid = 'DOCKING_BAY_' .. (baynum + 1)
	local stage = get_animation_stage(bayid)
	local spos = get_animation_position(bayid)
	local baypos = 0

	if stage == 3 then
		baypos = -75*spos
	elseif stage == -2 then
		baypos = -75*(1-spos)
	end
	if stage >= 4 or stage == -1 then
		if stage > 4 then
			spos = 1
		elseif stage == -1 then
			spos = 1-spos
		end
		use_material('body')
		baypos = -75
		cuboid(pos+v(-50,-10,-50), v(50*spos, 5, 100))
		cuboid(pos+v(50*(1.0-spos),-10,-50), v(50*spos, 5, 100))
		function arrows(p1, p2)
			quad(p1, p1+v(5,0,-5), p1+v(0,0,-10), p1+v(-10,0,0))
			quad(p1, p1+v(-10,0,0), p1+v(0,0,10), p1+v(5,0,5))
			quad(p2, p2+v(10,0,0), p2+v(0,0,-10), p2+v(-5,0,-5))
			quad(p2, p2+v(-5,0,5), p2+v(0,0,10), p2+v(10,0,0))
		end
		use_material('markings')
		zbias(1, pos, v(0,1,0))
		arrows(pos+v(25+50*(1.0-spos),-5, 0), pos+v(-75+50*spos, -5, 0))
		zbias(0)
	end
	use_material('lift_floor')
	geomflag(0x10 + baynum)
	quad(pos+v(-50,baypos,50), pos+v(50,baypos,50), pos+v(50,baypos,-50), pos+v(-50,baypos,-50))
	geomflag(0)
	--use_material('body')

	if lod > 2 then
		zbias(1, pos+v(0,baypos,0), v(0,1,0))
		use_material('text')
		text(0, pos+v(0,baypos,0), v(0,1,0), v(1,0,0), 2.0, {center=true})
		zbias(0)
		text(baynum+1, pos+v(0,baypos,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
		zbias(0)

		use_material('inside')
		texture('lmrmodels/stationwall.png', v(.5,.9,0),v(0,0,1.1),v(0,-.009,0))
		xref_quad(pos+v(50,0,50), pos+v(50,0,-50), pos+v(50,-75,-50), pos+v(50,-75,50))

		texture('lmrmodels/stationwall.png', v(.5,.9,0),v(.01,0,0),v(0,-.009,0))
		quad(pos+v(-50,-75,-50), pos+v(50,-75,-50), pos+v(50,0,-50), pos+v(-50,0,-50))
		quad(pos+v(50,-75,50), pos+v(-50,-75,50), pos+v(-50,0,50), pos+v(50,0,50))
		texture(nil)

		if (math.fmod(get_time('SECONDS'), 2) > 1) then
			local color
			if stage > 1 or stage < 0 then
				color = v(1,0,0) -- red
			elseif stage == 1 then
				color = v(0,1,0) -- green
			else
				color = v(1,0.5,0) -- orange
			end
			billboard('smoke.png', 50, color, { pos+v(-50,1,50), pos+v(50,1,50), pos+v(-50,1,-50), pos+v(50,1,-50) })
		end
	end
end

define_model('nice_spacestation', {
	info = {
			bounding_radius=700.0,
			materials = {'text', 'body', 'green_lens'},
			tags = {'orbital_station'},
			lod_pixels = { 50, 0 },
		},
	static = function(lod)
		-- front
		-- f7   f3    f6
		--
		--    a b
		-- f4 d c   f2
		--
		-- f8  f1    f5
		local a = v(-100,400,50)
		local b = v(100,400,50)
		local c = v(100,400,-50)
		local d = v(-100,400,-50)

		local f1 = v(0,400,-400)
		local f2 = v(400,400,0)
		local f3 = v(0,400,400)
		local f4 = v(-400,400,0)

		local f5 = v(400,0,-400)
		local f6 = v(400,0,400)
		local f7 = v(-400,0,400)
		local f8 = v(-400,0,-400)

		-- back face points
		local f1b = v(0,-400,-400)
		local f2b = v(400,-400,0)
		local f3b = v(0,-400,400)
		local f4b = v(-400,-400,0)
		set_material('text', 0,0,0,1,0.3,0.3,0.3,5)
		set_material('green_lens',0,1,0,.9,0,0,1,1)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		--front face
		--door frame outside
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		cuboid(v(-110,380.1,-59.5),v(220,25,10))--top
		cuboid(v(-110,380.1,49.5),v(220,25,10))--bottom
		cuboid(v(-110,380.1,-50),v(10.5,25,100))--left
		cuboid(v(99.5,380.1,-50),v(10.5,25,100))--right
		--door frame inside
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0))
		cuboid(v(-100,380.1,-49.5),v(200,24,.5))--top
		cuboid(v(-100,380.1,49),v(200,24,.5))--bottom
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(0,0,1), v(0,0.005,0))
		cuboid(v(-99.5,380.1,-50),v(.5,24,100))--left
		cuboid(v(99.4,380.1,-50),v(.5,24,100))--right
		texture(nil)
		--lights
		--lamp housings
		use_material('text')
		cylinder(8,v(-150,401,0),v(-150,401.5,0),v(0,0,1),5)
		cylinder(8,v(-175,401,0),v(-175,401.5,0),v(0,0,1),5)
		cylinder(8,v(-200,401,0),v(-200,401.5,0),v(0,0,1),5)
		cylinder(8,v(150,401,0),v(150,401.5,0),v(0,0,1),5)
		cylinder(8,v(175,401,0),v(175,401.5,0),v(0,0,1),5)
		cylinder(8,v(200,401,0),v(200,401.5,0),v(0,0,1),5)
		--lamp lens
		use_material('green_lens')
		cylinder(8,v(-150,401.5,0),v(-150,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(-175,401.5,0),v(-175,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(-200,401.5,0),v(-200,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(150,401.5,0),v(150,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(175,401.5,0),v(175,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(200,401.5,0),v(200,402.5,0),v(0,0,1),4.5)
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))--tex12.png
		use_material('body')
		tri(f1,d,c)
		xref_tri(f1,c,f2)
		xref_tri(f2,c,b)
		xref_tri(f2,b,f3)
		tri(f3,b,a)
		--pyramid sides
		xref_tri(f2,f3,f6)
		xref_tri(f5,f1,f2)
		xref_tri(f6,f3b,f2b)
		xref_tri(f5,f2b,f1b)
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(0,0,.9), v(0,.01,0))
		xref_quad(f2,f6,f2b,f5) -- sides
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		quad(f3,f7,f3b,f6) -- top
		quad(f5,f1b,f8,f1) -- bottom
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		quad(f1b,f2b,f3b,f4b) -- rear
		call_model('spacestation_entry1', v(0,400,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			local textpos = v(0,400,-80)
			use_material('text')
			zbias(1, textpos, v(0,1,0))
			text(get_label(), textpos, v(0,1,0), v(1,0,0), 11, {center=true})
			zbias(0)
			call_model('ad_acme_2', v(0,-400.1,20), v(-1,0,0), v(0,0,-1), 40.0)
		end
		if (math.fmod(get_time('SECONDS'), 2) > 1) then
			billboard('smoke.png', 50, v(0,1,0), { v(-150,405.5,0), v(-175,405.5,0), v(-200,405.5,0) })
		else
			billboard('smoke.png', 50, v(0,1,0), { v(150,405.5,0), v(175,405.5,0), v(200,405.5,0) })
		end
	end
})

define_model('hoop_spacestation', {
	info = {
			bounding_radius=2000.0,
			materials = {'text', 'body', 'green_lens'},
			tags = {'orbital_station'},
			lod_pixels = { 50, 0 },
		},
	static = function(lod)
		-- front
		-- f7   f3    f6
		--
		--    a b
		-- f4 d c   f2
		--
		-- f8  f1    f5
		local a = v(-100,400,50)
		local b = v(100,400,50)
		local c = v(100,400,-50)
		local d = v(-100,400,-50)

		local f1 = v(0,400,-400)
		local f2 = v(400,400,0)
		local f3 = v(0,400,400)
		local f4 = v(-400,400,0)

		local f5 = v(400,0,-400)
		local f6 = v(400,0,400)
		local f7 = v(-400,0,400)
		local f8 = v(-400,0,-400)

		-- back face points
		local f1b = v(0,-400,-400)
		local f2b = v(400,-400,0)
		local f3b = v(0,-400,400)
		local f4b = v(-400,-400,0)

		set_material('text', 1.0,0.5,1.0,1)
		set_material('green_lens',0,1,0,.9,0,0,1,1)
		set_material('body', .5,.5,.5,1)
		use_material('body')
		--front face
		--door frame outside
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		cuboid(v(-110,380.1,-59.5),v(220,25,10))--top
		cuboid(v(-110,380.1,49.5),v(220,25,10))--bottom
		cuboid(v(-110,380.1,-50),v(10.5,25,100))--left
		cuboid(v(99.5,380.1,-50),v(10.5,25,100))--right
		--door frame inside
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(.005,0,0), v(0,.005,0))
		cuboid(v(-100,380.1,-49.5),v(200,24,.5))--top
		cuboid(v(-100,380.1,49),v(200,24,.5))--bottom
		texture('ships/4_eagles/tex2.png', v(.5,.5,0), v(0,0,1), v(0,0.005,0))
		cuboid(v(-99.5,380.1,-50),v(.5,24,100))--left
		cuboid(v(99.4,380.1,-50),v(.5,24,100))--right
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		tri(f1,d,c)
		xref_tri(f1,c,f2)
		xref_tri(f2,c,b)
		xref_tri(f2,b,f3)
		tri(f3,b,a)
		--pyramid sides
		xref_tri(f2,f3,f6)
		xref_tri(f5,f1,f2)
		xref_tri(f6,f3b,f2b)
		xref_tri(f5,f2b,f1b)
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(0,0,.9), v(0,.01,0))
		xref_quad(f2,f6,f2b,f5) -- sides
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,.01,0))
		quad(f3,f7,f3b,f6) -- top
		quad(f5,f1b,f8,f1) -- bottom
		texture('ships/4_eagles/tex12.png', v(.5,.5,0), v(.01,0,0), v(0,0,.9))
		quad(f1b,f2b,f3b,f4b) -- rear
		tube(16, v(0,200,0), v(0,-200,0), v(0,0,1), 1300.0, 1500.0)
		extrusion(v(0,0,-400), v(0,0,-1300), v(1,0,0), 100.0, v(-1,-1,0), v(1,-1,0), v(1,1,0), v(-1,1,0))
		extrusion(v(0,0,1300), v(0,0,400), v(1,0,0), 100.0, v(-1,-1,0), v(1,-1,0), v(1,1,0), v(-1,1,0))
		texture(nil)
		--lights
		--lamp housings
		use_material('text')
		cylinder(8,v(-150,401,0),v(-150,401.5,0),v(0,0,1),5)
		cylinder(8,v(-175,401,0),v(-175,401.5,0),v(0,0,1),5)
		cylinder(8,v(-200,401,0),v(-200,401.5,0),v(0,0,1),5)
		cylinder(8,v(150,401,0),v(150,401.5,0),v(0,0,1),5)
		cylinder(8,v(175,401,0),v(175,401.5,0),v(0,0,1),5)
		cylinder(8,v(200,401,0),v(200,401.5,0),v(0,0,1),5)
		--lamp lens
		use_material('green_lens')
		cylinder(8,v(-150,401.5,0),v(-150,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(-175,401.5,0),v(-175,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(-200,401.5,0),v(-200,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(150,401.5,0),v(150,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(175,401.5,0),v(175,402.5,0),v(0,0,1),4.5)
		cylinder(8,v(200,401.5,0),v(200,402.5,0),v(0,0,1),4.5)
		call_model('spacestation_entry1', v(0,400,0), v(1,0,0), v(0,1,0), 1.0)
	end,
	dynamic = function(lod)
		if lod > 1 then
			local textpos = v(0,400,-80)
			use_material('text')
			zbias(1, textpos, v(0,1,0))
			text(get_label(), textpos, v(0,1,0), v(1,0,0), 11.0, {center=true})
			zbias(0)
		end
		if (math.fmod(get_time('SECONDS'), 2) > 1) then
			billboard('smoke.png', 50, v(0,1,0), { v(-150,405.5,0), v(-175,405.5,0), v(-200,405.5,0) })
		else
			billboard('smoke.png', 50, v(0,1,0), { v(150,405.5,0), v(175,405.5,0), v(200,405.5,0) })
		end
	end
})
