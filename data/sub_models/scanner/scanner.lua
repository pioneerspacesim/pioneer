-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--[[ scanner models
		set a material of your choice for the base and the shell,
		scale the model when called to your ships size.

		"scanner" 		-model without base
		"scanner_+"     -model with a pentagonal basepart
  ]]--

define_model('scanner_sub', {
	info = {
			bounding_radius = 1,
			materials = {'scan1', 'scan2'}
            },

	static = function(lod)
	    texture('scan.png')
		load_obj('scanner_01.obj', matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1)))
		set_material('scan2', .4, 0, 0,1, .5, .5, .5, 10)
		use_material('scan2')
		load_obj('scanner_03.obj', matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1)))
  		set_material('scan1', .63,.7,.83,1,1.26,1.4,1.66,10)
		use_material('scan1')
        load_obj('scanner_02.obj', matrix.new(v(-1,0,0),v(0,1,0),v(0,0,-1)))
	end
})

define_model('scanner', {
	info = {
			bounding_radius = 1,
			},

	static = function(lod)
	end,
	dynamic = function(lod)
		local factor = (get_time('SECONDS')*0.5)*math.pi
		call_model('scanner_sub', v(0,0,0), v(math.cos(factor),0,math.sin(factor)), v(0,1,0),1)
	end
})

define_model('scanner_+', {
	info = {
			bounding_radius = 1,
			},

	static = function(lod)
	texture('scan.png')
	sphere_slice(5,2, 0, 0.5*math.pi, matrix.scale(v(.8,0.3,.8)))
	end,
	dynamic = function(lod)
		local factor = (get_time('SECONDS')*0.5)*math.pi
		call_model('scanner_sub', v(0,.3,0), v(math.cos(factor),0,math.sin(factor)), v(0,1,0),1)
	end
})

define_model('scanner_-', {
	info = {
			bounding_radius = 1,
			},

	static = function(lod)
	texture('scan.png')
	sphere_slice(5,2, 0, 0.5*math.pi, matrix.scale(v(.8,0.3,.8)))
	end,
	dynamic = function(lod)
		local factor = (get_time('SECONDS')*0.5)*math.pi
		call_model('scanner_sub', v(0,.3,0), v(math.sin(factor),0,math.cos(factor)), v(0,1,0),1)
	end
})
