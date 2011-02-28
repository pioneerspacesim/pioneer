local s = CustomSystem:new("DX Cancri")

s:type({ Body.Type.STAR_M })
s:sector(1,0)
s:pos(v(0.701,0.862,0.784))

s:add_to_universe()
