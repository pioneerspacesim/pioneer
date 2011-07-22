
define_model('clockhand', {
	info = {
		bounding_radius = 1,
		materials={'mat'}
	},
	static = function(lod)
		set_material("mat",0,0,1,1, 0,0,0, 10)
		use_material("mat")
		zbias(3, v(0,0,0), v(0,0,1))
		tri(v(-0.06, -0.06, 0), v(0.06, -0.06, 0), v(0, 1, 0))
	end
})

define_model('clock', {
	info = {
		bounding_radius=1,
		materials={'face','numbers'}
	},
	static = function(lod)
		set_material("face", 1,1,1,1)
		set_material("numbers",.5,.5,0,1)
		use_material("face")
		zbias(1, v(0,0,0), v(0,0,1))
		circle(24, v(0,0,0), v(0,0,1), v(0,1,0), 1.0)
		zbias(2, v(0,0,0), v(0,0,1))

		use_material("numbers")
		text("12", v(0,0.85,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("1", v(0.425,0.74,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("2", v(0.74,0.43,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("3", v(0.85,0,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("4", v(0.74,-.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("5", v(0.425,-.74,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("6", v(0,-.85,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("7", v(-.425,-.736,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("8", v(-.74,-.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("9", v(-.85,0,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("10", v(-.74,.425,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		text("11", v(-.425,.736,0), v(0,0,1), v(1,0,0), 0.15, {center=true})
		zbias(0)

	end,
	dynamic = function(lod)
		local handPos = -2*math.pi * get_arg(3)
		call_model("clockhand", v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 0.65)
		handPos = handPos / 12
		call_model("clockhand", v(0,0,0),
				v(math.cos(handPos),math.sin(handPos),0),
				v(math.cos(handPos+math.pi*0.5), math.sin(handPos+math.pi*0.5),0), 0.45)
	end
})


function biodome(lod, trans)
	local d = 1/math.sqrt(2)
	local height = 40
	use_material('base')
	local yank=-690
	xref_cubic_bezier_quad(32, 1, trans*v(0,0,500), trans*v(0,0.25*height,500), trans*v(0,0.75*height,500), trans*v(0,height,500),
			trans*v(yank,0,500), trans*v(yank,0.25*height,500), trans*v(yank,0.75*height,500), trans*v(yank,height,500),
			trans*v(yank,0,-500), trans*v(yank,0.25*height,-500), trans*v(yank,0.75*height,-500), trans*v(yank,height,-500),
			trans*v(0,0,-500), trans*v(0,0.25*height,-500), trans*v(0,0.75*height,-500), trans*v(0,height,-500))
	use_material('green')
	zbias(1, v(0,height,0), v(0,1,0))
	local s = 660
	quadric_bezier_quad(16, 16, trans*v(d*500,height,d*-500), trans*v(0,height,-s), trans*v(d*-500,height,d*-500),
		trans*v(s,height,0), trans*v(0,height,0), trans*v(-s,height,0),
		trans*v(d*500,height,d*500), trans*v(0,height,s), trans*v(d*-500,height,d*500))
	use_material('dome')
	quadric_bezier_quad(16, 16, trans*v(d*500,height,d*-500), trans*v(0,height,-s), trans*v(d*-500,height,d*-500),
		trans*v(s,height,0), trans*v(0,500,0), trans*v(-s,height,0),
		trans*v(d*500,height,d*500), trans*v(0,height,s), trans*v(d*-500,height,d*500))
	zbias(0)
end

define_model('biodomes', {
	info = {
		bounding_radius=1000,
		materials={'base','green','dome'},
		tags = {'city_building'},
		},
	static = function(lod)
		set_material('base', .4,.4,.5,1, .5,.5,.7,40)
		set_material('green', .1,.6,.1,1)
		set_material('dome', .5,.5,1,.3, 1,1,1,100)
		m = Matrix.translate(v(0,0,0))
		biodome(lod, m)
	--	m = Matrix.scale(v(0.5,0.5,0.5)) * Matrix.translate(v(600,0,0))
	--	biodome(lod, m)

		use_material("base")
	end
})

