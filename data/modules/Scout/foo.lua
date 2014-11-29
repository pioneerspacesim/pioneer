local Game       = import("Game")
local StarSystem = import("StarSystem")
local Lang       = import("Lang")

local l = Lang.GetResource("core");

--
-- Method: GetNearbyStation by John Bartolomew
--
-- Gets a list of stations in nearby systems that match some criteria.
--
-- Example:
--
-- >  local orbital_ports = Game.system:GetNearbyStationPaths(
-- >      30, nil, function (station) return station.type == 'STARPORT_ORBITAL' end, true)
-- >
-- >  for i = 1, #orbital_ports do
-- >      local path = orbital_ports[i]
-- >      print(path, ' -- ', path:GetSystemBody().name, ' in system ', path:GetStarSystem().name)
-- >  end
--
-- Parameters:
--
--   range_ly        Range limit for nearby systems to search.
--   system_filter   [optional] function, taking a StarSystem object, used to filter systems.
--   station_filter  [optional] function, taking a SystemBody object, used to filter stations.
--   include_local   [optional] if this is true, then stations in the origin system will be included.
--
function StarSystem:GetNearbyStationPaths(range_ly, system_filter, station_filter, include_local)
	local full_system_filter
	if system_filter then
		full_system_filter = function (sys) return (#sys:GetStationPaths() > 0) and system_filter(sys) end
	else
		full_system_filter = function (sys) return (#sys:GetStationPaths() > 0) end
	end
	local nearby_systems = Game.system:GetNearbySystems(range_ly, full_system_filter)

	local function filter_and_add_stations(output_table, sys)
		local station_paths = sys:GetStationPaths()
		for j = 1, #station_paths do
			local station_path = station_paths[j]
			local station = station_path :GetSystemBody()
			if station_filter == nil or station_filter(station) then
				table.insert(output_table, station_path)
			end
		end
	end

	local nearby_stations = {}
	if include_local == true then
		filter_and_add_stations(nearby_stations, self)
	end
	for i = 1, #nearby_systems do
		filter_and_add_stations(nearby_stations, nearby_systems[i])
	end

	return nearby_stations
end
