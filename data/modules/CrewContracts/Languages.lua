-- Copyright © 2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")

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
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/tydzień)',
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

Translate:Add({
	Russian = {
		['Crew for hire'] = 'Найм экипажа',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Потенциальные члены экипажа, зарегистрированные в качестве ищущих работу на {station}:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/неделя)',
		-- Experience levels
		['No experience'] = 'Опыт отсутствует',
		['Simulator training only'] = 'Тренировки на симуляторе',
		['Some experience in controlled environments'] = 'Некоторый опыт под наблюдением инструкторов.',
		['Minimal time served aboard ship'] = 'Недолго служил на корабле',
		['Time served crew member'] = 'Служил в составе экипажа корабля',
		['Veteran, time served crew member'] = 'Ветеран, служил в составе экипажа корабля',
		-- Crew positions
		["Ship's Engineer"] = "Корабельный инженер",
		["Pilot"] = "Пилот",
		["Navigator"] = "Навигатор",
		["Sensors and defence"] = "Сенсоры и защита",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Найм экипажа

Имя: {name}
Опыт: {experience}
Желаемая з\п: {wage} в неделю

{response}
]],
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Предложить должность на корабле с указанной зарплатой',
		['Suggest new weekly wage of {newAmount}'] = 'Предложить уровень зарплаты в {newAmount}',
		['Ask candidate to sit a test'] = 'Попросить кандидата пройти тест',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Спасибо, я немедленно поднимаюсь на борт.",
		["There doesn't seem to be space for me on board!"] = "Кажется, для меня нет места на корабле!",
		["I'm sorry, your offer isn't attractive to me."] = "Извините, но ваше предложение меня не заинтересовало.",
		["That's extremely generous of you!"] = "Это очень великодушно с вашей стороны!",
		["That certainly makes this offer look better!"] = "Это, конечно, делает ваше предложение интересней!",
		["OK, I suppose that's all right."] = "ОК, мне кажется, что всё в порядке.",
		["I'm sorry, I'm not prepared to go any lower."] = "Извините, но я не готов к понижению.",
		crewTestResultsBB = [[Экзаменационные результаты:

Общая компетенция экипажа: {general}%
Инженерия и ремонт: {engineering}%
Пилотирование и перелеты: {piloting}%
Навигация и картография: {navigation}%
Сенсоры и защита: {sensors}%
Общая экзаменационная оценка: {overall}%]],
	}
})

Translate:Add({
	Czech = {
		['Crew for hire'] = 'Najmout posádku',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Potencionální členové posádky, hledající práci, registrovaní na stanici {station}:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/wk)',
		-- Experience levels
		['No experience'] = 'Bez zkušeností',
		['Simulator training only'] = 'Pouze trénink na simulátoru',
		['Some experience in controlled environments'] = 'Nějaké zkušenosti v kontrolovaném prostředí',
		['Minimal time served aboard ship'] = 'Krátký čas sloužil(a) na palubě lodi',
		['Time served crew member'] = 'Časem prověřený člen posádky',
		['Veteran, time served crew member'] = 'Veterán, časem prověřený člen posádky',
		-- Crew positions
		["Ship's Engineer"] = "Údržba lodi",
		["Pilot"] = "Pilot",
		["Navigator"] = "Navigátor",
		["Sensors and defence"] = "Senzory a obrana lodi",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Najmout posádku

Jméno: {name}
Zkušenosti: {experience}
Požadovaný plat: {wage} za týden

{response}
]],
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Učinit nabídku pozice na lodi za uvedenou částku',
		['Suggest new weekly wage of {newAmount}'] = 'Navrhnout novou částku týdenní výplaty ve výši {newAmount}',
		['Ask candidate to sit a test'] = 'Zeptat se zájemnce, zda podstoupí test',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Děkuji, ihned se nalodím.",
		["There doesn't seem to be space for me on board!"] = "Vypadá to, že na lodi pro mě není místo!",
		["I'm sorry, your offer isn't attractive to me."] = "Promiňte, ale vaše nabídka mi nepřijde zajímavá.",
		["That's extremely generous of you!"] = "To je od vás velkorysé!",
		["That certainly makes this offer look better!"] = "To jistě učiní vaši nabídku zajímavějsí!",
		["OK, I suppose that's all right."] = "OK, domnívám se, že je to v pořádku.",
		["I'm sorry, I'm not prepared to go any lower."] = "Promiňte, ale níž už nepůjdu.",
		crewTestResultsBB = [[Výsledek testu:

Rozsah působnosti: {general}%
Strojnictví a opravy: {engineering}%
Pilotáž: {piloting}%
Navigace: {navigation}%
Senzory a obrana: {sensors}%
Celkový výsledek testu: {overall}%]],
	}
})

Translate:AddFlavour('Deutsch','CrewContracts',{
})

