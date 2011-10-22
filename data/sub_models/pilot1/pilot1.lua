
define_model('head1', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'face', 'helm', 'glass'},
			},
			
	static = function(lod)
	    
        set_material('glass', .05,.04,.1,.8,1,1,3,100)
		
			    	
	    use_material('helm')
    	sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.new(v(1.1,0,0), v(0,1.1,0), v(0,0,1.2)))
	    sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.new(v(1.1,0,0), v(0,.90,0.3), v(0,-0.3,1.17))* Matrix.new(v(1,0,0), v(0,0,-1), v(0,1,0)))
		circle(6*lod, v(0,0,-.3), v(0,1,0), v(0,0,1), 1.05)
        circle(6*lod, v(0,0,-.3), v(0,-1,0), v(0,0,1), 1.05)

        use_material('face')
        texture('head01.png',v(0.5,.6,0),v(.35,0,0),v(0,-.55,0))
	    sphere_slice(3*lod, 2*lod, 0.6*math.pi, 0, Matrix.new(v(.9,0,0), v(0,-1.1,-0.1), v(0,-0.3,1.1)))	
		ring(4*lod, v(0,-0.8,0), v(0,-1.1,0), v(0,0,1), 0.6)
		texture(nil)
	end,
	
	dynamic = function(lod)

   		local visor = math.clamp(get_animation_position('WHEEL_STATE'), 0.5, 1)
		use_material('glass')
		cylinder(4*lod, v(0,-1+visor,-0.23), v(0,0,-0.23), v(0,0,1), .93)


--local select1 = 200  -- try this to load model in modelviewer

		selector1()   -- disable this to load model in modelviewer, i have no idea why, other submodels work well with selector call includet :(
		if select1 < 201 then
	        set_material('helm', .5,0,0,1,.7,.7,.7,50)
	    else
			if select1 < 401 then
               	set_material('helm', .45,.35,.01,1,.6,.6,.6,50)
			else
    		    if select1 < 601 then
					set_material('helm', 0,.1,.5,1,.8,.8,.8,50)
   				else
   				    if select1 < 801 then
						set_material('helm', 0.05,.25,0,1,.7,.7,.7,50)
					else
						if select1 > 800 then
							set_material('helm', .2,0,.35,1,.6,.6,.6,50)
						end						
					end
				end
			end
		end

--local select2 = 10
		selector2()
		if select2 < 13 then
      		set_material('face', .5,.4,.3,1,.3,.3,.3,5)
      	else
      		if select2 < 26 then
      			set_material('face', 0.07,0.02,0.01,1,.3,.3,.3,5)
			else
			    if select2 < 38 then
            		set_material('face', .3,.2,0.08,1,.3,.3,.3,5)
				else
				    if select2 < 51 then
				        set_material('face', .2,.12,0.02,1,.3,.3,.3,5)
					else
					    if select2 < 63 then
                            set_material('face', .50,.4,.3,1,.3,.3,.3,5)
						else
						    if select2 < 76 then
						        set_material('face', 0.07,0.02,0.01,1,.3,.3,.3,5)
							else
								if select2 < 88 then
								    set_material('face', .3,.2,0.08,1,.3,.3,.3,5)
								else
								    if select2 > 87 then
								        set_material('face', .2,.12,0.02,1,.3,.3,.3,5)
									end
								end
							end
						end
					end
				end
			end
		end

	end	    
})


