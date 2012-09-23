-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('console1', {
	info =	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 5,
			materials = {'chromea', 'random1'},
			},

	static = function(lod)

			set_material('chromea', .63,.7,.83,1,1.26,1.4,1.66,50)
			use_material('chromea')
			load_obj('console_arm.obj')
			use_material('random1')
			load_obj('console_screen.obj')

	end,

	dynamic = function(lod)

		set_material('random1', get_arg_material(0))

	end
})
