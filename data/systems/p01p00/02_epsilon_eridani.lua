local s = CustomSystem:new("Epsilon Eridani")

s:type({ TYPE_STAR_K })
s:sector(1,0)
s:pos(v(0.351,0.735,-0.999))
s:seed(5)
s:govtype(Polit.GovType.CISLIBDEM)
s:short_desc('First off-earth colony. Industrial world with indigenous life.')
s:long_desc([[Epsilon Eridani was the first star system beyond Sol to be colonised by humanity. The New Hope colony on the life-bearing planet of the same name was founded in 2279. Its 1520 initial inhabitants completed their pre-hyperspace voyage of 10.7 lightyears from Sol in just under 25 years.
Mass emigration from Earth in the 27th century drove a population explosion and today Epsilon Eridani counts itself among the most populous of inhabited systems.
The system's history has been marked by political friction between Epsilon Eridani and the Earth government. This began with the advent of hyperspace around the end of the 26th century. While previously the communications lag of 20 years had prevented exertion of Earth's power, suddenly the rulers of Epsilon Eridani found themselves constantly subject to the interference of Earth.
This conflict flared up in 2714 when the pro-Earth president of Epsilon Eridani was toppled amid strikes and civil disorder over the unfair tax and trade conditions imposed by Earth. The 'Free Republic' then established survived nine months until Earth rule was re-imposed by force, including the notorious use of orbital lasers on population centres.
Independence was not finally won until the wars of the 30th century, and the formation of the Confederation of Independent Worlds, of which Epsilon Eridani was a founding member.
Epsilon Eridani is today a thriving centre of industry, cutting-edge technology and tourism.
Reproduced with the kind permission of Enrique Watson, New Hope University, 2992]])

s.add_to_universe()
