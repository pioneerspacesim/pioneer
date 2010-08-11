define_model('gelati_l', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('pine_01.png')
		load_obj('pine_01.obj', Matrix.new(v(0.66,0,0),v(0,1,0),v(0,0,0.66)))
end
})

define_model('gelati_n', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('pine_01.png')
		load_obj('pine_01.obj', Matrix.new(v(1,0,0),v(0,1,0),v(0,0,1)))
end
})


define_model('gelati_w', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('pine_01.png')
		load_obj('pine_01.obj', Matrix.new(v(1.5,0,0),v(0,1,0),v(0,0,1.5)))
end
})

define_model('broccoli_l', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('oak_01.png')
		load_obj('oak_01.obj', Matrix.new(v(0.66,0,0),v(0,1,0),v(0,0,0.66)))
end
})

define_model('broccoli_n', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('oak_01.png')
		load_obj('oak_01.obj', Matrix.new(v(1,0,0),v(0,1,0),v(0,0,1)))
end
})

define_model('broccoli_w', {
		
	info = {
			bounding_radius=5,
			materials={'tree'}
			},
	static = function(lod)
		set_material('tree', 1,1,1, .99, .6,.6,.5, 1)
		use_material ('tree') 
		texture('oak_01.png')
		load_obj('oak_01.obj', Matrix.new(v(1.5,0,0),v(0,1,0),v(0,0,1.5)))
end
})



define_model('woods_1', {

	info = {
	        bounding_radius=60,
	        tags = {'city_building', 'city_power'}
	        },

	static = function(lod)

		call_model('gelati_l', v(10,0,15), v(1,0,0), v(0,1,0), 30)
		call_model('gelati_l', v(-15,0,20), v(0,0,-1), v(0,1,0), 25)
		call_model('broccoli_w', v(20,0,5), v(-1,0,0), v(0,1,0), 10)
		call_model('gelati_w', v(15,0,-15), v(0,0,1), v(0,1,0), 6)
		call_model('broccoli_l', v(-20,0,-5), v(0,0,-1), v(0,1,0), 15)
		call_model('gelati_n', v(10,0,-10), v(-1,0,0), v(0,1,0), 12)
	   	call_model('broccoli_n', v(-5,0,-20), v(1,0,0), v(0,1,0), 6)
		call_model('broccoli_n', v(5,0,-20), v(1,0,0), v(0,1,0), 6)
		
	end
})	

