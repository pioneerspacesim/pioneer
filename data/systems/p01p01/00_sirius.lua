local s = CustomSystem:new('Sirius',{ Body.Type.STAR_A, Body.Type.WHITE_DWARF })

s:seed(5)
s:govtype(Polit.GovType.CORPORATE)
s:short_desc('Corporate system')
s:long_desc([[The Sirius system is home to Sirius Corporation, market leader in Robotics, Neural Computing, Security and Defence Systems, to name but a few of its endeavours. Sirius research and development institutes are at the very cutting edge of galactic science. The young, bright and ambitious from worlds all over galaxy flock to Sirius to make a name for themselves.
Above text all rights reserved Sirius Corporation.]])

s:add_to_sector(1,1,v(0.222,0.273,-0.173))
