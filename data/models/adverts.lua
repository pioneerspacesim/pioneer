
define_model('diet_steakette', {
	info = {
			bounding_radius = 1.0,
			materials = { 'bg', 'text' }
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
		
