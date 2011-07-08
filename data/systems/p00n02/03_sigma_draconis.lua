local s = CustomSystem:new('Sigma Draconis',{ 'STAR_K' })

s:seed(2)
s:govtype('SOCDEM')
s:short_desc('Social democracy')
s:long_desc([[Sigma Draconis (s Dra, s Draconis) is a star 18.8 light-years away from Earth. Its traditional name is Alsafi. It is in the constellation Draco. Its visual magnitude is 4.68. The traditional name 'Alsafi' (also Athafi) supposedly derives from an Arabic word al-athafi 'the cooking tripods'. This refers to the tripods used by nomads for open-air cooking.
The star is a main sequence dwarf of spectral type K0. ]])

s:add_to_sector(0,-2,v(0.914,0.420,0.859))
