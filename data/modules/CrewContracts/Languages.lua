-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Something for each personality, perhaps
Translate:AddFlavour('English','CrewContracts',{
})

Translate:Add({
	English = {
		['Crew for hire'] = 'Crew for hire',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Potential crew members are registered as seeking employment at {station}:',
		['Examine {potentialCrewMember} ({wage}/wk)'] = 'Examine {potentialCrewMember} ({wage}/wk)',
		-- Experience levels
		['No experience'] = 'No experience',
		['Simulator training only'] = 'Simulator training only',
		['Some experience in controlled environments'] = 'Some experience in controlled environments',
		['Minimal time served aboard ship'] = 'Minimal time served aboard ship',
		['Time served crew member'] = 'Time served crew member',
		['Veteran, time served crew member'] = 'Veteran, time served crew member',
		-- Crew positions
		["Ship's Engineer"] = "Ship's Engineer",
		["Pilot"] = "Pilot",
		["Navigator"] = "Navigator",
		["Sensors and defence"] = "Sensors and defence",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Crew for hire

Name: {name}
Experience: {experience}
Asking wage: {wage} per week

{response}
]],
		['Make offer of position on ship for stated amount'] = 'Make offer of position on ship for stated amount',
		['Suggest new weekly wage of {newAmount}'] = 'Suggest new weekly wage of {newAmount}',
		["Thanks, I'll get settled on board immediately."] = "Thanks, I'll get settled on board immediately.",
	}
})