Translate:Add({
	Deutsch = {
		['Crew for hire'] = 'Suche Anstellung',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Potenzielle Crewmitglieder sind als Arbeitssuchend registriert bei {station}:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/Woche)',
		-- Experience levels
		['No experience'] = 'Keine Erfahrung',
		['Simulator training only'] = 'Nur Simulatortraining',
		['Some experience in controlled environments'] = 'Etwas Erfahrung in kontrollierten Gebieten',
		['Minimal time served aboard ship'] = 'Kurzzeitig angestellt auf Schiffen',
		['Time served crew member'] = 'Langzeitiges Crewmitglied',
		['Veteran, time served crew member'] = 'Veteran, langzeitiges Crewmitflied',
		-- Crew positions
		["Ship's Engineer"] = "Schiffsingenieur",
		["Pilot"] = "Pilot",
		["Navigator"] = "Navigator",
		["Sensors and defence"] = "Sensoren und Abwehr",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Crew for hire

Name: {name}
Erfahrung: {experience}
Lohnforderung: {wage} pro Woche

{response}
]],
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Position auf dem Schiff mit dem festgelegten Lohn anbieten',
		['Suggest new weekly wage of {newAmount}'] = 'Neuen wöchentlichen Lohn von {newAmount} vorschlagen',
		['Ask candidate to sit a test'] = 'Kandidat einen Test absolvieren lassen',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Danke, ich werde mich sofort an Bord begeben.",
		["There doesn't seem to be space for me on board!"] = "Es scheint keinen Platz für mich an Bord zu geben!",
		["I'm sorry, your offer isn't attractive to me."] = "Entschuldigung, das Angebot ist nicht attraktiv für mich.",
		["That's extremely generous of you!"] = "Das ist wirklich extrem großzügig!",
		["That certainly makes this offer look better!"] = "So sieht das Angebot schon etwas besser aus!",
		["OK, I suppose that's all right."] = "OK, ich denke, dass das klar geht.",
		["I'm sorry, I'm not prepared to go any lower."] = "Entschuldigung, Ich kann wirklich nicht mehr tiefer gehen.",
		crewTestResultsBB = [[Examination results:

Generelle Crew-Kompetenzen: {general}%
Technik und Reperatur: {engineering}%
Steuern und Raumflug: {piloting}%
Navigation und Auswertung: {navigation}%
Sensoren und Abwehr: {sensors}%
Insgesamtes Testergebnis: {overall}%]],
	}
})

Translate:Add({
	Spanish = {
		['Crew for hire'] = 'Se ofrece tripulación',
		['Potential crew members are registered as seeking employment at {station}:'] = 'Tripulación en potencia registrada buscando trabajo en {station}:',
		['{potentialCrewMember} ({wage}/wk)'] = '{potentialCrewMember} ({wage}/sem.)',
		-- Experience levels
		['No experience'] = 'Sin experiencia',
		['Simulator training only'] = 'Entrenamiento en simulador solamente',
		['Some experience in controlled environments'] = 'Algo de experiencia en entornos controlados',
		['Minimal time served aboard ship'] = 'Un tiempo mínimo en una nave',
		['Time served crew member'] = 'Experiencia normal como tripulante',
		['Veteran, time served crew member'] = 'Tripulante veterano',
		-- Crew positions
		["Ship's Engineer"] = "Ingeniero de naves",
		["Pilot"] = "Piloto",
		["Navigator"] = "Navegador",
		["Sensors and defence"] = "Detección y defensa",
		-- Details form for a single crew member
		crewDetailSheetBB = [[Se ofrece tripulante

Nombre: {name}
Experiencia: {experience}
Sueldo solicitado: {wage} por semana

{response}
]],
		-- Buttons to click
		['Make offer of position on ship for stated amount'] = 'Ofertar la posición en la nave por el sueldo ofrecido',
		['Suggest new weekly wage of {newAmount}'] = 'Sugerir nuevo sueldo mensual de {newAmount}',
		['Ask candidate to sit a test'] = 'Pedir al candidato pasar un examen',
		-- Responses
		["Thanks, I'll get settled on board immediately."] = "Gracias, iré a bordo inmediatamente.",
		["There doesn't seem to be space for me on board!"] = "¡No parece haber espacio para mí!",
		["I'm sorry, your offer isn't attractive to me."] = "Lo siento, su oferta no me interesa.",
		["That's extremely generous of you!"] = "¡Es usted muy generoso!",
		["That certainly makes this offer look better!"] = "¡Eso hace que esta oferta parezca mejor!",
		["OK, I suppose that's all right."] = "De acuerdo, creo que es adecuado.",
		["I'm sorry, I'm not prepared to go any lower."] = "Lo siento, no estoy preparado para bajar más.",
		crewTestResultsBB = [[Resultado del examen:

Competencia general: {general}%
Ingeniería y reparación: {engineering}%
Pilotaje y vuelo: {piloting}%
Navegación y preparación: {navigation}%
Detección y defensa: {sensors}%
Puntuación total: {overall}%]],
	}
})