define_model('pilot1', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 3,
			materials = {'dress1', 'dress2', 'grey', 'black'},
			},
			
	static = function(lod)
	
		use_material('dress1')
  		sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.scale(v(2,0.8,1))) -- shoulder
        sphere_slice(4*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(1.6,-0.3,0)) * Matrix.rotate(-0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- r_axle
	    sphere_slice(3*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(-1.6,-0.3,0)) * Matrix.rotate(0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- l_axle
        extrusion(v(0,-2.4,-0.7), v(0,-2.4,-0.9), v(0,1,0), 1, v(-0.25,-0.25,0), v(0.25,-0.25,0), v(0.5,-0.05,0), v(0.5,0.3,0), v(0.25,0.5,0), v(-0.25,0.5,0), v(-0.5,0.3,0.05), v(-0.5,-0.05,0))
        sphere(.5*lod, Matrix.translate(v(0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,-1,0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- r_hand
        sphere(.5*lod, Matrix.translate(v(-0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,1,-0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- l_hand

	    use_material('dress2')
	    ring(4*lod, v(0,0.6,0), v(0,0.9,0), v(0,0,1), 0.61) -- neck
	    sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.scale(v(2,-4,1)))  -- top
		sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,0.4,0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- ru_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,-0.4,-0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- lu_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,-0.6,-1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,0.6,1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,-0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- ru_leg
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- lu_leg
        		
        set_material('grey', .1,.1,.1,1,.2,.2,.2,5)
		use_material('grey')

		cubic_bezier_quad(3*lod,1,v(2.5,1,0.64), v(2.5,0,0.51), v(2.5,-2,0.49), v(2.5,-4.3,0.47),
                                v(1.5,3.5,1.14), v(1.5,0,1.01), v(1.5,-2,0.99), v(1.5,-4.3,0.97),
                                v(-1.5,3.5,1.14), v(-1.5,0,1.01), v(-1.5,-2,0.99), v(-1.5,-4.3,0.97),
                                v(-2.5,1,0.64), v(-2.5,0,0.51), v(-2.5,-2,0.49), v(-2.5,-4.3,0.47))
								   	 
        cubic_bezier_quad(3*lod,1,v(-2.5,1,1.14), v(-2.5,0,1.01), v(-2.5,-2,0.99), v(-2.5,-4.3,0.97),
		                        v(-1.5,3.5,1.64), v(-1.5,0,1.51), v(-1.5,-2,1.49), v(-1.5,-4.3,1.47),
		                        v(1.5,3.5,1.64), v(1.5,0,1.51), v(1.5,-2,1.49), v(1.5,-4.3,1.47),
								v(2.5,1,1.14), v(2.5,0,1.01), v(2.5,-2,0.99), v(2.5,-4.3,0.97))
		
		cubic_bezier_quad(1,3*lod,v(-2.5,1,1.14), v(-1.5,3.5,1.64), v(1.5,3.5,1.64), v(2.5,1,1.14),
								v(-2.5,1,0.972), v(-1.5,3.5,1.472), v(1.5,3.5,1.472), v(2.5,1,0.972),
								v(-2.5,1,0.806), v(-1.5,3.5,1.306), v(1.5,3.5,1.306), v(2.5,1,0.806),
								v(-2.5,1,0.64), v(-1.5,3.5,1.14), v(1.5,3.5,1.14), v(2.5,1,0.64))
			
		xref_quad(v(2.5,1,1.14), v(2.5,-4.3,0.97), v(2.5,-4.3,0.47), v(2.5,1,0.64))

		extrusion(v(0,-3.9,0.8), v(0,-4.3,-2.3), v(0,1,0), 1, v(-2.5,0,0), v(2.5,0,0), v(2.5,0.6,0), v(-2.5,0.6,0))		
		extrusion(v(-2.2,-4.3,0.8), v(-2.2,-4.3,-2.4), v(0,1,0), 1, v(-0.3,0,0), v(0.3,0,0), v(0.3,2,0), v(-0.3,2,0))
		extrusion(v(2.2,-4.3,0.8), v(2.2,-4.3,-2.4), v(0,1,0), 1, v(-0.3,0,0), v(0.3,0,0), v(0.3,2,0), v(-0.3,2,0))

  		set_material('black', 0,0,0,1,.3,.3,.35,5)
		use_material('black')

		sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.1,-3.6,-2.8)) * Matrix.scale(v(0.35,-2.2,0.45))) -- rl_leg
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.1,-3.6,-2.8)) * Matrix.scale(v(0.35,-2.2,0.45))) -- ll_leg

		xref_cylinder(3*lod, v(0.8,-2.7,-2.3), v(0.7,-1.5,-2.3), v(0,0,1), 0.2)
  		cylinder(3*lod, v(0.85,-2.5,-2.3), v(-0.85,-2.5,-2.3), v(0,0,1), 0.19)
  		cylinder(3*lod, v(0,-2.5,-2.3), v(0,-2.8,-5), v(0,1,0.3), 0.15)

	end,
	
	dynamic = function(lod)

--local select1 = 200
		selector1()
		if select1 < 201 then
	        set_material('dress1', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 401 then
               	set_material('dress1', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 601 then
					set_material('dress1', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 < 801 then
           				set_material('dress1', .06,.35,0,.99,.3,.3,.3,5)
           			else
           				if select1 > 800 then
           					 set_material('dress1', .2,0,.35,1,.3,.3,.3,5)
           				end
					end
				end
			end
		end

	
--local select3 = 10
        selector3()
        if select3 < 11 then
        	set_material('dress2', .6, .6, .6, 1, .3, .3, .3, 5)
        else
            if select3 < 21 then
                set_material('dress2', .65, .25, .02, 1, .3, .3, .3, 5)
			else
			    if select3 < 31 then
			        set_material('dress2', .0, .3, .6, 1, .3, .3, .3, 5)
				else
				    if select3 < 41 then
				        set_material('dress2', .5, .10, .6, 1, .4, .4, .4, 5)
					else
						if select3 < 51 then
							set_material('dress2', .02, 0, .04, 1, .2, .2, .2, 5)
						else
							if select3 < 61 then
        						set_material('dress2', .6, .6, .6, 1, .3, .3, .3, 5)
        					else
            					if select3 < 71 then
                					set_material('dress2', .65, .25, .02, 1, .3, .3, .3, 5)
								else
			    					if select3 < 81 then
			        					set_material('dress2', .0, .3, .6, 1, .3, .3, .3, 5)
									else
				    					if select3 < 91 then
				        					set_material('dress2', .5, .10, .6, 1, .4, .4, .4, 5)
										else
											if select3 > 90 then
												set_material('dress2', .02, 0, .04, 1, .2, .2, .2, 5)
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end

--local select4 = 4
		selector4()
		if select4 < 26 then
		    use_material('dress2')
			sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
		end
        if select4 > 50 then
    		if select4 < 76 then
		    	use_material('dress2')
				sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            	sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
			end
		end

		call_model('head1', v(0,1.9,0), v(1,0,0), v(0,1,0), 1)

	end
})

define_model('pilot2', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 3,
			materials = {'dress1', 'dress2', 'grey', 'black'},
			},

	static = function(lod)

		use_material('dress1')
  		sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.scale(v(2,0.8,1))) -- shoulder
        sphere_slice(3*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(1.6,-0.3,0)) * Matrix.rotate(-0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- r_axle
	    sphere_slice(3*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(-1.6,-0.3,0)) * Matrix.rotate(0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- l_axle
        extrusion(v(0,-2.4,-0.7), v(0,-2.4,-0.9), v(0,1,0), 1, v(-0.25,-0.25,0), v(0.25,-0.25,0), v(0.5,-0.05,0), v(0.5,0.3,0), v(0.25,0.5,0), v(-0.25,0.5,0), v(-0.5,0.3,0.05), v(-0.5,-0.05,0))
        sphere(.5*lod, Matrix.translate(v(0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,-1,0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- r_hand
        sphere(.5*lod, Matrix.translate(v(-0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,1,-0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- l_hand

	    use_material('dress2')
	    ring(3*lod, v(0,0.6,0), v(0,0.9,0), v(0,0,1), 0.61) -- neck
	    sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.scale(v(2,-4,1)))  -- top
		sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,0.4,0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- ru_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,-0.4,-0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- lu_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,-0.6,-1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,0.6,1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,-0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- ru_leg
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- lu_leg

        set_material('grey', .1,.1,.1,1,.2,.2,.2,5)
		use_material('grey')

		cubic_bezier_quad(3*lod,1,v(2.5,1,0.64), v(2.5,0,0.51), v(2.5,-2,0.49), v(2.5,-4.3,0.47),
                                v(1.5,3.5,1.14), v(1.5,0,1.01), v(1.5,-2,0.99), v(1.5,-4.3,0.97),
                                v(-1.5,3.5,1.14), v(-1.5,0,1.01), v(-1.5,-2,0.99), v(-1.5,-4.3,0.97),
                                v(-2.5,1,0.64), v(-2.5,0,0.51), v(-2.5,-2,0.49), v(-2.5,-4.3,0.47))

        cubic_bezier_quad(3*lod,1,v(-2.5,1,1.14), v(-2.5,0,1.01), v(-2.5,-2,0.99), v(-2.5,-4.3,0.97),
		                        v(-1.5,3.5,1.64), v(-1.5,0,1.51), v(-1.5,-2,1.49), v(-1.5,-4.3,1.47),
		                        v(1.5,3.5,1.64), v(1.5,0,1.51), v(1.5,-2,1.49), v(1.5,-4.3,1.47),
								v(2.5,1,1.14), v(2.5,0,1.01), v(2.5,-2,0.99), v(2.5,-4.3,0.97))

		cubic_bezier_quad(1,3*lod,v(-2.5,1,1.14), v(-1.5,3.5,1.64), v(1.5,3.5,1.64), v(2.5,1,1.14),
								v(-2.5,1,0.972), v(-1.5,3.5,1.472), v(1.5,3.5,1.472), v(2.5,1,0.972),
								v(-2.5,1,0.806), v(-1.5,3.5,1.306), v(1.5,3.5,1.306), v(2.5,1,0.806),
								v(-2.5,1,0.64), v(-1.5,3.5,1.14), v(1.5,3.5,1.14), v(2.5,1,0.64))

		xref_quad(v(2.5,1,1.14), v(2.5,-4.3,0.97), v(2.5,-4.3,0.47), v(2.5,1,0.64))

		extrusion(v(0,-3.9,0.8), v(0,-4.3,-2.3), v(0,1,0), 1, v(-2.5,0,0), v(2.5,0,0), v(2.5,0.6,0), v(-2.5,0.6,0))
		extrusion(v(-2.2,-4.3,0.8), v(-2.2,-4.3,-2.4), v(0,1,0), 1, v(-0.3,0,0), v(0.3,0,0), v(0.3,2,0), v(-0.3,2,0))
		extrusion(v(2.2,-4.3,0.8), v(2.2,-4.3,-2.4), v(0,1,0), 1, v(-0.3,0,0), v(0.3,0,0), v(0.3,2,0), v(-0.3,2,0))

  		set_material('black', 0,0,0,1,.3,.3,.35,5)
		use_material('black')

		xref_cylinder(3*lod, v(0.8,-2.7,-2.3), v(0.7,-1.5,-2.3), v(0,0,1), 0.2)
  		cylinder(3*lod, v(0.85,-2.5,-2.3), v(-0.85,-2.5,-2.3), v(0,0,1), 0.19)
  		cylinder(3*lod, v(0,-2.5,-2.3), v(0,-2.8,-5), v(0,1,0.3), 0.15)

	end,

	dynamic = function(lod)

--local select1 = 200
		selector1()
		if select1 < 201 then
	        set_material('dress1', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 401 then
               	set_material('dress1', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 601 then
					set_material('dress1', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 < 801 then
           				set_material('dress1', .06,.35,0,.99,.3,.3,.3,5)
           			else
           				if select1 > 800 then
           					 set_material('dress1', .2,0,.35,1,.3,.3,.3,5)
           				end
					end
				end
			end
		end


--local select3 = 10
        selector3()
        if select3 < 11 then
        	set_material('dress2', .6, .6, .6, 1, .3, .3, .3, 5)
        else
            if select3 < 21 then
                set_material('dress2', .65, .25, .02, 1, .3, .3, .3, 5)
			else
			    if select3 < 31 then
			        set_material('dress2', .0, .3, .6, 1, .3, .3, .3, 5)
				else
				    if select3 < 41 then
				        set_material('dress2', .5, .10, .6, 1, .4, .4, .4, 5)
					else
						if select3 < 51 then
							set_material('dress2', .02, 0, .04, 1, .2, .2, .2, 5)
						else
							if select3 < 61 then
        						set_material('dress2', .6, .6, .6, 1, .3, .3, .3, 5)
        					else
            					if select3 < 71 then
                					set_material('dress2', .65, .25, .02, 1, .3, .3, .3, 5)
								else
			    					if select3 < 81 then
			        					set_material('dress2', .0, .3, .6, 1, .3, .3, .3, 5)
									else
				    					if select3 < 91 then
				        					set_material('dress2', .5, .10, .6, 1, .4, .4, .4, 5)
										else
											if select3 > 90 then
												set_material('dress2', .02, 0, .04, 1, .2, .2, .2, 5)
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end

--local select4 = 4
		selector4()
		if select4 < 26 then
		    use_material('dress2')
			sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
		end
        if select4 > 50 then
    		if select4 < 76 then
		    	use_material('dress2')
				sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            	sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
			end
		end

		call_model('head1', v(0,1.9,0), v(1,0,0), v(0,1,0), 1)

	end
})

define_model('pilot_3_m_helmet', {
	info = {
 			lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'glass', 'radio'},
			},

	static = function(lod)
     	set_material('glass', 0,0,.05,.4,1,1,1.5,100)
     	set_material('radio', .3,.32,.35,1,.5,.55,.6,5)

		use_material('radio')
		xref_cylinder(3*lod,v(1,0,0),v(1.05,0,0),v(0,1,0),.2)
		xref_tapered_cylinder(3*lod,v(1.025,.2,0),v(1.025,.7,0),v(1,0,0),.02,.01)
		tube(3*lod, v(0,-1.1,0),v(0,-.7,0),v(1,0,0),.58,.6)

		use_material('glass')
		sphere(2)
	end
})

define_model('pilot_3_m', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'face', 'head', 'hands', 'body', 'black', 'sign'},
			},

	static = function(lod)
	    set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
		set_material('black',.12,.12,.1,1,.2,.2,.2,10)
        set_material('face', .45,.43,.4,1,.4,.4,.4,5)
     	set_material('head', .25,.22,.2,1,.3,.3,.3,5)

        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
        set_light(2, 0.05, v(0,3,3), v(2,2,2))
        
        set_local_lighting(true)
        use_light(1)
        use_light(2)
        
		use_material('black')
		load_obj('chair_1.obj')

		texture('head3_m_b.png')
		use_material('head')
		load_obj('pilot3_m_head_b.obj')

     	texture('head3_m_b.png')
		use_material('head')
		load_obj('pilot3_m_head_b.obj')

		texture('pilot3_m.png')
  		use_material('body')
		load_obj('pilot3_m_body.obj')
		
		use_material('hands')
		load_obj('pilot3_m_hands.obj')
		
		use_material('feet')
		load_obj('pilot3_m_feet.obj')

        texture('sign.png')
		use_material('sign')
        zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('pilot3_m_sign.obj')
		zbias(0)
  	end,
	
	dynamic = function(lod)
     	use_light(1)
     	use_light(2)
     	
		selector2()
  		if select2 < 34 then
		    texture('sub_models/pilot1/head3_m_f.png')
		else
		    if select2 < 67 then
		        texture('sub_models/pilot1/head3_m_f_2.png')
		    else
		        if select2 > 66 then
		            texture('sub_models/pilot1/head3_m_f_3.png')
		        end
			end
		end
        use_material('face')
		load_obj('sub_models/pilot1/pilot3_m_head_f.obj')
	    set_local_lighting(false)
	    
		selector3()
        if select3 < 11 then
        	set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
        else
            if select3 < 21 then
                set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
			else
			    if select3 < 31 then
			        set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
				else
				    if select3 < 41 then
				        set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
					else
						if select3 < 51 then
							set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
						else
							if select3 < 61 then
        						set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
        					else
            					if select3 < 71 then
                					set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
								else
			    					if select3 < 81 then
			        					set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
									else
				    					if select3 < 91 then
				        					set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
										else
											if select3 > 90 then
												set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end
		
		selector1()
		if select1 < 201 then
	        set_material('sign', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('sign', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('sign', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('sign', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('sign', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end
	
	end
})

define_model('pilot_4_m', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'face', 'head', 'hands', 'body', 'black', 'sign'},
			},

	static = function(lod)
     	set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
		set_material('black',.12,.12,.1,1,.2,.2,.2,10)
        set_material('face', .45,.43,.4,1,.4,.4,.4,5)
     	set_material('head', .25,.22,.2,1,.3,.3,.3,5)

        set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
        set_light(2, 0.05, v(0,3,3), v(2,2,2))

        set_local_lighting(true)
        use_light(1)
        use_light(2)

        use_material('black')
		load_obj('chair_1.obj')
        
     	texture('head3_m_b.png')
		use_material('head')
		load_obj('pilot3_m_head_b.obj')

		texture('pilot3_m.png')
  		use_material('body')
		load_obj('pilot3_m_body.obj')

		use_material('hands')
		load_obj('pilot3_m_hands.obj')

		use_material('feet')
		load_obj('pilot3_m_feet.obj')

        texture('sign.png')
		use_material('sign')
        zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('pilot3_m_sign.obj')
		zbias(0)

	end,

	dynamic = function(lod)
        use_light(1)
        use_light(2)
        
  		selector2()
  		if select2 < 34 then
		    texture('sub_models/pilot1/head3_m_f_2.png')
		else
		    if select2 < 67 then
		        texture('sub_models/pilot1/head3_m_f_3.png')
		    else
		        if select2 > 66 then
		            texture('sub_models/pilot1/head3_m_f.png')
		        end
			end
		end
        use_material('face')
		load_obj('sub_models/pilot1/pilot3_m_head_f.obj')
		set_local_lighting(false)
        
        --texture(nil)
        --call_model('pilot_3_m_helmet',v(0,.85,.13),v(1,0,0),v(0,1,.2),.18)
        
        
		selector3()
        if select3 < 11 then
        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
        else
            if select3 < 21 then
                set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			else
			    if select3 < 31 then
			        set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
				else
				    if select3 < 41 then
				        set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
					else
						if select3 < 51 then
							set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
						else
							if select3 < 61 then
        						set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
        					else
            					if select3 < 71 then
                					set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
								else
			    					if select3 < 81 then
			        					set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
									else
				    					if select3 < 91 then
				        					set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
										else
											if select3 > 90 then
												set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end

		selector1()
		if select1 < 201 then
	        set_material('sign', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('sign', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('sign', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('sign', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('sign', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end

	end
})

define_model('pilot_5_m_glob', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'face', 'head', 'hands', 'body', 'black', 'sign'},
			},

	static = function(lod)
     	set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
		set_material('black',.12,.12,.1,1,.2,.2,.2,10)
        set_material('face', .45,.43,.4,1,.4,.4,.4,5)
     	set_material('head', .25,.22,.2,1,.3,.3,.3,5)

        --set_light(1, 0.05, v(0,1.5,-3), v(2,2,2))
        --set_light(2, 0.05, v(0,3,3), v(2,2,2))

        --set_local_lighting(true)
        --use_light(1)
        --use_light(2)

        use_material('black')
		load_obj('chair_1.obj')
        
     	texture('head3_m_b.png')
		use_material('head')
		load_obj('pilot3_m_head_b.obj')

		texture('pilot3_m.png')
  		use_material('body')
		load_obj('pilot3_m_body.obj')

		use_material('hands')
		load_obj('pilot3_m_hands.obj')

		use_material('feet')
		load_obj('pilot3_m_feet.obj')

        texture('sign.png')
		use_material('sign')
        zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('pilot3_m_sign.obj')
		zbias(0)

	end,

	dynamic = function(lod)
        --use_light(1)
        --use_light(2)
        
  		selector2()
  		if select2 < 34 then
		    texture('sub_models/pilot1/head3_m_f_3.png')
		else
		    if select2 < 67 then
		        texture('sub_models/pilot1/head3_m_f.png')
		    else
		        if select2 > 66 then
		            texture('sub_models/pilot1/head3_m_f_2.png')
		        end
			end
		end
        use_material('face')
		load_obj('sub_models/pilot1/pilot3_m_head_f.obj')
		--set_local_lighting(false)
        
        texture(nil)
        call_model('pilot_3_m_helmet',v(0,.85,.15),v(1,0,0),v(0,1,.2),.18)
        
        
		selector3()
        if select3 < 11 then
        	set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
        else
            if select3 < 21 then
                set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
			else
			    if select3 < 31 then
			        set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
				else
				    if select3 < 41 then
				        set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
					else
						if select3 < 51 then
							set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
						else
							if select3 < 61 then
        						set_material('body', .65, .25, .02, 1, .3, .3, .3, 5)
        					else
            					if select3 < 71 then
                					set_material('body', .0, .3, .6, 1, .3, .3, .3, 5)
								else
			    					if select3 < 81 then
			        					set_material('body', .5, .10, .6, 1, .4, .4, .4, 5)
									else
				    					if select3 < 91 then
				        					set_material('body', .02, 0, .04, 1, .2, .2, .2, 5)
										else
											if select3 > 90 then
												set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
											end
										end
									end
								end
							end
						end
					end
				end
			end
		end

		selector1()
		if select1 < 201 then
	        set_material('sign', .5,0,0,.99,.2,.2,.2,10)
	    else
			if select1 < 401 then
               	set_material('sign', .45,.35,.01,.99,.2,.2,.2,10)
			else
    		    if select1 < 601 then
					set_material('sign', 0,.15,.7,.99,.2,.2,.2,10)
   				else
   				    if select1 < 801 then
						set_material('sign', .06,.35,0,.99,.2,.2,.2,10)
					else
						if select1 > 800 then
							set_material('sign', .2,0,.35,.99,.2,.2,.2,10)
						end
					end
				end
			end
		end

	end
})

define_model('pilot_6_m_helmet', {
	info = {
 			lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'broken', 'glass', 'radio'},
			},

	static = function(lod)
     	set_material('broken', .1,.1,.2,.99999,1,1,1.5,100)
        set_material('glass', 0,0,.05,.4,1,1,1.5,100)
		set_material('radio', .3,.32,.35,1,.5,.55,.6,5)

		use_material('radio')
		xref_cylinder(3*lod,v(1,0,0),v(1.05,0,0),v(0,1,0),.2)
		xref_tapered_cylinder(3*lod,v(1.025,.2,0),v(1.025,.7,0),v(1,0,0),.02,.01)
		tube(3*lod, v(0,-1.1,0),v(0,-.7,0),v(1,0,0),.58,.6)

		texture('broken.png')
		use_material('broken')
		sphere(1)
        use_material('glass')
		texture(nil)
		zbias(2,v(0,0,0),v(0,0,0))
		use_material('glass')
		sphere(2)
		zbias(0)
	end
})

define_model('pilot_6_dead', {
	info = 	{
            lod_pixels = {2, 5, 10, 0},
			bounding_radius = 2,
			materials = {'feet', 'face', 'head', 'hands', 'body', 'black', 'sign'},
			},

	static = function(lod)
     	set_material('feet', .2,.2,.2,1,.2,.2,.2,5)
		set_material('hands', .48,.32,.2,1,.3,.3,.3,5)
		set_material('black',.12,.12,.1,1,.2,.2,.2,10)
        set_material('face', .45,.43,.4,1,.4,.4,.4,5)
     	set_material('head', .25,.22,.2,1,.3,.3,.3,5)
        set_material('body', .6, .6, .6, 1, .3, .3, .3, 5)
        set_material('sign', .5,0,0,.99,.2,.2,.2,10)
        
     	texture('head3_m_b.png')
		use_material('head')
		load_obj('pilot3_m_head_b.obj')

		texture('pilot3_m.png')
  		use_material('body')
		load_obj('pilot3_m_body.obj')

		use_material('hands')
		load_obj('pilot3_m_hands.obj')

		use_material('feet')
		load_obj('pilot3_m_feet.obj')

        texture('sign.png')
		use_material('sign')
        zbias(1,v(0,0,0),v(0,0,-1))
		load_obj('pilot3_m_sign.obj')
		zbias(0)

        texture('head3_m_f.png')
        use_material('face')
		load_obj('pilot3_m_head_f.obj')


        call_model('pilot_6_m_helmet',v(0,.85,.15),v(1,0,0),v(0,1,.2),.18)
	end
})
