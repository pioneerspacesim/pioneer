local s_diso = CustomSystem:new("Diso",{'STAR_G'})
	:govtype('LIBDEM')
	:short_desc('This planet is mildly noted for its ancient Ouza tulip plantations but ravaged by frequent earthquakes.')
	:long_desc([[This planet is mildly noted for its ancient Ouza tulip plantations but ravaged by frequent earthquakes.]])

local lucy = CustomSBody:new("Lucy",'STAR_G')
   :radius(f(11,10))
   :mass(f(9,10))
   :temp(5750)

local diso = CustomSBody:new('Diso', 'PLANET_TERRESTRIAL')
   :seed(1)
   :radius(f(6155,6378))
   :mass(f(980,1000))
   :temp(280)
   :semi_major_axis(f(90,100))
   :eccentricity(f(2,100))
   :rotation_period(f(9,10))

local diso_station =  CustomSBody:new('Diso High','STARPORT_ORBITAL')
	:semi_major_axis(f(150,100000))
	:rotation_period(f(1,24*60*4))

s_diso:bodies(lucy, {diso, {diso_station}})

local s_lave = CustomSystem:new("Lave",{'STAR_K'})
	:govtype('MILDICT1')
	:short_desc('Lave is most famous for its vast rain forests and the Laveian tree grub.')
	:long_desc([[Lave is most famous for its vast rain forests and the Laveian tree grub.]])

local eshe = CustomSBody:new("Eshe",'STAR_K')
   :radius(f(11,10))
   :mass(f(11,10))
   :temp(4850)

local lave = CustomSBody:new('Lave', 'PLANET_TERRESTRIAL')
   :seed(2)
   :radius(f(4116,6378))
   :mass(f(900,1000))
   :temp(290)
   :semi_major_axis(f(95,100))
   :eccentricity(f(2,100))
   :rotation_period(f(1,1))

local lave_station =  CustomSBody:new('Lave Station','STARPORT_ORBITAL')
	:semi_major_axis(f(4,100000))
	:rotation_period(f(1,24*60*4))

s_lave:bodies(eshe, {lave, {lave_station}})

local s_leesti = CustomSystem:new("Leesti",{'STAR_G'})
	:govtype('CORPORATE')
	:short_desc('The planet Leesti is reasonably fabled for Zero-G cricket and Leestiian evil juice.')
	:long_desc([[The planet Leesti is reasonably fabled for Zero-G cricket and Leestiian evil juice.]])

local lucy = CustomSBody:new("Lucy",'STAR_G')
   :radius(f(11,10))
   :mass(f(11,10))
   :temp(5540)

local leesti = CustomSBody:new('Leesti', 'PLANET_TERRESTRIAL')
   :seed(3)
   :radius(f(3085,6378))
   :mass(f(790,1000))
   :temp(290)
   :semi_major_axis(f(109,100))
   :eccentricity(f(9,1000))
   :rotation_period(f(36,13))

local leesti_station =  CustomSBody:new('Dodec One','STARPORT_ORBITAL')
	:semi_major_axis(f(120,100000))
	:rotation_period(f(1,24*60*2))

s_leesti:bodies(lucy, {leesti, {leesti_station}})

local s_riedquat = CustomSystem:new("Riedquat",{'STAR_G'})
	:govtype('DISORDER')
	:short_desc('This planet is most notable for its fabulous cuisine but beset by occasional civil war')
	:long_desc('This planet is most notable for its fabulous cuisine but beset by occasional civil war')

local riedquat = CustomSBody:new('Riedquat', 'STAR_G')
	:radius(f(12,10))
	:mass(f(12,10))
	:temp(5600)

local tortuga = CustomSBody:new('Tortuga', 'PLANET_TERRESTRIAL')
	:seed(7)
	:radius(f(6403,6378))
	:mass(f(21,20))
	:temp(295)
	:semi_major_axis(f(115,100))
	:eccentricity(f(8,1000))
	:rotation_period(f(25,24))

local hard_harbour = CustomSBody:new('Hard Harbour', 'STARPORT_ORBITAL')
	:semi_major_axis(f(140,100000))
	:rotation_period(f(1,24*30))

s_riedquat:bodies(riedquat, {tortuga, {hard_harbour}})

-- Add them to the map

s_diso:add_to_sector(-3,0,90,v(0.550,0.950,0.930))
s_lave:add_to_sector(-2,1,90,v(0.001,0.001,0.910))
s_leesti:add_to_sector(-3,0,90,v(0.650,0.650,0.960))
s_riedquat:add_to_sector(-3,0,90,v(0.150,0.400,0.950))
