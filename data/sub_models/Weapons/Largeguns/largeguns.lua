-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('largegun1', {
   	info = {
			bounding_radius = 5,
			},

	static = function(lod)

		texture(nil)
		load_obj('lfg.obj')

	end

})

define_model('largegun2', {
   	info = {
			bounding_radius = 5,
			},

	static = function(lod)

		texture(nil)
		load_obj('lrg.obj')

	end

})
