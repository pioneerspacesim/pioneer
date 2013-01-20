-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--
-- ground_stations.lua
-- original model by Philbywhizz
-- detailing by Marcel
-- optimizations by potsmoke66
--

define_model('control_tower', {
	info = {
		lod_pixels = {1,80,160,0},
		bounding_radius = 250.0,
		materials = {'tower_base', 'body', 'top', 'dark_glass', 'screen', 'lens', 'lamp', 'lit_lamp'},
	},
	static = function(lod)
		local div0 = 2*lod
		local div1 = 8*lod
		set_material('body', .5, .5, .5, 1)
		use_material('body')
		if lod > 1 then
			texture('textures/seamless_texture_07.png', v(.5,.5,0),v(.02,0,0),v(0,0,-1))
		end
		lathe(div1, v(0,-50,0), v(0,21.5,0), v(0,0,1), {0,75,  1,23})--base cone
		if lod > 1 then
			texture('textures/tex12.png', v(.5,.5,0), v(.005,0,0), v(0,.02,0))
			ring(div1, v(0,150,0), v(0,200,0), v(0,0,1), 2.5)--center/socket
			ring(div1, v(0,201,0), v(0,202.3,0), v(0,0,1), 3.3)--socket ring
		end
		ring(8, v(0,25, 0), v(0,150,0), v(0,0,1), 10)--stem
--INTERIOR
	--control panels1
		if lod > 2 then
			lathe(div1, v(0,154.8,0), v(0,157.8,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(8,151,-4), v(8,151,4), v(0,1,0), 1)--e
			cylinder(4, v(-8,151,-4), v(-8,151,4), v(0,1,0), 1)--w
			cylinder(4, v(4,151,-8), v(-4,151,-8), v(0,1,0), 1)--n
			cylinder(4, v(4,151,8), v(-4,151,8), v(0,1,0), 1)--s
	--control panels2
			lathe(div1, v(0,163.6,0), v(0,166.1,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(12,159.8,4), v(4,159.8,12), v(0,1,0), 1)--se
			cylinder(4, v(-12,159.8,-4), v(-4,159.8,-12), v(0,1,0), 1)--nw
			cylinder(4, v(-12,159.8,4), v(-4,159.8,12), v(0,1,0), 1)--sw
			cylinder(4, v(4,159.8,-12), v(12,159.8,-4), v(0,1,0), 1)--ne
	--control panels3
			lathe(div1, v(0,172,0), v(0,174.5,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(8,168.1,-4), v(8,168.1,4), v(0,1,0), 1)--e
			cylinder(4, v(-8,168.1,-4), v(-8,168.1,4), v(0,1,0), 1)--w
			cylinder(4, v(4,168.1,-8), v(-4,168.1,-8), v(0,1,0), 1)--n
			cylinder(4, v(4,168.1,8), v(-4,168.1,8), v(0,1,0), 1)--s
			cylinder(4, v(13,168.1,5), v(5,168.1,13), v(0,1,0), 1)--se
			cylinder(4, v(-13,168.1,-5), v(-5,168.1,-13), v(0,1,0), 1)--nw
			cylinder(4, v(-13,168.1,5), v(-5,168.1,13), v(0,1,0), 1)--sw
			cylinder(4, v(5,168.1,-13), v(13,168.1,-5), v(0,1,0), 1)--ne
	--control panels4
			lathe(div1, v(0,180.5,0), v(0,182.7,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(8,176.5,-4), v(8,176.5,4), v(0,1,0), 1)--e
			cylinder(4, v(-8,176.5,-4), v(-8,176.5,4), v(0,1,0), 1)--w
			cylinder(4, v(4,176.5,-8), v(-4,176.5,-8), v(0,1,0), 1)--n
			cylinder(4, v(4,176.5,8), v(-4,176.5,8), v(0,1,0), 1)--s
			cylinder(4, v(13,176.5,5), v(5,176.5,13), v(0,1,0), 1)--se
			cylinder(4, v(-13,176.5,-5), v(-5,176.5,-13), v(0,1,0), 1)--nw
			cylinder(4, v(-13,176.5,5), v(-5,176.5,13), v(0,1,0), 1)--sw
			cylinder(4, v(5,176.5,-13), v(13,176.5,-5), v(0,1,0), 1)--ne
			cylinder(4, v(16,176.5,-4), v(16,176.5,4), v(0,1,0), 1)--e2
			cylinder(4, v(-16,176.5,-4), v(-16,176.5,4), v(0,1,0), 1)--w2
			cylinder(4, v(4,176.5,-16), v(-4,176.5,-16), v(0,1,0), 1)--n2
			cylinder(4, v(4,176.5,16), v(-4,176.5,16), v(0,1,0), 1)--s2
	--control panels5
			lathe(div1, v(0,188.5,0), v(0,191,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(8,184.7,-4), v(8,184.7,4), v(0,1,0), 1)--e
			cylinder(4, v(-8,184.7,-4), v(-8,184.7,4), v(0,1,0), 1)--w
			cylinder(4, v(4,184.7,-8), v(-4,184.7,-8), v(0,1,0), 1)--n
			cylinder(4, v(4,184.7,8), v(-4,184.7,8), v(0,1,0), 1)--s
			cylinder(4, v(13,184.7,5), v(5,184.7,13), v(0,1,0), 1)--se
			cylinder(4, v(-13,184.7,-5), v(-5,184.7,-13), v(0,1,0), 1)--nw
			cylinder(4, v(-13,184.7,5), v(-5,184.7,13), v(0,1,0), 1)--sw
			cylinder(4, v(5,184.7,-13), v(13,184.7,-5), v(0,1,0), 1)--ne
	--control panels6
			lathe(div1, v(0,195.6,0), v(0,197.6,0), v(0,0,1), {0,2.5, 1,6})
			cylinder(4, v(8,193,-4), v(8,193,4), v(0,1,0), 1)--e
			cylinder(4, v(-8,193,-4), v(-8,193,4), v(0,1,0), 1)--w
			cylinder(4, v(4,193,-8), v(-4,193,-8), v(0,1,0), 1)--n
			cylinder(4, v(4,193,8), v(-4,193,8), v(0,1,0), 1)--s
		end
	--floors, etc.
		if lod > 1 then
			texture('textures/bot5.png', v(.5,.5,0), v(.02,0,0), v(0,0,1))
			ring(div1, v(0,34,0), v(0,44,0), v(0,0,1), 10.25)--base roof stripes
			cylinder(div1, v(0,25,0), v(0,34,0), v(0,0,1), 12)--base roof ring
			cylinder(16, v(0,139,0), v(0,146,0), v(0,0,1), 10.15)--top stripes
			cylinder(div1, v(0,146,0), v(0,150,0), v(0,0,1), 11)--floor1
			cylinder(div1, v(0,157.8,0), v(0,158.8,0), v(0,0,1), 15.5)--floor2
			cylinder(div1, v(0,166.1,0), v(0,167.1,0), v(0,0,1), 18.8)--floor3
			cylinder(div1, v(0,174.5,0), v(0,175.5,0), v(0,0,1), 22.1)--floor4
			cylinder(div1, v(0,182.7,0), v(0,183.7,0), v(0,0,1), 18.8)--floor5
			cylinder(div1, v(0,191,0), v(0,192,0), v(0,0,1), 15.5)--floor6
			tapered_cylinder(div1, v(0,21.5,0), v(0,25,0), v(0,0,1), 23, 20)--base roof cap
			tapered_cylinder(div1, v(0,197.6,0), v(0,201.6,0), v(0,0,1), 11.5, 10.15)--roof
		end
	--screens
		if lod > 2 then
			texture(nil)
			set_material('screen', 0,0,1,1,0,0,0,5,0.7,0.7,1)
			use_material('screen')
		--floor1
			--south
			cylinder(4, v(2,151.4,6.9), v(2,151.4,7.1), v(0,1,0), .43)
			cylinder(4, v(0,151.4,6.9), v(0,151.4,7.1), v(0,1,0), .43)
			cylinder(4, v(-2,151.4,6.9), v(-2,151.4,7.1), v(0,1,0), .43)
			--north
			cylinder(4, v(2,151.4,-6.9), v(2,151.4,-7.1), v(0,1,0), .43)
			cylinder(4, v(0,151.4,-6.9), v(0,151.4,-7.1), v(0,1,0), .43)
			cylinder(4, v(-2,151.4,-6.9), v(-2,151.4,-7.1), v(0,1,0), .43)
			--east
			cylinder(4, v(7,151.4,-2), v(6.8,151.4,-2), v(0,1,0), .43)
			cylinder(4, v(7,151.4,0), v(6.8,151.4,0), v(0,1,0), .43)
			cylinder(4, v(7,151.4,2), v(6.8,151.4,2), v(0,1,0), .43)
			--west
			cylinder(4, v(-7,151.4,-2), v(-6.8,151.4,-2), v(0,1,0), .43)
			cylinder(4, v(-7,151.4,0), v(-6.8,151.4,0), v(0,1,0), .43)
			cylinder(4, v(-7,151.4,2), v(-6.8,151.4,2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,153.5,0), v(-2.7,153.5,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,153.5,-2.7), v(0,153.5,2.7), v(0,1,0), .75)
		--floor2
			--se
			cylinder(4, v(5,160.2,11), v(4.2,160.2,10.2), v(0,1,0), .43)
			cylinder(4, v(7,160.2,9), v(6.2,160.2,8.2), v(0,1,0), .43)
			cylinder(4, v(9,160.2,7), v(8.2,160.2,6.2), v(0,1,0), .43)
			cylinder(4, v(11,160.2,5), v(10.2,160.2,4.2), v(0,1,0), .43)
			--nw
			cylinder(4, v(-5,160.2,-11), v(-4.2,160.2,-10.2), v(0,1,0), .43)
			cylinder(4, v(-7,160.2,-9), v(-6.2,160.2,-8.2), v(0,1,0), .43)
			cylinder(4, v(-9,160.2,-7), v(-8.2,160.2,-6.2), v(0,1,0), .43)
			cylinder(4, v(-11,160.2,-5), v(-10.2,160.2,-4.2), v(0,1,0), .43)
			--sw
			cylinder(4, v(-5,160.2,11), v(-4.2,160.2,10.2), v(0,1,0), .43)
			cylinder(4, v(-7,160.2,9), v(-6.2,160.2,8.2), v(0,1,0), .43)
			cylinder(4, v(-9,160.2,7), v(-8.2,160.2,6.2), v(0,1,0), .43)
			cylinder(4, v(-11,160.2,5), v(-10.2,160.2,4.2), v(0,1,0), .43)
			--ne
			cylinder(4, v(5,160.2,-11), v(4.2,160.2,-10.2), v(0,1,0), .43)
			cylinder(4, v(7,160.2,-9), v(6.2,160.2,-8.2), v(0,1,0), .43)
			cylinder(4, v(9,160.2,-7), v(8.2,160.2,-6.2), v(0,1,0), .43)
			cylinder(4, v(11,160.2,-5), v(10.2,160.2,-4.2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,162,0), v(-2.7,162,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,162,-2.7), v(0,162,2.7), v(0,1,0), .75)
		--floor3
			--south
			cylinder(4, v(2,168.3,6.9), v(2,168.3,7.1), v(0,1,0), .43)
			cylinder(4, v(0,168.3,6.9), v(0,168.3,7.1), v(0,1,0), .43)
			cylinder(4, v(-2,168.3,6.9), v(-2,168.3,7.1), v(0,1,0), .43)
			--north
			cylinder(4, v(2,168.3,-6.9), v(2,168.3,-7.1), v(0,1,0), .43)
			cylinder(4, v(0,168.3,-6.9), v(0,168.3,-7.1), v(0,1,0), .43)
			cylinder(4, v(-2,168.3,-6.9), v(-2,168.3,-7.1), v(0,1,0), .43)
			--east
			cylinder(4, v(7,168.3,-2), v(6.8,168.3,-2), v(0,1,0), .43)
			cylinder(4, v(7,168.3,0), v(6.8,168.3,0), v(0,1,0), .43)
			cylinder(4, v(7,168.3,2), v(6.8,168.3,2), v(0,1,0), .43)
			--west
			cylinder(4, v(-7,168.3,-2), v(-6.8,168.3,-2), v(0,1,0), .43)
			cylinder(4, v(-7,168.3,0), v(-6.8,168.3,0), v(0,1,0), .43)
			cylinder(4, v(-7,168.3,2), v(-6.8,168.3,2), v(0,1,0), .43)
			--se
			cylinder(4, v(6,168.3,12), v(5.2,168.3,11.2), v(0,1,0), .43)
			cylinder(4, v(8,168.3,10), v(7.2,168.3,9.2), v(0,1,0), .43)
			cylinder(4, v(10,168.3,8), v(9.2,168.3,7.2), v(0,1,0), .43)
			cylinder(4, v(12,168.3,6), v(11.2,168.3,5.2), v(0,1,0), .43)
			--nw
			cylinder(4, v(-6,168.3,-12), v(-5.2,168.3,-11.2), v(0,1,0), .43)
			cylinder(4, v(-8,168.3,-10), v(-7.2,168.3,-9.2), v(0,1,0), .43)
			cylinder(4, v(-10,168.3,-8), v(-9.2,168.3,-7.2), v(0,1,0), .43)
			cylinder(4, v(-12,168.3,-6), v(-11.2,168.3,-5.2), v(0,1,0), .43)
			--sw
			cylinder(4, v(-6,168.3,12), v(-5.2,168.3,11.2), v(0,1,0), .43)
			cylinder(4, v(-8,168.3,10), v(-7.2,168.3,9.2), v(0,1,0), .43)
			cylinder(4, v(-10,168.3,8), v(-9.2,168.3,7.2), v(0,1,0), .43)
			cylinder(4, v(-12,168.3,6), v(-11.2,168.3,5.2), v(0,1,0), .43)
			--ne
			cylinder(4, v(6,168.3,-12), v(5.2,168.3,-11.2), v(0,1,0), .43)
			cylinder(4, v(8,168.3,-10), v(7.2,168.3,-9.2), v(0,1,0), .43)
			cylinder(4, v(10,168.3,-8), v(9.2,168.3,-7.2), v(0,1,0), .43)
			cylinder(4, v(12,168.3,-6), v(11.2,168.3,-5.2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,170.6,0), v(-2.7,170.6,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,170.6,-2.7), v(0,170.6,2.7), v(0,1,0), .75)
		--floor4
			--south
			cylinder(4, v(2,176.7,6.9), v(2,176.7,7.1), v(0,1,0), .43)
			cylinder(4, v(0,176.7,6.9), v(0,176.7,7.1), v(0,1,0), .43)
			cylinder(4, v(-2,176.7,6.9), v(-2,176.7,7.1), v(0,1,0), .43)
			--north
			cylinder(4, v(2,176.7,-6.9), v(2,176.7,-7.1), v(0,1,0), .43)
			cylinder(4, v(0,176.7,-6.9), v(0,176.7,-7.1), v(0,1,0), .43)
			cylinder(4, v(-2,176.7,-6.9), v(-2,176.7,-7.1), v(0,1,0), .43)
			--east
			cylinder(4, v(7,176.7,-2), v(6.8,176.7,-2), v(0,1,0), .43)
			cylinder(4, v(7,176.7,0), v(6.8,176.7,0), v(0,1,0), .43)
			cylinder(4, v(7,176.7,2), v(6.8,176.7,2), v(0,1,0), .43)
			--west
			cylinder(4, v(-7,176.7,-2), v(-6.8,176.7,-2), v(0,1,0), .43)
			cylinder(4, v(-7,176.7,0), v(-6.8,176.7,0), v(0,1,0), .43)
			cylinder(4, v(-7,176.7,2), v(-6.8,176.7,2), v(0,1,0), .43)
			--se
			cylinder(4, v(6,176.7,12), v(5.2,176.7,11.2), v(0,1,0), .43)
			cylinder(4, v(8,176.7,10), v(7.2,176.7,9.2), v(0,1,0), .43)
			cylinder(4, v(10,176.7,8), v(9.2,176.7,7.2), v(0,1,0), .43)
			cylinder(4, v(12,176.7,6), v(11.2,176.7,5.2), v(0,1,0), .43)
			--nw
			cylinder(4, v(-6,176.7,-12), v(-5.2,176.7,-11.2), v(0,1,0), .43)
			cylinder(4, v(-8,176.7,-10), v(-7.2,176.7,-9.2), v(0,1,0), .43)
			cylinder(4, v(-10,176.7,-8), v(-9.2,176.7,-7.2), v(0,1,0), .43)
			cylinder(4, v(-12,176.7,-6), v(-11.2,176.7,-5.2), v(0,1,0), .43)
			--sw
			cylinder(4, v(-6,176.7,12), v(-5.2,176.7,11.2), v(0,1,0), .43)
			cylinder(4, v(-8,176.7,10), v(-7.2,176.7,9.2), v(0,1,0), .43)
			cylinder(4, v(-10,176.7,8), v(-9.2,176.7,7.2), v(0,1,0), .43)
			cylinder(4, v(-12,176.7,6), v(-11.2,176.7,5.2), v(0,1,0), .43)
			--ne
			cylinder(4, v(6,176.7,-12), v(5.2,176.7,-11.2), v(0,1,0), .43)
			cylinder(4, v(8,176.7,-10), v(7.2,176.7,-9.2), v(0,1,0), .43)
			cylinder(4, v(10,176.7,-8), v(9.2,176.7,-7.2), v(0,1,0), .43)
			cylinder(4, v(12,176.7,-6), v(11.2,176.7,-5.2), v(0,1,0), .43)
			--s2
			cylinder(4, v(2,176.7,16), v(2,176.7,14.8), v(0,1,0), .43)
			cylinder(4, v(0,176.7,16), v(0,176.7,14.8), v(0,1,0), .43)
			cylinder(4, v(-2,176.7,16), v(-2,176.7,14.8), v(0,1,0), .43)
			--n2
			cylinder(4, v(2,176.7,-16), v(2,176.7,-14.8), v(0,1,0), .43)
			cylinder(4, v(0,176.7,-16), v(0,176.7,-14.8), v(0,1,0), .43)
			cylinder(4, v(-2,176.7,-16), v(-2,176.7,-14.8), v(0,1,0), .43)
			--e2
			cylinder(4, v(16,176.7,-2), v(14.8,176.7,-2), v(0,1,0), .43)
			cylinder(4, v(16,176.7,0), v(14.8,176.7,0), v(0,1,0), .43)
			cylinder(4, v(16,176.7,2), v(14.8,176.7,2), v(0,1,0), .43)
			--w2
			cylinder(4, v(-16,176.7,-2), v(-14.8,176.7,-2), v(0,1,0), .43)
			cylinder(4, v(-16,176.7,0), v(-14.8,176.7,0), v(0,1,0), .43)
			cylinder(4, v(-16,176.7,2), v(-14.8,176.7,2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,179,0), v(-2.7,179,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,179,-2.7), v(0,179,2.7), v(0,1,0), .75)
		--floor5
			--south
			cylinder(4, v(2,184.9,6.9), v(2,184.9,7.1), v(0,1,0), .43)
			cylinder(4, v(0,184.9,6.9), v(0,184.9,7.1), v(0,1,0), .43)
			cylinder(4, v(-2,184.9,6.9), v(-2,184.9,7.1), v(0,1,0), .43)
			--north
			cylinder(4, v(2,184.9,-6.9), v(2,184.9,-7.1), v(0,1,0), .43)
			cylinder(4, v(0,184.9,-6.9), v(0,184.9,-7.1), v(0,1,0), .43)
			cylinder(4, v(-2,184.9,-6.9), v(-2,184.9,-7.1), v(0,1,0), .43)
			--east
			cylinder(4, v(7,184.9,-2), v(6.8,184.9,-2), v(0,1,0), .43)
			cylinder(4, v(7,184.9,0), v(6.8,184.9,0), v(0,1,0), .43)
			cylinder(4, v(7,184.9,2), v(6.8,184.9,2), v(0,1,0), .43)
			--west
			cylinder(4, v(-7,184.9,-2), v(-6.8,184.9,-2), v(0,1,0), .43)
			cylinder(4, v(-7,184.9,0), v(-6.8,184.9,0), v(0,1,0), .43)
			cylinder(4, v(-7,184.9,2), v(-6.8,184.9,2), v(0,1,0), .43)
			--se
			cylinder(4, v(6,184.9,12), v(5.2,184.9,11.2), v(0,1,0), .43)
			cylinder(4, v(8,184.9,10), v(7.2,184.9,9.2), v(0,1,0), .43)
			cylinder(4, v(10,184.9,8), v(9.2,184.9,7.2), v(0,1,0), .43)
			cylinder(4, v(12,184.9,6), v(11.2,184.9,5.2), v(0,1,0), .43)
			--nw
			cylinder(4, v(-6,184.9,-12), v(-5.2,184.9,-11.2), v(0,1,0), .43)
			cylinder(4, v(-8,184.9,-10), v(-7.2,184.9,-9.2), v(0,1,0), .43)
			cylinder(4, v(-10,184.9,-8), v(-9.2,184.9,-7.2), v(0,1,0), .43)
			cylinder(4, v(-12,184.9,-6), v(-11.2,184.9,-5.2), v(0,1,0), .43)
			--sw
			cylinder(4, v(-6,184.9,12), v(-5.2,184.9,11.2), v(0,1,0), .43)
			cylinder(4, v(-8,184.9,10), v(-7.2,184.9,9.2), v(0,1,0), .43)
			cylinder(4, v(-10,184.9,8), v(-9.2,184.9,7.2), v(0,1,0), .43)
			cylinder(4, v(-12,184.9,6), v(-11.2,184.9,5.2), v(0,1,0), .43)
			--ne
			cylinder(4, v(6,184.9,-12), v(5.2,184.9,-11.2), v(0,1,0), .43)
			cylinder(4, v(8,184.9,-10), v(7.2,184.9,-9.2), v(0,1,0), .43)
			cylinder(4, v(10,184.9,-8), v(9.2,184.9,-7.2), v(0,1,0), .43)
			cylinder(4, v(12,184.9,-6), v(11.2,184.9,-5.2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,187.2,0), v(-2.7,187.2,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,187.2,-2.7), v(0,187.2,2.7), v(0,1,0), .75)
		--floor6
			--south
			cylinder(4, v(2,193.2,6.9), v(2,193.2,7.1), v(0,1,0), .43)
			cylinder(4, v(0,193.2,6.9), v(0,193.2,7.1), v(0,1,0), .43)
			cylinder(4, v(-2,193.2,6.9), v(-2,193.2,7.1), v(0,1,0), .43)
			--north
			cylinder(4, v(2,193.2,-6.9), v(2,193.2,-7.1), v(0,1,0), .43)
			cylinder(4, v(0,193.2,-6.9), v(0,193.2,-7.1), v(0,1,0), .43)
			cylinder(4, v(-2,193.2,-6.9), v(-2,193.2,-7.1), v(0,1,0), .43)
			--east
			cylinder(4, v(7,193.2,-2), v(6.8,193.2,-2), v(0,1,0), .43)
			cylinder(4, v(7,193.2,0), v(6.8,193.2,0), v(0,1,0), .43)
			cylinder(4, v(7,193.2,2), v(6.8,193.2,2), v(0,1,0), .43)
			--west
			cylinder(4, v(-7,193.2,-2), v(-6.8,193.2,-2), v(0,1,0), .43)
			cylinder(4, v(-7,193.2,0), v(-6.8,193.2,0), v(0,1,0), .43)
			cylinder(4, v(-7,193.2,2), v(-6.8,193.2,2), v(0,1,0), .43)
			--center screens e/w
			cylinder(4, v(2.7,194.5,0), v(-2.7,194.5,0), v(0,1,0), .75)
			-- s/n
			cylinder(4, v(0,194.5,-2.7), v(0,194.5,2.7), v(0,1,0), .75)
		end
		--glass coverage
		set_material('dark_glass',.01,.01,.1,.6,.1,.1,.1,100)
		use_material('dark_glass')
		lathe(div1, v(0,150,0), v(0,200,0), v(0,0,1), {0,10,  .5,20,  1,10})
		end,
	dynamic = function(lod)
		if lod > 1 then

			local lightphase = math.fmod(get_time('SECONDS')+0.620486, 1)
			billboard('smoke.png', 50, lightphase > .5 and v(1,0,0) or v(0,1,0), { v(0, 205.3, 0) })
		-- tower lens
			set_material('lens', .2,.2,0,.35,.2,.2,0,100,.2,.2,0)
			use_material('lens')
			tapered_cylinder(8, v(0,202.3,0), v(0,205.3,0), v(0,0,1), 3.2, 2.25)
		end
	end
})

function createLandingPadStatic(padNum, position, lod)
	-- padNum: The landing pad number (zero based)
	-- position: vector of the landing pad v(0,0,0) is where the ship lands

	local padId = 'DOCKING_BAY_' .. (padNum + 1)
	local div0 = 2*lod
	local div1 = 4*lod
	-- draw landing pad
	set_material('pad', .7, .7, .7, 1)
	use_material('pad')
	geomflag(0x10 + padNum)
	if lod > 1 then
		texture('textures/seamless_texture_07.png', v(.5,.5,0),v(0,0.125,0),v(0,0,1.25))
	end
	cylinder(10, position + v(0,0,0), position + v(0,-.4,0), v(0,0,1), 50)
	if lod > 1 then
		cylinder(10, position + v(0,-2,0), position + v(0,-3,0), v(0,0,1), 52.15)
		cylinder(10, position + v(0,-4.5,0), position + v(0,-5.5,0), v(0,0,1), 53.15)
		cylinder(10, position + v(0,-7,0), position + v(0,-8,0), v(0,0,1), 54.15)
	end
	geomflag(0)
	--windows
	--frames
	if lod > 1 then
		texture('textures/bot5.png', v(.5,.5,0), v(.02,0,0), v(0,0,1))
		cuboid(position + v(-51.1,-4,8), v(0,-.1,-16))
		--cuboid(position + v(-51.1,-4,7.9), v(0,-.1,-16.1))
		cuboid(position + v(51,-4,8), v(.1,-.1,-16))
		--cuboid(position + v(51,-4,7.9), v(0,-.1,-16.1))
		cuboid(position + v(-51.2,-3,8.1), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,6), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,3.1), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,1), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,8.1), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,6), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,3.1), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,1), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,-8), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,-5.9), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,-3), v(.2,-1.5,-.1))
		cuboid(position + v(-51.2,-3,-.9), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,-8), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,-5.9), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,-3), v(.2,-1.5,-.1))
		cuboid(position + v(51,-3,-.9), v(.2,-1.5,-.1))
	--interior
		set_material('screen', 0,0,1,1,0,0,0,5,0.7,0.7,1)
		use_material('screen')
		texture('textures/tex12.png', v(.5,.5,0), v(0,0,.3), v(.3,0,0))
		quad(position + v(-47,-4.49,16), position + v(-47,-3.01,16), position + v(-47,-3.01,-16), position + v(-47,-4.49,-16))--w wall
		quad(position + v(47,-4.49,16), position + v(47,-4.49,-16), position + v(47,-3.01,-16), position + v(47,-3.01,16))--e wall
		quad(position + v(-51.1,-4.49,16), position + v(51.1,-4.49,16), position + v(51.1,-4.49,-16), position + v(-51.1,-4.49,-16))--floor
		quad(position + v(-51.1,-3.01,-16), position + v(51.1,-3.01,-16), position + v(51.1,-3.01,16), position + v(-51.1,-3.01,16))--ceiling
		quad(position + v(-51.1,-4.49,-16), position + v(51.1,-4.49,-16), position + v(51.1,-3.01,-16), position + v(-51.1,-3.01,-16))--n wall
		quad(position + v(-51.1,-3.01,16), position + v(51.1,-3.01,16), position + v(51.1,-4.49,16), position + v(-51.1,-4.49,16))--s wall
	--windows
		set_material('lens', .2, .2, 0.2, .15, .2, .2, 0.2, 100, .2, .2, 0.2)
		use_material('lens')
		texture(nil)
		--west
		quad(position + v(-51.2,-4,8), position + v(-51.2,-3,8), position + v(-51.2,-3,7.1), position + v(-51.2,-4,7.1))
		quad(position + v(-51.2,-4,6.9), position + v(-51.2,-3,6.9), position + v(-51.2,-3,6), position + v(-51.2,-4,6))
		quad(position + v(-51.2,-4,3), position + v(-51.2,-3,3), position + v(-51.2,-3,2.1), position + v(-51.2,-4,2.1))
		quad(position + v(-51.2,-4,1.9), position + v(-51.2,-3,1.9), position + v(-51.2,-3,1), position + v(-51.2,-4,1))
		quad(position + v(-51.2,-4,-1), position + v(-51.2,-3,-1), position + v(-51.2,-3,-1.9), position + v(-51.2,-4,-1.9))
		quad(position + v(-51.2,-4,-2.1), position + v(-51.2,-3,-2.1), position + v(-51.2,-3,-3), position + v(-51.2,-4,-3))
		quad(position + v(-51.2,-4,-6), position + v(-51.2,-3,-6), position + v(-51.2,-3,-6.9), position + v(-51.2,-4,-6.9))
		quad(position + v(-51.2,-4,-7.1), position + v(-51.2,-3,-7.1), position + v(-51.2,-3,-8), position + v(-51.2,-4,-8))
		--east
		quad(position + v(51.2,-4,8), position + v(51.2,-4,7.1), position + v(51.2,-3,7.1), position + v(51.2,-3,8))
		quad(position + v(51.2,-4,6.9), position + v(51.2,-4,6), position + v(51.2,-3,6), position + v(51.2,-3,6.9))
		quad(position + v(51.2,-4,3), position + v(51.2,-4,2.1), position + v(51.2,-3,2.1), position + v(51.2,-3,3))
		quad(position + v(51.2,-4,1.9), position + v(51.2,-4,1), position + v(51.2,-3,1), position + v(51.2,-3,1.9))
		quad(position + v(51.2,-4,-1), position + v(51.2,-4,-1.9), position + v(51.2,-3,-1.9), position + v(51.2,-3,-1))
		quad(position + v(51.2,-4,-2.1), position + v(51.2,-4,-3), position + v(51.2,-3,-3), position + v(51.2,-3,-2.1))
		quad(position + v(51.2,-4,-6), position + v(51.2,-4,-6.9), position + v(51.2,-3,-6.9), position + v(51.2,-3,-6))
		quad(position + v(51.2,-4,-7.1), position + v(51.2,-4,-8), position + v(51.2,-3,-8), position + v(51.2,-3,-7.1))
	-- draw out the pad number
		texture('textures/seamless_texture_07.png', v(.5,.5,0),v(0,0.125,0),v(0,0,1.25))
		set_material('text', 1, 1, 1, 1)
		use_material('text')
		zbias(1, position + v(0,0,0), v(0,1,0))
		text(padNum+1, position + v(0,0,0), v(0,1,0), v(1,0,0), 20.0, {center=true})
		-- starport name
		-- note: actual starport name is added in the dynamic section where it can use get_label()
		text("ACME", position + v(0,0,35), v(0,1,0), v(1,0,0), 5, {center=true})
		text("Inc", position + v(0,0,39), v(0,1,0), v(1,0,0), 4, {center=true})
		-- elevator labels
		text("A", position + v(-44.4,0,8), v(0,1,0), v(0,0,-1), 4, {center=true})
		text("B", position + v(-44.4,0,-8), v(0,1,0), v(0,0,-1), 4, {center=true})
		text("C", position + v(44.4,0,-8), v(0,1,0), v(0,0,1), 4, {center=true})
		text("D", position + v(44.4,0,8), v(0,1,0), v(0,0,1), 4, {center=true})
		zbias(0)
	--pad body
		texture('textures/bot5.png', v(.5,.5,0), v(.02,0,0), v(0,0,1))
	end
	geomflag(0x10 + padNum)			-- landing surface must be at least 1m thick
	cylinder(10, position + v(0,-.5,0), position + v(0,-10.5,0), v(0,0,1), 51.15)
	geomflag(0)
	--pad pylon
	lathe(10, position + v(-0,-10.5,0), position + v(0,-30.5,0), v(0,0,1), {0.0,25, 1,10})
	if lod > 1 then
	--fence posts
		ring(div0, position + v(-49.5,0,16), position + v(-49.5,1.3,16), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,16), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-16), position + v(-49.5,1.3,-16), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-16), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,16), position + v(49.5,1.3,16), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,16), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-16), position + v(49.5,1.3,-16), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-16), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,14), position + v(-49.5,1.3,14), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,14), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-14), position + v(-49.5,1.3,-14), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-14), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,14), position + v(49.5,1.3,14), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,14), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-14), position + v(49.5,1.3,-14), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-14), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,12), position + v(-49.5,1.3,12), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,12), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-12), position + v(-49.5,1.3,-12), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-12), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,12), position + v(49.5,1.3,12), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,12), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-12), position + v(49.5,1.3,-12), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-12), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,10), position + v(-49.5,1.3,10), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,10), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-10), position + v(-49.5,1.3,-10), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-10), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,10), position + v(49.5,1.3,10), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,10), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-10), position + v(49.5,1.3,-10), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-10), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,8), position + v(-49.5,1.3,8), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,8), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-8), position + v(-49.5,1.3,-8), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-8), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,8), position + v(49.5,1.3,8), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,8), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-8), position + v(49.5,1.3,-8), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-8), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,6), position + v(-49.5,1.3,6), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,6), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-6), position + v(-49.5,1.3,-6), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-6), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,6), position + v(49.5,1.3,6), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,6), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-6), position + v(49.5,1.3,-6), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-6), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,4), position + v(-49.5,1.3,4), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,4), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-4), position + v(-49.5,1.3,-4), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-4), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,4), position + v(49.5,1.3,4), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,4), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-4), position + v(49.5,1.3,-4), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-4), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,2), position + v(-49.5,1.3,2), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,2), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,-2), position + v(-49.5,1.3,-2), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,-2), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,2), position + v(49.5,1.3,2), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,2), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,-2), position + v(49.5,1.3,-2), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,-2), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-49.5,0,0), position + v(-49.5,1.3,0), v(0,0,1), .125)
		circle(div0, position + v(-49.5,1.3,0), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(49.5,0,0), position + v(49.5,1.3,0), v(0,0,1), .125)
		circle(div0, position + v(49.5,1.3,0), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(48.2,0,17.75), position + v(48.2,1.3,17.75), v(0,0,1), .125)
		circle(div0, position + v(48.2,1.3,17.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(48.2,0,-17.75), position + v(48.2,1.3,-17.75), v(0,0,1), .125)
		circle(div0, position + v(48.2,1.3,-17.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-48.2,0,-17.75), position + v(-48.2,1.3,-17.75), v(0,0,1), .125)
		circle(div0, position + v(-48.2,1.3,-17.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-48.2,0,17.75), position + v(-48.2,1.3,17.75), v(0,0,1), .125)
		circle(div0, position + v(-48.2,1.3,17.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(46.9,0,19.5), position + v(46.9,1.3,19.5), v(0,0,1), .125)
		circle(div0, position + v(46.9,1.3,19.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(46.9,0,-19.5), position + v(46.9,1.3,-19.5), v(0,0,1), .125)
		circle(div0, position + v(46.9,1.3,-19.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-46.9,0,-19.5), position + v(-46.9,1.3,-19.5), v(0,0,1), .125)
		circle(div0, position + v(-46.9,1.3,-19.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-46.9,0,19.5), position + v(-46.9,1.3,19.5), v(0,0,1), .125)
		circle(div0, position + v(-46.9,1.3,19.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(45.6,0,21.25), position + v(45.6,1.3,21.25), v(0,0,1), .125)
		circle(div0, position + v(45.6,1.3,21.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(45.6,0,-21.25), position + v(45.6,1.3,-21.25), v(0,0,1), .125)
		circle(div0, position + v(45.6,1.3,-21.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-45.6,0,-21.25), position + v(-45.6,1.3,-21.25), v(0,0,1), .125)
		circle(div0, position + v(-45.6,1.3,-21.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-45.6,0,21.25), position + v(-45.6,1.3,21.25), v(0,0,1), .125)
		circle(div0, position + v(-45.6,1.3,21.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(44.3,0,23), position + v(44.3,1.3,23), v(0,0,1), .125)
		circle(div0, position + v(44.3,1.3,23), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(44.3,0,-23), position + v(44.3,1.3,-23), v(0,0,1), .125)
		circle(div0, position + v(44.3,1.3,-23), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-44.3,0,-23), position + v(-44.3,1.3,-23), v(0,0,1), .125)
		circle(div0, position + v(-44.3,1.3,-23), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-44.3,0,23), position + v(-44.3,1.3,23), v(0,0,1), .125)
		circle(div0, position + v(-44.3,1.3,23), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(43,0,24.75), position + v(43,1.3,24.75), v(0,0,1), .125)
		circle(div0, position + v(43,1.3,24.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(43,0,-24.75), position + v(43,1.3,-24.75), v(0,0,1), .125)
		circle(div0, position + v(43,1.3,-24.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-43,0,-24.75), position + v(-43,1.3,-24.75), v(0,0,1), .125)
		circle(div0, position + v(-43,1.3,-24.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-43,0,24.75), position + v(-43,1.3,24.75), v(0,0,1), .125)
		circle(div0, position + v(-43,1.3,24.75), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(41.7,0,26.5), position + v(41.7,1.3,26.5), v(0,0,1), .125)
		circle(div0, position + v(41.7,1.3,26.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(41.7,0,-26.5), position + v(41.7,1.3,-26.5), v(0,0,1), .125)
		circle(div0, position + v(41.7,1.3,-26.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-41.7,0,-26.5), position + v(-41.7,1.3,-26.5), v(0,0,1), .125)
		circle(div0, position + v(-41.7,1.3,-26.5), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-41.7,0,26.5), position + v(-41.7,1.3,26.5), v(0,0,1), .125)
		circle(div0, position + v(-41.7,1.3,26.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(40.4,0,28.25), position + v(40.4,1.3,28.25), v(0,0,1), .125)
		circle(div0, position + v(40.4,1.3,28.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(40.4,0,-28.25), position + v(40.4,1.3,-28.25), v(0,0,1), .125)
		circle(div0, position + v(40.4,1.3,-28.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-40.4,0,-28.25), position + v(-40.4,1.3,-28.25), v(0,0,1), .125)
		circle(div0, position + v(-40.4,1.3,-28.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(-40.4,0,28.25), position + v(-40.4,1.3,28.25), v(0,0,1), .125)
		circle(div0, position + v(-40.4,1.3,28.25), v(0,1,0), v(0,0,1), .125)
		ring(div0, position + v(39.1,0,30), position + v(40.4,1.1,28.25), v(0,0,1), .125)
		ring(div0, position + v(39.1,0,-30), position + v(40.4,1.1,-28.25), v(0,0,1), .125)
		ring(div0, position + v(-39.1,0,-30), position + v(-40.4,1.1,-28.25), v(0,0,1), .125)
		ring(div0, position + v(-39.1,0,30), position + v(-40.4,1.1,28.25), v(0,0,1), .125)
	--vents
		tube(div0, position + v(0,-10,-53.5), position + v(0,-.3,-53.5), v(1,0,0), .9, 1)
		tube(div0, position + v(0,-10,53.5), position + v(0,-.3,53.5), v(1,0,0), .9, 1)
		tube(div0, position + v(-31.4,-10,-43.3), position + v(-31.4,-.3,-43.3), v(1,0,0), .9, 1)
		tube(div0, position + v(-31.4,-10,43.3), position + v(-31.4,-.3,43.3), v(1,0,0), .9, 1)
		tube(div0, position + v(31.4,-10,43.3), position + v(31.4,-.3,43.3), v(1,0,0), .9, 1)
		tube(div0, position + v(31.4,-10,-43.3), position + v(31.4,-.3,-43.3), v(1,0,0), .9, 1)
		tube(div0, position + v(-51,-10,-16.5), position + v(-51,-.3,-16.5), v(1,0,0), .9, 1)
		tube(div0, position + v(-51,-10,16.5), position + v(-51,-.3,16.5), v(1,0,0), .9, 1)
		tube(div0, position + v(51,-10,16.5), position + v(51,-.3,16.5), v(1,0,0), .9, 1)
		tube(div0, position + v(51,-10,-16.5), position + v(51,-.3,-16.5), v(1,0,0), .9, 1)
	--vent grills
		texture('textures/tex7Uruboros.png', v(.5,.5,0), v(.5,0,0), v(0,0,1))
		circle(div0, position + v(0,-.38,-53.5), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(0,-.38,53.5), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(-31.4,-.38,-43.3), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(-31.4,-.38,43.3), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(31.4,-.38,43.3), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(31.4,-.38,-43.3), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(-51,-.38,-16.5), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(-51,-.38,16.5), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(51,-.38,16.5), v(0,1,0), v(1,0,0), .9)
		circle(div0, position + v(51,-.38,-16.5), v(0,1,0), v(1,0,0), .9)
	--border vent grills
		texture('textures/metal_7280826.png', v(.5,.5,0), v(.07,0,0), v(0,0,1))
		circle(10, position + v(0,-.49,0), v(0,1,0),v(0,0,1), 50.85)
	--elevator frames
		texture('textures/tex4.png', v(.5,.5,0),v(0,.6,0),v(0,0,1))
		cuboid(position + v(-48,0,14), v(-.4,.05,-12))
		cuboid(position + v(-39.6,0,14), v(-.4,.05,-12))
		cuboid(position + v(-48.4,0,14), v(8.8,.05,.4))
		cuboid(position + v(-39.6,0,2), v(-8.8,.05,-.4))
		cuboid(position + v(-48,0,-2), v(-.4,.05,-12))
		cuboid(position + v(-39.6,0,-2), v(-.4,.05,-12))
		cuboid(position + v(-48.4,0,-14.4), v(8.8,.05,.4))
		cuboid(position + v(-39.6,0,-1.6), v(-8.8,.05,-.4))
		cuboid(position + v(48.4,0,14), v(-.4,.05,-12))
		cuboid(position + v(40,0,14), v(-.4,.05,-12))
		cuboid(position + v(39.6,0,14), v(8.8,.05,.4))
		cuboid(position + v(39.6,0,1.6), v(8.8,.05,.4))
		cuboid(position + v(48.4,0,-2), v(-.4,.05,-12))
		cuboid(position + v(40,0,-2), v(-.4,.05,-12))
		cuboid(position + v(39.6,0,-14.4), v(8.8,.05,.4))
		cuboid(position + v(39.6,0,-2), v(8.8,.05,.4))
	--lamp housings
		cylinder(div1, position + v(-30,0,30), position + v(-30,.05,30), v(0,0,1), .8)
		cylinder(div1, position + v(30,0,30), position + v(30,.05,30), v(0,0,1), .8)
		cylinder(div1, position + v(-30,0,-30), position + v(-30,.05,-30), v(0,0,1), .8)
		cylinder(div1, position + v(30,0,-30), position + v(30,.05,-30), v(0,0,1), .8)
	--railings
		ring(div0, position + v(40.4,1.2,28.25), position + v(49.5,1.2,16), v(0,0,1), .0625)
		ring(div0, position + v(40.4,1.2,-28.25), position + v(49.5,1.2,-16), v(0,0,1), .0625)
		ring(div0, position + v(-40.4,1.2,-28.25), position + v(-49.5,1.2,-16), v(0,0,1), .0625)
		ring(div0, position + v(-40.4,1.2,28.25), position + v(-49.5,1.2,16), v(0,0,1), .0625)
		ring(div0, position + v(40.4,.6,28.25), position + v(49.5,.6,16), v(0,0,1), .0625)
		ring(div0, position + v(40.4,.6,-28.25), position + v(49.5,.6,-16), v(0,0,1), .0625)
		ring(div0, position + v(-40.4,.6,-28.25), position + v(-49.5,.6,-16), v(0,0,1), .0625)
		ring(div0, position + v(-40.4,.6,28.25), position + v(-49.5,.6,16), v(0,0,1), .0625)
		ring(div0, position + v(-49.5,1.2,-16), position + v(-49.5,1.2,16), v(1,0,0), .0625)
		ring(div0, position + v(49.5,1.2,-16), position + v(49.5,1.2,16), v(1,0,0), .0625)
		ring(div0, position + v(-49.5,.6,-16), position + v(-49.5,.6,16), v(1,0,0), .0625)
		ring(div0, position + v(49.5,.6,-16), position + v(49.5,.6,16), v(1,0,0), .0625)
	--border vents
		cylinder(4, position + v(0,-.47,-53.5), position + v(31.4,-.47,-43.3), v(0,0,1), .0625)
		cylinder(4, position + v(31.4,-.47,-43.3), position + v(51,-.47,-16.5), v(0,0,1), .0625)
		cylinder(4, position + v(51,-.47,-16.5), position + v(51,-.47,16.5), v(1,0,0), .0625)
		cylinder(4, position + v(31.4,-.47,43.3), position + v(51,-.47,16.5), v(0,0,1), .0625)
		cylinder(4, position + v(0,-.47,53.5), position + v(31.4,-.47,43.3), v(0,0,1), .0625)
		cylinder(4, position + v(0,-.47,53.5), position + v(-31.4,-.47,43.3), v(0,0,1), .0625)
		cylinder(4, position + v(-31.4,-.47,43.3), position + v(-51,-.47,16.5), v(0,0,1), .0625)
		cylinder(4, position + v(-51,-.47,-16.5), position + v(-51,-.47,16.5), v(1,0,0), .0625)
		cylinder(4, position + v(-31.4,-.47,-43.3), position + v(-51,-.47,-16.5), v(0,0,1), .0625)
		cylinder(4, position + v(0,-.47,-53.5), position + v(-31.4,-.47,-43.3), v(0,0,1), .0625)
	end

-- draw the pad body
	if lod > 1 then
		texture('textures/tex12.png', v(.5,.5,0), v(.005,0,0), v(0,.02,0))
	end
	set_material('body', .3, .3, .3, 1)
	use_material('body')
	lathe(10, position + v(0,-30.5,0), position + v(0,-100,0), v(0,0,1), {0,10, 1,20})
	if lod > 1 then
		texture(nil)
		set_material('lens', .2, .2, 0, .35, .2, .2, 0, 100, .2, .2, 0)
		use_material('lens')
		tapered_cylinder(16, position + v(-30,.05,30), position + v(-30,.15,30), v(0,0,1), .6, .25)
		tapered_cylinder(16, position + v(30,.05,30), position + v(30,.15,30), v(0,0,1), .6, .25)
		tapered_cylinder(16, position + v(-30,.05,-30), position + v(-30,.15,-30), v(0,0,1), .6, .25)
		tapered_cylinder(16, position + v(30,.05,-30), position + v(30,.15,-30), v(0,0,1), .6, .25)
		--sequential lights-code from potsmoke66
		circle(6 , position + v(0,0.1,46), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,38), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,30), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,22), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,14), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,-46), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,-38), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,-30), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,-22), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(0,0.1,-14), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(46,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(38,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(30,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(22,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(14,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(-46,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(-38,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(-30,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(-22,0.1,0), v(0,1,0), v(0,0,1), .3)
		circle(6 , position + v(-14,0.1,0), v(0,1,0), v(0,0,1), .3)
	end
end

function createLandingPadDynamic(padNum, position, lod)
	-- padNum: The landing pad number (zero based)
	-- position: vector of the landing pad v(0,0,0) is where the ship lands
	local padId = 'DOCKING_BAY_' .. (padNum + 1)
	local stage = get_animation_stage(padId) -- used to determine landing lights
	local div0 = 2*lod
	local div1 = 4*lod

	-- station name label
	if lod > 1 then
		texture('lmrmodels/stations/textures/seamless_texture_07.png', v(.5,.5,0),v(0,0.125,0),v(0,0,1.25))
		set_material('text', 1, 1, 1, 1)
		use_material('text')
		zbias(1, position + v(0,0,0), v(0,1,0))
		-- starport name
		text(get_label(), position + v(0,0,-35), v(0,1,0), v(1,0,0), 5.0, {center=true})
		zbias(0)
	end

	-- dynamic lights
	if lod > 1 then
		if (math.fmod(get_time('SECONDS'), 2) > 1) then
			local color
			--local light
			if stage > 1 or stage < 0 then
				color = v(1,0,0) -- red
			elseif stage == 1 then
				color = v(0,1,0) -- green
			else
				color = v(1,0.5,0) -- orange
			end
			--landing lights
			texture(nil)
			set_material('lit_lamp', 1,0.5,0.5,1,1,0.5,0.5,5,1,0.5,0.5)
			use_material('lit_lamp')
			tapered_cylinder(div1, position + v(-30,.05,30), position + v(-30,.15,30), v(0,0,1), .6, .25)
			tapered_cylinder(div1, position + v(30,.05,30), position + v(30,.15,30), v(0,0,1), .6, .25)
			tapered_cylinder(div1, position + v(-30,.05,-30), position + v(-30,.15,-30), v(0,0,1), .6, .25)
			tapered_cylinder(div1, position + v(30,.05,-30), position + v(30,.15,-30), v(0,0,1), .6, .25)
			billboard('smoke.png', 50, color, {
				position + v(-30,3,30),
				position + v(30,3,30),
				position + v(-30,3,-30),
				position + v(30,3,-30) })
		end

		local freq0 = math.fmod(get_time('SECONDS'),1)
		local freq1 = math.sin(freq0*50)
		if stage == 1 then
			billboard('smoke.png', 20, v(0,freq1*2,0), {
				vlerp(freq0, position + v(0,0.15,50), position + v(0,0.15,10)),
				vlerp(freq0, position + v(0,0.15,-50), position + v(0,0.15,-10))})

			billboard('smoke.png', 20, v(0,freq1*2,0), {
				vlerp(freq0, position + v(50,0.15,0), position + v(10,0.15,0)),
				vlerp(freq0, position + v(-50,0.15,0), position + v(-10,0.15,0))})
		end

	end
end

define_model('ground_station_1', {
	info = {
		lod_pixels = {.1,80,160,0},
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body', 'lens', 'screen', 'lit_lamp'},
		tags = {'surface_station'},
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		createLandingPadStatic(0, v(-150,50,0), lod)
	end,
	dynamic = function(lod)
		-- display dynamic landing lights
		createLandingPadDynamic(0, v(-150,50,0), lod)
	end,
})

define_model('ground_station_2', {
	info = {
		lod_pixels = {.1,80,160,0},
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body', 'lens', 'screen', 'lit_lamp'},
		tags = {'surface_station'},
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		createLandingPadStatic(0, v(-150,50,0),lod)
		createLandingPadStatic(1, v(150,50,0),lod)
	end,
	dynamic = function(lod)
		-- display dynamic landing lights
		createLandingPadDynamic(0, v(-150,50,0),lod)
		createLandingPadDynamic(1, v(150,50,0),lod)
	end,
})

define_model('ground_station_3', {
	info = {
		lod_pixels = {.1,80,160,0},
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body', 'lens', 'screen', 'lit_lamp'},
		tags = {'surface_station'},
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		createLandingPadStatic(0, v(-150,50,0),lod)
		createLandingPadStatic(1, v(150,50,0),lod)
		createLandingPadStatic(2, v(0,50,-150),lod)
	end,
	dynamic = function(lod)
		-- display dynamic landing lights
		createLandingPadDynamic(0, v(-150,50,0),lod)
		createLandingPadDynamic(1, v(150,50,0),lod)
		createLandingPadDynamic(2, v(0,50,-150),lod)
	end,
})

define_model('ground_station_4', {
	info = {
		lod_pixels = {.1,80,160,0},
		bounding_radius=300.0,
		materials = {'text', 'pad', 'body', 'lens', 'screen', 'lit_lamp'},
		tags = {'surface_station'},
	},
	static = function(lod)
		-- Control Tower
		call_model('control_tower', v(0,0,0), v(1,0,0), v(0,1,0), 1.0)
		createLandingPadStatic(0, v(-150,50,0),lod)
		createLandingPadStatic(1, v(150,50,0),lod)
		createLandingPadStatic(2, v(0,50,-150),lod)
		createLandingPadStatic(3, v(0,50,150),lod)
	end,
	dynamic = function(lod)
		-- display dynamic landing lights
		createLandingPadDynamic(0, v(-150,50,0),lod)
		createLandingPadDynamic(1, v(150,50,0),lod)
		createLandingPadDynamic(2, v(0,50,-150),lod)
		createLandingPadDynamic(3, v(0,50,150),lod)
	end,
})
