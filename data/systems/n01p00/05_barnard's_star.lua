local s = CustomSystem:new('Barnard\'s star',{ Body.Type.STAR_M })

s:govtype(Polit.GovType.EARTHCOLONIAL)
s:short_desc('Earth Federation Colonial Rule')
s:long_desc([[Federation Prison Camps.]])

s:add_to_sector(-1,0,v(0.877,0.131,0.186))
