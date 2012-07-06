define_model('squad_color', {
	info = 	{
        	bounding_radius = 1,
			materials = {'squad'},
			},
	static = function(lod)
	end,
	dynamic = function(lod)
    	selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.6,.6,.6,30)
	    else
			if select1 < 401 then
               	set_material('squad', .45,.35,.01,.99,.6,.6,.6,30)
			else
    		    if select1 < 601 then
					set_material('squad', 0,.15,.7,.99,.6,.6,.6,30)
   				else
   				    if select1 < 801 then
						set_material('squad', .06,.35,0,.99,.6,.6,.6,30)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,.99,.6,.6,.6,30)
						end
					end
				end
			end
		end
		use_material('squad')
	end
})

-- four different types, because i couldn't decide which i like best ;)

define_model('squadsign_1', {
	info = 	{
        	bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)

	end,

	dynamic = function(lod)

		selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('squad', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('squad', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('squad', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end


	use_material('squad')
	texture('sub_models/squadsign/squad_1.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	zbias(100,v(0,0,0),v(0,0,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	zbias(0)
	end
})

define_model('squadsign_2', {
	info = 	{
        	bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)

	end,

	dynamic = function(lod)

		selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('squad', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('squad', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('squad', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end


	use_material('squad')
	texture('sub_models/squadsign/squad_2.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	zbias(100,v(0,0,0),v(0,0,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	zbias(0)
	end
})

define_model('squadsign_3', {
	info = 	{
        	bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)

	end,

	dynamic = function(lod)

		selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('squad', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('squad', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('squad', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end


	use_material('squad')
	texture('sub_models/squadsign/squad_3.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	zbias(100,v(0,0,0),v(0,0,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	zbias(0)
	end
})

define_model('squadsign_4', {
	info = 	{
        	bounding_radius = 1,
			materials = {'squad'},
			},

	static = function(lod)

	end,

	dynamic = function(lod)

		selector1()
		if select1 < 201 then
	        set_material('squad', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('squad', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('squad', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('squad', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('squad', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end


	use_material('squad')
	texture('sub_models/squadsign/squad_4.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	zbias(100,v(0,0,0),v(0,0,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	zbias(0)
	end
})
