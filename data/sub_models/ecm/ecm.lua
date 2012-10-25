-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('ecm_1', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
         	texture('metal.png')
			sphere_slice(3*lod, 2*lod, 0, 0.4*math.pi, matrix.scale(v(.3,.2,.3)))
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')
			ring(3*lod, v(0,.2,0), v(0,1,0), v(0,0,1), .04)
			cylinder(3*lod, v(0,1,0), v(0,1,-1.4), v(0,1,0), .04)
			cylinder(3*lod, v(-.6,1,0), v(.6,1,0), v(0,1,0), .04)
			cylinder(3*lod, v(-.5,1,-0.3), v(.5,1,-0.3), v(0,1,0), .04)
			cylinder(3*lod, v(-.4,1,-0.6), v(.4,1,-0.6), v(0,1,0), .04)
			cylinder(3*lod, v(-.3,1,-0.9), v(.3,1,-0.9), v(0,1,0), .04)
			cylinder(3*lod, v(-.2,1,-1.2), v(.2,1,-1.2), v(0,1,0), .04)
	end
})

define_model('ecm_2', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
         	texture('metal.png')
			sphere_slice(4*lod, 2*lod, 0, 0.4*math.pi, matrix.scale(v(.4,.2,.4)))
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')
			ring(3*lod, v(0,.16,0), v(0,1,0), v(0,1,1), .04)
            ring(3*lod, v(0,1,0), v(0,1,-1.5), v(0,1,1), .04)
            cylinder(4*lod, v(0,1,-.1), v(0,1,-.14), v(0,1,0), .6)
   			cylinder(4*lod, v(0,1,-.4), v(0,1,-.44), v(0,1,0), .5)
   			cylinder(4*lod, v(0,1,-.7), v(0,1,-.74), v(0,1,0), .4)
   			cylinder(4*lod, v(0,1,-1), v(0,1,-1.04), v(0,1,0), .3)
   			cylinder(4*lod, v(0,1,-1.3), v(0,1,-1.34), v(0,1,0), .2)
			sphere_slice(3*lod,2*lod, 0, math.pi, matrix.translate(v(0,1,-1.56)) * matrix.scale(v(.1,.1,.1)))
	end
})

define_model('ecm_1a', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
         	texture('metal.png')
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')

			cylinder(3*lod, v(0,0,0), v(0,0,-2.4), v(0,1,0), .04)
			cylinder(3*lod, v(-.6,0,-1), v(.6,0,-1), v(0,1,0), .04)
			cylinder(3*lod, v(-.5,0,-1.3), v(.5,0,-1.3), v(0,1,0), .04)
			cylinder(3*lod, v(-.4,0,-1.6), v(.4,0,-1.6), v(0,1,0), .04)
			cylinder(3*lod, v(-.3,0,-1.9), v(.3,0,-1.9), v(0,1,0), .04)
			cylinder(3*lod, v(-.2,0,-2.2), v(.2,0,-2.2), v(0,1,0), .04)
	end
})

define_model('ecm_2a', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chrome'}
			},
	static = function(lod)
         	texture('metal.png')
			set_material('chrome', .63,.7,.83,1,1.26,1.4,1.66,10)
			use_material('chrome')

            ring(3*lod, v(0,0,0), v(0,0,-2.5), v(0,1,1), .04)
            cylinder(4*lod, v(0,0,-1.1), v(0,0,-1.14), v(0,1,0), .6)
   			cylinder(4*lod, v(0,0,-1.4), v(0,0,-1.44), v(0,1,0), .5)
   			cylinder(4*lod, v(0,0,-1.7), v(0,0,-1.74), v(0,1,0), .4)
   			cylinder(4*lod, v(0,0,-2), v(0,0,-2.04), v(0,1,0), .3)
   			cylinder(4*lod, v(0,0,-2.3), v(0,0,-2.34), v(0,1,0), .2)
			sphere_slice(3*lod,2*lod, 0, math.pi, matrix.translate(v(0,0,-2.56)) * matrix.scale(v(.1,.1,.1)))
	end
})
