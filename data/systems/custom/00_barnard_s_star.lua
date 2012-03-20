local s = CustomSystem:new('Barnard\'s star',{ 'STAR_M' })
    :govtype('EARTHCOLONIAL')
    :short_desc('Earth Federation prison colony')
    :long_desc([[Barnard's Star is a very low-mass red dwarf star.  Somewhere between 7 and 12 billion years old, it is probably one of the most ancient stars in the galaxy.  Despite that, it is still fairly active.  Pilots entering the system are warned that there might be consideral stellar activity, including flares and massive coronal ejections.

One of the first stars to be visited after the introduction of interstellar travel, Barnard's Star was found to be solitary, with no planets.  Despite this, habitats were built here to serve as Federal prison colonies.
            
A permit is normally required in order to enter this system whilst carrying weapons.]])

local barnard = CustomSBody:new('Barnard\'s Star', 'STAR_M')
    :radius(f(17,100))
    :mass(f(16,100))
    :temp(3134)

local barnard_starports = {
    CustomSBody:new('High Security Prison Tranquility', 'STARPORT_ORBITAL')
        :semi_major_axis(f(32,10))
        :rotation_period(f(1,24*60*3)),
    CustomSBody:new('High Security Prison Serenity', 'STARPORT_ORBITAL')
        :semi_major_axis(f(32,10))
        :orbital_offset(f(1,3))
        :rotation_period(f(1,24*60*4)),
}
s:bodies(barnard, barnard_starports)
s:add_to_sector(-1,0,0,v(0.260,0.007,0.060))
