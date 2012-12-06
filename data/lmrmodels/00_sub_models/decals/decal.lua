-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--[[ sub-model for use in all models to place (cutout) decals on them,
	 texture can have a alpha channel for a cutout decal but it's not a must
	 if sequence is expandet so all squadrons can use all decals
]]--

define_model('decal', {
	info =	{
			bounding_radius = 1,
			materials = {'decal'}
			},

	static = function(lod)

	end,

	dynamic = function(lod)

        selector1()  -- requests "random" number from selector function
		set_material('decal', .7,.7,.7, 0.9, .3,.3,.3,5) -- creates semi-transparent material to cut out alpha channel, has to be < 1 but > 0
		use_material('decal')

  		if select1 < 26 then
	    	texture('lmrmodels/00_sub_models/decals/decal_a.png', v(0,0,0), v(0,0,-1), v(0,-1,0)) -- calls the texture for the decal,
      	else
     		if select1 < 51 then
				texture('lmrmodels/00_sub_models/decals/decal_b.png', v(0,0,0), v(0,0,-1), v(0,-1,0)) -- if sequence could be expandet to 1000!(limited by "select1or" function) different decals, but 10 will be ok i guess ;)
  			else
   				if select1 < 76 then
					texture('lmrmodels/00_sub_models/decals/decal_c.png', v(0,0,0), v(0,0,-1), v(0,-1,0)) -- when adding a additional one, divide 1000 through no. of total decals
 				else
   					if select1 < 101 then
						texture('lmrmodels/00_sub_models/decals/decal_d.png', v(0,0,0), v(0,0,-1), v(0,-1,0)) -- feel free to exchange the existing ones with your own style
  					else
   						if select1 < 126 then
		    				texture('lmrmodels/00_sub_models/decals/decal_e.png', v(0,0,0), v(0,0,-1), v(0,-1,0)) -- accepts any quadratical sized texture, recommendet are 100x100 - 200x200
						else
            				if select1 < 151 then
								texture('lmrmodels/00_sub_models/decals/decal_f.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                    else
            					if select1 < 176 then
            						texture('lmrmodels/00_sub_models/decals/decal_g.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
                              	else
            						if select1 < 201 then
		 								texture('lmrmodels/00_sub_models/decals/decal_h.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                           	else
           								if select1 < 226 then
											texture('lmrmodels/00_sub_models/decals/decal_i.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                           		else
            								if select1 < 251 then
												texture('lmrmodels/00_sub_models/decals/decal_j.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
       										else
   												if select1 < 276 then
													texture('lmrmodels/00_sub_models/decals/decal_a.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                       					else
            										if select1 < 301 then
														texture('lmrmodels/00_sub_models/decals/decal_b.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                                            else
            											if select1 < 326 then
															texture('lmrmodels/00_sub_models/decals/decal_c.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                                                else
           													if select1 < 351 then
		    													texture('lmrmodels/00_sub_models/decals/decal_d.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  															else
            													if select1 < 376 then
																	texture('lmrmodels/00_sub_models/decals/decal_e.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		    													else
   																	if select1 < 401 then
																		texture('lmrmodels/00_sub_models/decals/decal_f.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		    														else
            															if select1 < 426 then
																			texture('lmrmodels/00_sub_models/decals/decal_g.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
        																else
            																if select1 < 451 then
																				texture('lmrmodels/00_sub_models/decals/decal_h.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																			else
            																	if select1 < 476 then
																					texture('lmrmodels/00_sub_models/decals/decal_i.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		    																	else
            																		if select1 < 501 then
																						texture('lmrmodels/00_sub_models/decals/decal_j.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
        																			else
            																			if select1 < 526 then
																							texture('lmrmodels/00_sub_models/decals/decal_a.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																						else
            																				if select1 < 551 then
																								texture('lmrmodels/00_sub_models/decals/decal_b.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																							else
            																					if select1 < 576 then
																									texture('lmrmodels/00_sub_models/decals/decal_c.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																								else
            																						if select1 < 601 then
																										texture('lmrmodels/00_sub_models/decals/decal_d.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																									else
            																							if select1 < 626 then
																											texture('lmrmodels/00_sub_models/decals/decal_e.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																										else
            																								if select1 < 651 then
																												texture('lmrmodels/00_sub_models/decals/decal_f.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
 																											else
            																									if select1 < 676 then
																													texture('lmrmodels/00_sub_models/decals/decal_g.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
        																										else
            																										if select1 < 701 then
																														texture('lmrmodels/00_sub_models/decals/decal_h.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		                                                                                                            else
            																											if select1 < 726 then
																															texture('lmrmodels/00_sub_models/decals/decal_i.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																														else
            																												if select1 < 751 then
			    																												texture('lmrmodels/00_sub_models/decals/decal_j.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																															else
            																													if select1 < 776 then
																																	texture('lmrmodels/00_sub_models/decals/decal_a.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																else
   																																	if select1 < 801 then
																																		texture('lmrmodels/00_sub_models/decals/decal_b.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																	else
            																															if select1 < 826 then
																																			texture('lmrmodels/00_sub_models/decals/decal_c.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																		else
   																																			if select1 < 851 then
			   																																	texture('lmrmodels/00_sub_models/decals/decal_d.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
		   	                                                                                                   								else
            																																	if select1 < 876 then
			   																																		texture('lmrmodels/00_sub_models/decals/decal_e.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																				else
           																																			if select1 < 901 then
																																						texture('lmrmodels/00_sub_models/decals/decal_f.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																					else
            																																			if select1 < 926 then
																																							texture('lmrmodels/00_sub_models/decals/decal_g.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
        	                                                                                                            								else
   																																							if select1 < 951 then
																																								texture('lmrmodels/00_sub_models/decals/decal_h.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
  																																							else
            																																					if select1 < 976 then
																																									texture('lmrmodels/00_sub_models/decals/decal_i.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
			                        																															else
																																									if select1 > 975 then
																																										texture('lmrmodels/00_sub_models/decals/decal_j.png', v(0,0,0), v(0,0,-1), v(0,-1,0))
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
        zbias(100,v(0,0,0),v(0,0,0))
		quad(v(0,0,0), v(0,1,0), v(0,1,1), v(0,0,1)) -- creates a quadratic shape 1m x 1m for the decal
		zbias(0)
	end
})
