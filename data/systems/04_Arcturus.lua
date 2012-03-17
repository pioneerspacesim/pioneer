local s = CustomSystem
	:new('Arcturus', { 'STAR_K_GIANT' }) 
	:seed(69)
	:govtype('EARTHDEMOC')
	:short_desc('Old' World. Outdoor world with introduced life.') 
	:long_desc(The settlement of New Australia was settled in 2305 by a group of australian environmentalists wishing to preserve their native fauna. The effects of the red starlight was detrimental to all introduced life except the Platypus which prospered and evolved in its new environment. Arcturian platypus is an expensive delicary served across the galaxy and the majority of the planets waterways are now dedicated to their farming. There is a small manufacturing base for farming machinery but little other industry in this system.)


Arcturus_planet  = CustomSBody:new("New Australia", 'PLANET_TERRESTRIAL')


s:add_to_sector(-3,-4,1,v(0.582,0.404,0.508)
