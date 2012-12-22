-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

define_model('station_splash', {
	info = {
			bounding_radius = 1.0,
			materials = { 'splash' },
			tags = {'advert'}
		},
	static = function(lod)
		set_material('splash', 1,1,1,1)
		use_material('splash')

		set_light(1, 0.00005, v(0,0,1), v(1,1,1))
		set_local_lighting(true)
		use_light(1)

		zbias(1, v(0,0,0), v(0,0,1))
		texture('ships/default_ship_textures/station_splash.png', v(.5,.5,0),v(.5,0,0),v(0,-1,0))
		quad(v(-1,-0.5,0),v(1,-0.5,0),v(1,0.5,0),v(-1,0.5,0))
		zbias(2, v(0,0,0),v(0,0,1))
		--set_material('text', 0,0,0,1)
		--use_material('text')
		--text("STATION SPLASH:", v(0,.2,0), v(0,0,1), v(1,0,0), .14, {center=true})
		--text("NOW EVEN MORE BAD", v(0,-.2,0), v(0,0,1), v(1,0,0), .14, {center=true})
		--texture('ships/default_ship_textures/station_splash.png', v(.5,0,0),v(.5,0,0),v(0,-1,0))

		zbias(0)
	end
})
