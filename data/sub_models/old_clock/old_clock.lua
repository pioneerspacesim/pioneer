define_model('clkmin', {
	info = {
			bounding_radius = 1,
			materials={'gold'}
		},
	static = function(lod)
		set_material('gold', 1,1,1, 1, 1, .8, .8, 100, .3, .3, .3)
		use_material('gold')
		zbias(3, v(0,0,0), v(0,0,1))
		load_obj('old_minute.obj', Matrix.new(v(1,0,0),v(0,1,0),v(0,0,1)))
	end
})

define_model('clkhr', {
	info = {
			bounding_radius = 1,
			materials={'gold'}
		},
	static = function(lod)
		set_material('gold', 1,1,1, 1, 1, .8, .8, 100, .3, .3, .3)
		use_material('gold')
		zbias(3, v(0,0,0), v(0,0,1))
		load_obj('old_hour.obj', Matrix.new(v(1,0,0),v(0,1,0),v(0,0,1)))
	end
})


define_model('old_clock', {
	info = {
			bounding_radius=1,
			materials={'face',  'numbers'}
		},
	static = function(lod)
		set_material('face', .9, .9, .9, 1, .1, .1, .1, 10)
		set_material('numbers', .68, .56, .17, 1, 1, .8, .8, 100, .2, .2, .2)
		use_material('face')
		texture('old_clock.png')
		zbias(1, v(0,0,0), v(0,0,1))
		load_obj('old_clock.obj', Matrix.new(v(1.1,0,0),v(0,1.1,0),v(0,0,1.1)))
		zbias(2, v(0,0,0), v(0,0,1))
		texture(nil)

		use_material('numbers')
		text("XII", v(0,4,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("I", v(2,3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("II", v(3.47,2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("III", v(4,0,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("IV", v(3.47,-2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("V", v(2,-3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VI", v(0,-4,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VII", v(-2,-3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("VIII", v(-3.47,-2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("IX", v(-4,0,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("X", v(-3.47,2,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("XI", v(-2,3.47,0), v(0,0,1), v(1,0,0), 1, {center=true})
		text("Potsmoke66 Watches", v(0,-1,0), v(0,0,1), v(1,0,0), 0.3, {center=true})
		text("Switzerland", v(0,-1.5,0), v(0,0,1), v(1,0,0), 0.3, {center=true}) 
		zbias(0)

	end,
	dynamic = function(lod)
		local handPos = -2*math.pi * get_arg(3)
		call_model('clkmin', v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 1)
		handPos = handPos / 12
		call_model('clkhr', v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 1)
	end
})
