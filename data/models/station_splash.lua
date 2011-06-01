define_model('station_splash', {
	info = {
			bounding_radius = 1.0,
			materials = { 'bg', 'text', 'glow1'},
			tags = {'advert'}
		},
	static = function(lod)
		set_material('bg', .8,.8,.8,1)
		set_material('glow1', 0,0,0,1,0,0,0,0,1,2,2.5)
		--use_material('bg')
		use_material('glow1')

	
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