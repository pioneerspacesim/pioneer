-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('blank', {
	info = {
			bounding_radius = 1,
            materials = {'null'},
			},

	static = function(lod)
		set_material('null', 0,0,0,0,0,0,0,0,0,0,0,0)
		use_material('null')
		tri(v(0,0,0),v(1,0,0),v(1,1,0))
	end
})
