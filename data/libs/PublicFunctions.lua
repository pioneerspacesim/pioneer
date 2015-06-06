-- Public Functions

function SpawnPirate(num_pirate_taunts) 
  local pirate_greeting = string.interp(l["PIRATE_TAUNTS_"..Engine.rand:Integer(1,num_pirate_taunts)-1], { client = mission.client.name,})
				  Comms.ImportantMessage(pirate_greeting, ship.label)
end
