
define_model('head1', {
	info = 	{
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 2,
			materials = {'face', 'helm', 'glass'},
			},
			
	static = function(lod)
	    
        set_material('glass', .05,.04,.1,.8,1,1,3,100)
		
			    	
	    use_material('helm')
    	sphere_slice(6*lod, 4*lod, 0, 0.5*math.pi, Matrix.new(v(1.1,0,0), v(0,1.1,0), v(0,0,1.2)))
	    sphere_slice(6*lod, 4*lod, 0, 0.5*math.pi, Matrix.new(v(1.1,0,0), v(0,.90,0.3), v(0,-0.3,1.17))* Matrix.new(v(1,0,0), v(0,0,-1), v(0,1,0)))
		circle(6*lod, v(0,0,-.3), v(0,1,0), v(0,0,1), 1.05)
        circle(6*lod, v(0,0,-.3), v(0,-1,0), v(0,0,1), 1.05)

        use_material('face')
	    sphere_slice(6*lod, 4*lod, 0.6*math.pi, 0, Matrix.new(v(.9,0,0), v(0,-1.1,-0.1), v(0,-0.3,1.1)))	
		ring(6*lod, v(0,-0.8,0), v(0,-1.1,0), v(0,0,1), 0.6)

	end,
	
	dynamic = function(lod)

   		local visor = math.clamp(get_arg(0), 0.5, 1)
		use_material('glass')
		cylinder(6*lod, v(0,-1+visor,-0.23), v(0,0,-0.23), v(0,0,1), .93)


--local select1 = 200  -- try this to load model in modelviewer

		selector1()   -- disable this to load model in modelviewer, i have no idea why, other submodels work well with selector call includet :(
		if select1 < 251 then
	        set_material('helm', .5,0,0,1,.7,.7,.7,50)
	    else
			if select1 < 501 then
               	set_material('helm', .45,.35,.01,1,.6,.6,.6,50)
			else
    		    if select1 < 751 then
					set_material('helm', 0,.1,.5,1,.8,.8,.8,50)
   				else
   				    if select1 > 750 then
						set_material('helm', 0.05,.25,0,1,.7,.7,.7,50)
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
            lod_pixels = {5, 10, 20, 0},
			bounding_radius = 3,
			materials = {'dress1', 'dress2', 'grey', 'black'},
			},
			
	static = function(lod)
	
		use_material('dress1')
  		sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.scale(v(2,0.8,1))) -- shoulder
        sphere_slice(3*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(1.6,-0.3,0)) * Matrix.rotate(-0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- r_axle
	    sphere_slice(3*lod, 2*lod, 0, 0.7*math.pi, Matrix.translate(v(-1.6,-0.3,0)) * Matrix.rotate(0.6*math.pi,v(0,-0.3,1)) *  Matrix.scale(v(0.75,0.75,0.6))) -- l_axle
        extrusion(v(0,-2.4,-0.7), v(0,-2.4,-0.9), v(0,1,0), 1, v(-0.25,-0.25,0), v(0.25,-0.25,0), v(0.5,-0.05,0), v(0.5,0.3,0), v(0.25,0.5,0), v(-0.25,0.5,0), v(-0.5,0.3,0.05), v(-0.5,-0.05,0))
        sphere(lod, Matrix.translate(v(0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,-1,0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- r_hand
        sphere(lod, Matrix.translate(v(-0.77,-1.95,-2.46)) * Matrix.rotate(0.25*math.pi,v(-0.5,1,-0.2)) * Matrix.scale(v(0.7,0.12,0.35))) -- l_hand

	    use_material('dress2')
	    ring(4*lod, v(0,0.6,0), v(0,0.9,0), v(0,0,1), 0.61) -- neck
	    sphere_slice(4*lod, 2*lod, 0.5*math.pi, 0, Matrix.scale(v(2,-4,1)))  -- top
		sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,0.4,0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- ru_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.85,-0.25,-0.05)) * Matrix.rotate(0.2*math.pi,v(1,-0.4,-0.3)) * Matrix.scale(v(0.4,-2.2,0.5))) -- lu_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,-0.6,-1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-1.98,-1.81,-1.07)) * Matrix.rotate(0.5*math.pi,v(1.2,0.6,1.4)) * Matrix.scale(v(0.3,-2,0.2))) -- rl_arm
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,-0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- ru_leg
        sphere_slice(3*lod, 2*lod, 0.5*math.pi, 0, Matrix.translate(v(-0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,0.2,0)) * Matrix.scale(v(0.8,-3.5,0.7))) -- lu_leg
        if lod > 2 then
			sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,-0.2,0)) * Matrix.scale(v(0.8,0.5,0.7)))
        	sphere_slice(3*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.6,-3,0.1)) * Matrix.rotate(0.45*math.pi,v(1,0.2,0)) * Matrix.scale(v(0.8,0.5,0.7)))
		end
		
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
		if select1 < 251 then
	        set_material('dress1', .5,0,0,.99,.3,.3,.3,5)
	    else
			if select1 < 501 then
               	set_material('dress1', .45,.35,.01,.99,.3,.3,.3,5)
			else
    		    if select1 < 751 then
					set_material('dress1', 0,.15,.7,.99,.3,.3,.3,5)
   				else
   				    if select1 > 750 then
           				set_material('dress1', .06,.35,0,.99,.3,.3,.3,5)
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
			sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
		end
        if select4 > 50 then
    		if select4 < 76 then
		    	use_material('dress2')
				sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,-0.1)) * Matrix.new(v(1,0,0), v(-0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
            	sphere_slice(4*lod, 2*lod, 0, 0.5*math.pi, Matrix.translate(v(-0.75,-0.6,-0.65)) * Matrix.rotate(0.55*math.pi, v(-1,0,0.1)) * Matrix.new(v(1,0,0), v(0.1,1,0), v(0,-0.7,1)) * Matrix.scale(v(.8,.7,.7)))
			end
		end

		call_model('head1', v(0,1.9,0), v(1,0,0), v(0,1,0), 1)

	end
})