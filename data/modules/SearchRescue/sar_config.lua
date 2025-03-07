-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- This module defines basic "Search and Rescue" module config options.

local sar_config = {}

-- basic variables for mission creation
sar_config.max_mission_dist = 30          -- max distance for long distance mission target location [ly]
sar_config.max_close_dist = 5000          -- max distance for "CLOSE_PLANET" target location [km]
sar_config.max_close_space_dist = 10000   -- max distance for "CLOSE_SPACE" target location [km]
sar_config.far_space_orbit_dist = 3.5     -- orbital distance around planet for "FAR_SPACE" target location (number of planet radii)
sar_config.min_interaction_dist = 50      -- min distance for successful interaction with target [meters]
sar_config.target_interaction_time = 10   -- target interaction time to load/unload one unit of cargo/person [sec]
sar_config.max_pass = 20                  -- max number of passengers on target ship
-- sar_config.max_crew = 8                   -- max number of crew on target ship (high max: 8)
sar_config.reward_close = 200             -- basic reward for "CLOSE" mission (+/- random half of that)
sar_config.reward_medium = 1000           -- basic reward for "MEDIUM" mission (+/- random half of that)
sar_config.reward_far = 2000              -- basic reward for "FAR" mission (+/- random half of that)
sar_config.ad_freq_max = 3.0              -- maximum frequency for ad creation
sar_config.ad_freq_min = 0.3              -- minimum frequency for ad creation

return sar_config
