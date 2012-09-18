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
