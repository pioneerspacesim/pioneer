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
		if select1 < 251 then
	        set_material('squad', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 501 then
               	set_material('squad', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 751 then
					set_material('squad', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 > 750 then
						set_material('squad', .06,.35,0,.99,.3,.3,.3,5)
					end
				end
			end
		end
	

	use_material('squad')
	texture('sub_models/squadsign/squad_1.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	
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
		if select1 < 251 then
	        set_material('squad', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 501 then
               	set_material('squad', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 751 then
					set_material('squad', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 > 750 then
						set_material('squad', .06,.35,0,.99,.3,.3,.3,5)
					end
				end
			end
		end
	

	use_material('squad')
	texture('sub_models/squadsign/squad_2.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	
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
		if select1 < 251 then
	        set_material('squad', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 501 then
               	set_material('squad', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 751 then
					set_material('squad', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 > 750 then
						set_material('squad', .06,.35,0,.99,.3,.3,.3,5)
					end
				end
			end
		end
	

	use_material('squad')
	texture('sub_models/squadsign/squad_3.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	
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
		if select1 < 251 then
	        set_material('squad', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 501 then
               	set_material('squad', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 751 then
					set_material('squad', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 > 750 then
						set_material('squad', .06,.35,0,.99,.3,.3,.3,5)
					end
				end
			end
		end
	

	use_material('squad')
	texture('sub_models/squadsign/squad_4.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
	quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1))
	
	end    
})
