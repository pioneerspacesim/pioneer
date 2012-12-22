-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('diet_steakette', {
	info = {
		bounding_radius = 1.0,
		materials = { 'bg', 'text' },
		tags = {'advert'}
	},
	static = function(lod)
		set_material('bg', .8,.8,.8,1)
		use_material('bg')
		zbias(1, v(0,0,0), v(0,0,1))
		quad(v(-1,-0.5,0),v(1,-0.5,0),v(1,0.5,0),v(-1,0.5,0))
		zbias(2, v(0,0,0),v(0,0,1))
		set_material('text', 0,0,0,1)
		use_material('text')
		text("DIET STEAKETTE:", v(0,.2,0), v(0,0,1), v(1,0,0), .14, {center=true})
		text("NOW EVEN MORE BAD", v(0,-.2,0), v(0,0,1), v(1,0,0), .14, {center=true})
		zbias(0)
	end
})

define_model('ad_sirius', {
	info = {
		bounding_radius = 1.0,
		materials = { 'bg', 'text' },
		tags = {'advert'}
	},
	static = function(lod)
		set_material('bg', .4,.4,.8,1, .4,.4,.8)
		use_material('bg')
		zbias(1, v(0,0,0), v(0,0,1))
		quad(v(-1,-0.5,0),v(1,-0.5,0),v(1,0.5,0),v(-1,0.5,0))
		zbias(2, v(0,0,0),v(0,0,1))
		set_material('text', 0,0,0,1)
		use_material('text')
		text("Sirius Ships", v(0,0,0), v(0,0,1), v(1,0,0), .2, {center=true})
		zbias(0)
	end
})
