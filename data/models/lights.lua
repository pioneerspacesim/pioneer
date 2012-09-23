-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of CC-BY-SA 3.0. See licenses/CC-BY-SA-3.0.txt

--collision lights (don't blink)
--red blinking lights on the left (port) side of the craft
--green blinking lights on the right (starboard) side of the craft
navigation_lights = function(positions_coll, positions_red, positions_green)
	local state = get_flight_state()
	if ((state == 'DOCKING') or (state == 'FLYING' and get_animation_position('WHEEL_STATE') > 0)) then

		billboard('smoke.png', 1.0,  v(1,1,1), positions_coll)

		local lightphase = math.fmod((get_time('SECONDS')*0.75),1)

		if lightphase > .1 and lightphase < .3 then
			billboard('smoke.png', 1.0,  v(1,0,0), positions_red)
		end

		if lightphase > .3 and lightphase < .5 then
			billboard('smoke.png', 1.0,  v(0,1,0), positions_green)
		end
	end
end
