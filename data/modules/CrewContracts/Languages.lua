-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Something for each personality, perhaps
Translate:AddFlavour('English','CrewContracts',{
})

Translate:Add({
	English = {
		['Crew for hire'] = 'Crew for hire',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Potential crew members are registered as seeking employment at {station}:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/wk)',
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
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Make offer of position on ship for stated amount',
		['Suggest new weekly wage of {newAmount}'] = 'Suggest new weekly wage of {newAmount}',
		['Ask candidate to sit a test'] = 'Ask candidate to sit a test',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Thanks, I'll get settled on board immediately.",
		["There doesn't seem to be space for me on board!"] = "There doesn't seem to be space for me on board!",
		["I'm sorry, your offer isn't attractive to me."] = "I'm sorry, your offer isn't attractive to me.",
		["That's extremely generous of you!"] = "That's extremely generous of you!",
		["That certainly makes this offer look better!"] = "That certainly makes this offer look better!",
		["OK, I suppose that's all right."] = "OK, I suppose that's all right.",
		["I'm sorry, I'm not prepared to go any lower."] = "I'm sorry, I'm not prepared to go any lower.",
		crewTestResultsBB = [[Examination results:

General crew competence: {general}%
Engineering and repair: {engineering}%
Piloting and spaceflight: {piloting}%
Navigation and plotting: {navigation}%
Sensors and defence: {sensors}%
Overall exam score: {overall}%]],
	}
})

Translate:Add({
	Polski = {
		['Crew for hire'] = 'Załoga do wynajęcia',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Potencjalni członkowie załogi, zarejestrowani na stacji {station}, jako osoby poszukujące zatrudnienia:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} (${wage}/tydzień)',
		-- Experience levels
		['No experience'] = 'Brak doświadczenia',
		['Simulator training only'] = 'Tylko symulatory treningowe',
		['Some experience in controlled environments'] = 'Ma pewne doświadczenie w kontrolowanych warunkach',
		['Minimal time served aboard ship'] = 'Krótki czas służby na statku',
		['Time served crew member'] = 'Ma doświadczenie w służbie członka załogi',
		['Veteran, time served crew member'] = 'Weteran, ma doświadczenie w służbie członka załogi',
		-- Crew positions
		["Ship's Engineer"] = "Mechanik pokładowy",
		["Pilot"] = "Pilot",
		["Navigator"] = "Nawigator",
		["Sensors and defence"] = "Sensory i obrona",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Załoga do wynajęcia

Nazwisko: {name}
Doświadczenie: {experience}
Oczekiwana stawka: {wage} na tydzień

{response}
]],
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Złóż ofertę zatrudnienia przy wynegocjowanej stawce',
		['Suggest new weekly wage of {newAmount}'] = 'Zaproponuj zmianę stawki tygodniowej na {newAmount}',
		['Ask candidate to sit a test'] = 'Poproś kandydata o przeprowadzenie testu',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Dzięki, stawię się niezwłocznie na pokładzie.",
		["There doesn't seem to be space for me on board!"] = "Nie wydaje mi się, aby było dla mnie miejsce na pokładzie!",
		["I'm sorry, your offer isn't attractive to me."] = "Przepraszam, ale twoja oferta nie jest dla mnie atrakcyjna.",
		["That's extremely generous of you!"] = "Twoja hojność jest niezmiernie wielka!",
		["That certainly makes this offer look better!"] = "To z pewnością uczyni ofertę atrakcyjniejszą!",
		["OK, I suppose that's all right."] = "OK, myślę że stawka jest w porządku.",
		["I'm sorry, I'm not prepared to go any lower."] = "Przepraszam, ale nie zdecyduję się na tak niską stawkę.",
		crewTestResultsBB = [[Wynik kontroli:

Kompetencje członka załogi: {general}%
Inżynieria i naprawy: {engineering}%
Pilotowanie i lot przestrzenny: {piloting}%
Nawigacja i mapy: {navigation}%
Sensory i obrona: {sensors}%
Ogólny wynik egzaminu: {overall}%]],
	}
})
