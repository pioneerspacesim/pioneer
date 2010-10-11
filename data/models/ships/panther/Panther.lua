define_model('pantengl', {     
   info = {
         scale = 1.00,
         bounding_radius = 30,
         materials={'courier'},

         },

   static = function(lod)

        --set_material('courier', .13,.13,.13,1,1.26,1.4,1.66,30)


        --use_material('courier')


        texture('pantherleg.png')
        load_obj('panthereng.obj', Matrix.rotate(math.pi,v(0,0,1))) 
        
     end
  })
  
  define_model('pantengr', {     
   info = {
         scale = 1.00,
         bounding_radius = 30,
         materials={'courier'},

         },

   static = function(lod)

        --set_material('courier', .13,.13,.13,1,1.26,1.4,1.66,30)


        --use_material('courier')


        texture('pantherleg.png')
        load_obj('panthereng.obj')
        
     end
  })

 
        define_model('pant', {     
           info = {
                 scale = 1.70,
                 bounding_radius = 30, --??
                 materials={'red', 'blue', 'green', 'dblue', 'lblue', 'yellow', 'text1', 'text2', 'dred', 'light', 'dark'},
                 
                 --tags = { 'ship' },
                 --[[ship_defs = {
                    {
                       'Panther Cliper',
                       { 7*10^6,-87*10^6,4*10^6,-4*10^6,-4*10^6,4*10^6 },
                       1*10^7,
                       {
                       { v(0,-0.5,0), v(0,0,-1) },
                       { v(0,0,0), v(0,0,1) },
                       },
                       { 930, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                       930, 460, 350000,
                       6
                    }
                 }]]
              },


           static = function(lod)

                
           	set_material('red', .83,.2,.2,1,1.26,1.4,1.66,30)
			set_material('dred', .43,.1,.1,1,1.26,1.4,1.66,30)
			set_material('green', .1,.76,.76,1,1.26,1.4,1.66,30)
			set_material('yellow', .1,.73,.1,1,1.26,1.4,1.66,30)
			set_material('dblue', .1,.1,.45,1,1.26,1.4,1.66,30)
			set_material('blue', .1,.1,.63,1,1.26,1.4,1.66,30)
			set_material('lblue', .2,.2,.83,1,1.26,1.4,1.66,30)
			set_material('light', .9,.9,.9,1,1.26,1.4,1.66,30)
			set_material('dark', .1,.1,.1,1,1.26,1.4,1.66,30)
				

				--use_material('dblue')
      
              
              texture('panther.png')
               
                -- 
                --use_material('dark')
               
                load_obj('panther.obj')
                        
                local vMainThruster1 = v(-7,5,15)
                local vMainThruster2 = v(7,5,15)






              thruster(vMainThruster1, v(0,0,1), 20, true)
              thruster(vMainThruster2, v(0,0,1), 20, true)




           end,
           dynamic = function(lod)
           

				

           
           		local reg = get_arg_string(0)
				set_material('text1', .45,.45,.45,1,.1,.1,.1,10)
   				set_material('text2', .55,.55,.1,1,.1,.1,.1,10)
				use_material('text1')
				text(reg, v(0, 5, 19.775), v(0,0.25,1), v(1,0,0), 1.2, {center=true})
				use_material('text2')
				text(reg, v(0, 4.6, -17.85), v(0,0,-1), v(-1,0,0), 1.0, {center=true})
           
			  
			  
              --if get_arg(0) > 0.9 then
                --[[ local v1 = v(-6,3,-6) --TEMPORARY
                 local v2 = v(6,3,-6)  --TEMPORARY
                 local v3 = v(-6,3,12)  --TEMPORARY
                 local v4 = v(6,3,12)]]
                 
                 local v5 = v(-6,(1 + (0.5 - (0.5 * get_arg(0)))),(-6.35 + (6 * (1 - get_arg(0))))) --Moveable Thrusters, move with landing gear which is defined as get_arg(0)
                 local v6 = v(6,(1 + (0.5 - (0.5 * get_arg(0)))),(-6.35 + (6 * (1 - get_arg(0)))))  --
                 local v7 = v(-6,(1 + (0.5 - (0.5 * get_arg(0)))),(12.35 - (6 * (1 - get_arg(0)))))  --
                 local v8 = v(6,(1 + (0.5 - (0.5 * get_arg(0)))),(12.35 - (6 * (1 - get_arg(0)))))

                --[[ zbias(1, v1, v(0,-1,0))
                 call_model('pantengl', v1, v(-1,0,0), v(0,0,1), 1)
                 call_model('pantengr', v2, v(-1,0,0), v(0,0,1), 1)
                 call_model('pantengl', v3, v(-1,0,0), v(0,0,1), 1)
                 call_model('pantengr', v4, v(-1,0,0), v(0,0,(get_arg(0))), 1)
                 zbias(0)]]
				 
				 thruster(v5, v(0,-get_arg(0),(-get_arg(0) + 0.8)), 9)
				 thruster(v6, v(0,-get_arg(0),(-get_arg(0) + 0.8)), 9)
				 
                 thruster(v7, v(0,-get_arg(0),(get_arg(0) - 0.8)), 9)
				 thruster(v8, v(0,-get_arg(0),(get_arg(0) - 0.8)), 9)
             
              --end
              

              
              
             -- if get_arg(0) < 0.9 then
                 
                 local v1 = v(-6,(2.5 + get_arg(0)),-6) --TEMPORARY
                 local v2 = v(6,(2.5 + get_arg(0)),-6)  --TEMPORARY
                 local v3 = v(-6,(2.5 + get_arg(0)),12)  --TEMPORARY
                 local v4 = v(6,(2.5 + get_arg(0)),12)
                 zbias(1, v1, v(0,-1,0))
                 set_material('light', get_arg_material(0))
				 use_material('light')
                 call_model('pantengl', v1, v(-1,0,0), v(0,(1 - (get_arg(0) * 1.2)),(get_arg(0)+0.1)), 1)
                 call_model('pantengr', v2, v(-1,0,0), v(0,(1 - (get_arg(0) * 1.2)),(get_arg(0)+0.1)), 1)
                 call_model('pantengl', v3, v(-1,0,0), v(0,(-1 + (get_arg(0) * 1.2)),(get_arg(0)+0.1)), 1)
                 call_model('pantengr', v4, v(-1,0,0), v(0,(-1 + (get_arg(0) * 1.2)),(get_arg(0)+0.1)), 1)
                 zbias(0)
              
              
              
             -- end
              
              
           end
        })
        
        
        define_model('panther', {     
				 info = {
                 scale = 1.5,
                 bounding_radius = 30, --??
                 materials={'red', 'blue', 'green', 'dblue', 'lblue', 'yellow', 'text1', 'text2', 'dred', 'light', 'dark'},
                 
                 tags = { 'ship' },
                 ship_defs = {
                    {
                       'Panther Clipper',
                       { 28*10^6,-15*10^7,56*10^6,-12*10^6,-12*10^6,12*10^6 },
                       1*10^7,
                       {
                       { v(0,-0.5,0), v(0,0,-1) },
                       { v(0,0,0), v(0,0,1) },
                       },
                       { 930, 1, 2, 3, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 },
                       930, 260, 350000,
                       6
                    }
                 }
              },

   static = function(lod)
   
			--use_material('green')
			--call_model('pant', v(0,0,0), v(1,0,0), v(0,1,0),1)
		    set_material('red', .99,.4,.4,1,1.26,1.4,1.66,30)
			set_material('dred', .5,.2,.2,1,1.26,1.4,1.66,30)
			set_material('green', .3,.99,.99,1,1.26,1.4,1.66,30)
			set_material('yellow', .3,.99,.3,1,1.26,1.4,1.66,30)
			set_material('dblue', .2,.2,.75,1,1.26,1.4,1.66,30)
			set_material('blue', .5,.5,.90,1,1.26,1.4,1.66,30)
			set_material('lblue', .65,.65,1,1,1.26,1.4,1.66,30)
			set_material('light', 1,1,1,1,1.26,1.4,1.66,30)
			set_material('dark', 1,1,1,1,1.26,1.4,1.66,30)
			
        
     end,
     
	dynamic = function(lod)
	    set_material('light', get_arg_material(0))
		--set_material('blue', get_arg_material(2))
		use_material('light')
		call_model('pant', v(0,-5,0), v(1,0,0), v(0,1,0),1)
		use_material('dark')
		--texture('/models/ships/new/panther/pantherdecals.png')

 	end
           


						
					
				
     
  })