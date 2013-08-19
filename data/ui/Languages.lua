-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

Translate:Add({
	English = {
		-- Main menu
		['Start at Earth']    = 'Start at Earth',
		['Start at New Hope'] = 'Start at New Hope',
		['Start at Lave']     = 'Start at Lave',
		['Load game']         = 'Load game',
		['Options']           = 'Options',
		['Quit']              = 'Quit',

		-- Generic file dialog
		['Select file...'] = 'Select file...',
		['Select']         = 'Select',
		['Cancel']         = 'Cancel',

		-- Load dialog
		['Select game to load...'] = 'Select game to load...',

		-- Equipment
		['Equipment'] = 'Equipment',
		['{quantity} Shield Generators'] = '{quantity} Shield Generators',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Occupied Passenger Cabins',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Unoccupied Passenger Cabins',
		
		-- Ship Information
		['Ship Information'] = 'Ship Information',
		['Weight empty:'] = 'Weight empty:',
		['{range} light years ({maxRange} max)'] = '{range} light years ({maxRange} max)',
		['Minimum crew'] = 'Minimum crew',
		['Crew cabins'] = 'Crew cabins',
		['free'] = 'free',
		['max'] = 'max',
		
		-- Personal information
		['Personal Information'] = 'Personal Information',
		['Combat'] = 'Combat',
		['Rating:'] = 'Rating:',
		['Kills:'] = 'Kills:',
		['Military'] = 'Military',
		['Rank:'] = 'Rank:',
		['Male'] = 'Male',
		['Female'] = 'Female',
		['Toggle male/female'] = 'Toggle male/female',
		['Make new face'] = 'Make new face',

		['Commander'] = 'Commander',

		['HARMLESS'] = 'Harmless',
		['MOSTLY_HARMLESS'] = 'Mostly Harmless',
		['POOR'] = 'Poor',
		['AVERAGE'] = 'Average',
		['ABOVE_AVERAGE'] = 'Above Average',
		['COMPETENT'] = 'Competent',
		['DANGEROUS'] = 'Dangerous',
		['DEADLY'] = 'Deadly',
		['ELITE'] = 'ELITE',

		-- Economy & Trade
		['Economy & Trade'] = 'Economy & Trade',
		['Total: '] = 'Total: ',
		['Fuel tank full.'] = 'Fuel tank full.',

		-- Missions
		['Mission Details'] = 'Mission Details',
		['No missions.'] = 'No missions.',
		["%d days left"] = "Days left: %d",
		['INACTIVE'] = 'Inactive', -- Complement of ACTIVE, COMPLETED and FAILED

		-- Crew Tasks
		['Attempt to repair hull'] = 'Attempt to repair hull',
		['Not enough {alloy} to attempt a repair'] = 'Not enough {alloy} to attempt a repair',
		['Hull repaired by {name}, now at {repairPercent}%'] = 'Hull repaired by {name}, now at {repairPercent}%',
		['Hull repair attempt failed. Hull suffered minor damage.'] = 'Hull repair attempt failed. Hull suffered minor damage.',
		['Hull does not require repair.'] = 'Hull does not require repair.',
		['Destroy enemy ship'] = 'Destroy enemy ship',
		['You must request launch clearance first, Commander.'] = 'You must request launch clearance first, Commander.',
		['You must launch first, Commander.'] = 'You must launch first, Commander.',
		['We are in hyperspace, Commander.'] = 'We are in hyperspace, Commander.',
		['The ship is under station control, Commander.'] = 'The ship is under station control, Commander.',
		['You must first select a combat target, Commander.'] = 'You must first select a combat target, Commander.',
		['You must first select a suitable navigation target, Commander.'] = 'You must first select a suitable navigation target, Commander.',
		['There is nobody else on board able to fly this ship.'] = 'There is nobody else on board able to fly this ship.',
		['Pilot seat is now occupied by {name}'] = 'Pilot seat is now occupied by {name}',
		['Dock at current target'] = 'Dock at current target',

		-- Crew Roster
		['Name'] = 'Name',
		['Position'] = 'Position',
		['Wage'] = 'Wage',
		['Owed'] = 'Owed',
		['Next paid'] = 'Next paid',
		['More info...'] = 'More info...',
		['General crew'] = 'General crew',
		['Dismiss'] = 'Dismiss',
		['Qualification scores'] = 'Qualification scores',
		['Engineering:'] = 'Engineering',
		['Piloting:'] = 'Piloting:',
		['Navigation:'] = 'Navigation:',
		['Sensors:'] = 'Sensors:',
		['Employment'] = 'Employment',
		['Negotiate'] = 'Negotiate',
		['Crew Roster'] = 'Crew Roster',
		['Give orders to crew'] = 'Give orders to crew',
		['Total:'] = 'Total:',

		-- Taunts
		["I'm tired of working for nothing. Don't you know what a contract is?"] = "I'm tired of working for nothing. Don't you know what a contract is?",
		["It's been great working for you. If you need me again, I'll be here a while."] = "It's been great working for you. If you need me again, I'll be here a while.",
		["You're going to regret sacking me!"] = "You're going to regret sacking me!",
		["Good riddance to you, too."] = "Good riddance to you, too.",
		
		-- Orbital analysis
		['Orbit'] = 'Orbit',
		['Orbital Analysis'] = 'Orbital Analysis',
		['Located {distance}km from the centre of {name}:'] = 'Located {distance}km from the centre of {name}:',
		['Circular orbit speed:'] = 'Circular orbit speed',
		['Escape speed:'] = 'Escape speed:',
		['Descent-to-ground speed:'] = 'Descent-to-ground speed:',
		['Notes:'] = 'Notes:',
		ORBITAL_ANALYSIS_NOTES = [[
Circular orbit speed is given for a tangential velocity. The ship should be moving in a direction at 90° to the ship/{name} axis.

Descent speed is an absolute minimum, and is also tangential. A slower speed or a lower angle will result in a course which intersects with the surface of {name}.

Escape speed will in theory work in any direction, as long as the ship does not collide with {name} on the way.
		]]
	}
})

Translate:Add({
	Polski = {
		-- Main menu
		['Start at Earth']    = 'Rozpocznij na Ziemi',
		['Start at New Hope'] = 'Rozpocznij na New Hope',
		['Start at Lave']     = 'Rozpocznij na Lave',
		['Load game']         = 'Wczytaj grę',
		['Options']           = 'Opcje',
		['Quit']              = 'Wyjdź',

		-- Generic file dialog
		['Select file...'] = 'Wybierz plik...',
		['Select']         = 'Wybierz',
		['Cancel']         = 'Cofnij',

		-- Load dialog
		['Select game to load...'] = 'Wybierz zapis do wczytania...',

		-- Equipment
		['Equipment'] = 'Wyposażenie',
		['{quantity} Shield Generators'] = 'Genaratory Osłon: {quantity}',
		['{quantity} Occupied Passenger Cabins'] = 'Zajęte kabiny pasażerskie: {quantity}',
		['{quantity} Unoccupied Passenger Cabins'] = 'Wolne kabiny pasażerskie: {quantity}',
		
		-- Ship Information
		['Ship Information'] = 'Informacje o statku',
		['Weight empty:'] = 'Masa minimalna:',
		['{range} light years ({maxRange} max)'] = '{range} lat świetlnych ({maxRange} maks.)',
		['Minimum crew'] = 'Minimalna ilość załogi',
		['Crew cabins'] = 'Kabiny załogi',
		['free'] = 'wolne',
		['max'] = 'maks.',
		
		-- Personal information
		['Personal Information'] = 'Informacje o pilocie',
		['Combat'] = 'Status bojowy',
		['Rating:'] = 'Klasyfikacja:',
		['Kills:'] = 'Pokonanych:',
		['Military'] = 'Status wojskowy',
		['Rank:'] = 'Ranga:',
		['Male'] = 'Mężczyzna',
		['Female'] = 'Kobieta',
		['Toggle male/female'] = 'Mężczyzna/Kobieta',
		['Make new face'] = 'Nowa twarz',

		['Commander'] = 'Kapitan',

		['HARMLESS'] = 'Nieszkodliwy',
		['MOSTLY_HARMLESS'] = 'Przeważnie nieszkodliwy',
		['POOR'] = 'Żółtodziób',
		['AVERAGE'] = 'Przeciętny',
		['ABOVE_AVERAGE'] = 'Ponadprzeciętny',
		['COMPETENT'] = 'Fachowiec',
		['DANGEROUS'] = 'Groźny',
		['DEADLY'] = 'Zabójczy',
		['ELITE'] = 'ELITA',
		
		-- Economy & Trade
		['Economy & Trade'] = 'Ekonomia i Handel',
		['Total: '] = 'Łącznie: ',
		['Fuel tank full.'] = 'Zbiornik paliwa pełny.',
		
		-- Missions
		['Mission Details'] = 'Szczegóły misji',
		['No missions.'] = 'Brak misji.',
		["%d days left"] = "Pozostało dni: %d",
		['INACTIVE'] = 'Nieaktywna', -- Complement of ACTIVE, COMPLETED and FAILED
		
		-- Crew Tasks
		['Attempt to repair hull'] = 'Przystąpić do naprawy kadłuba',
		['Not enough {alloy} to attempt a repair'] = '{alloy} - jest zbyt mało by rozpocząć naprawę.',
		['Hull repaired by {name}, now at {repairPercent}%'] = '{name} naprawia kadłub, teraz jest {repairPercent}%',
		['Hull repair attempt failed. Hull suffered minor damage.'] = 'Naprawa kadłuba nieudana. Kadłub został nieznacznie uszkodzony.',
		['Hull does not require repair.'] = 'Kadłub nie wymaga naprawy.',
		['Destroy enemy ship'] = 'Zniszczyć wrogi statek',
		['You must request launch clearance first, Commander.'] = 'Kapitanie, najpierw musisz uzyskać zgodę na start.',
		['You must launch first, Commander.'] = 'Kapitanie, najpierw musisz wystartować.',
		['We are in hyperspace, Commander.'] = 'Kapitanie, jesteśmy w nadprzestrzeni',
		['The ship is under station control, Commander.'] = 'Kapitanie, statek jest pod kontrolą stacji.',
		['You must first select a combat target, Commander.'] = 'Kapitanie, najpierw musisz wskazać cel ataku.',
		['You must first select a suitable navigation target, Commander.'] = 'Kapitanie, najpierw musisz wskazać właściwy cel nawigacyjny.',
		['There is nobody else on board able to fly this ship.'] = 'Nikt więcej na pokładzie nie jest zdolny pilotować ten statek.',
		['Pilot seat is now occupied by {name}'] = '{name} zajmuje fotel pilota.',
		['Dock at current target'] = 'Dokować do wskazanego celu',

		-- Crew Roster
		['Name'] = 'Nazwisko',
		['Position'] = 'Stanowisko',
		['Wage'] = 'Stawka',
		['Owed'] = 'Zalegasz',
		['Next paid'] = 'Następna wypłata',
		['More info...'] = 'Więcej...',
		['General crew'] = 'Członek załogi',
		['Dismiss'] = 'Zwolnij',
		['Qualification scores'] = 'Posiadane kwalifikacje',
		['Engineering:'] = 'Naprawa:',
		['Piloting:'] = 'Pilotaż:',
		['Navigation:'] = 'Nawigacja:',
		['Sensors:'] = 'Sensory:',
		['Employment'] = 'Zatrudnienie',
		['Negotiate'] = 'Negocjacje',
		['Crew Roster'] = 'Lista załogi',
		['Give orders to crew'] = 'Wydaj rozkaz załodze',
		['Total:'] = 'Razem:',

		-- Taunts
		["I'm tired of working for nothing. Don't you know what a contract is?"] = "Odczuwam zmęczenie pracując za darmo. Czy Ty wiesz co to kontrakt?",
		["It's been great working for you. If you need me again, I'll be here a while."] = "Praca dla ciebie to przyjemność. Jeśli znów będziesz mnie potrzebować, to bedę tu chwilowo.",
		["You're going to regret sacking me!"] = "Jeszcze pożałujesz! Zwolnić, mnie?!",
		["Good riddance to you, too."] = "Dobrze by było tobie też się zwolnić.",

		-- Orbital analysis
		['Orbit'] = 'Orbita',
		['Orbital Analysis'] = 'Analiza Orbity',
		['Located {distance}km from the centre of {name}:'] = 'Położenie {distance}km od centrum {name}:',
		['Circular orbit speed:'] = 'Prędkość orbitalna:',
		['Escape speed:'] = 'Prędkość ucieczki:',
		['Descent-to-ground speed:'] = 'Prędkość graniczna:',
		['Notes:'] = 'Uwagi:',
		ORBITAL_ANALYSIS_NOTES = [[
Prędkość orbitalna to stała prędkość z jaką powinien poruszać się statek pod kątem 90° względem osi statek/{name}.

Prędkość graniczna, jest minimalną prędkością przy której statek pozostaje na orbicie.  Przy mniejszej prędkości lub kącie, rozpocznie się opadanie ku powierzchni {name}.

Prędkość ucieczki teoretycznie powoduje opuszczenie orbity, jeśli tylko statek nie jest na kursie kolizyjnym z {name}.
		]]				
	}
})

Translate:Add({
	Spanish = {
		 -- Main menu
		['Start at Earth'] = 'Comenzar en Tierra',
		['Start at New Hope'] = 'Comenzar en New Hope',
		['Start at Lave'] = 'Comenzar en Lave',
		['Load game'] = 'Cargar',
		['Options'] = 'Opciones',
		['Quit'] = 'Salir',

		-- Generic file dialog
		['Select file...'] = 'Seleccionar archivo...',
		['Select'] = 'Seleccionar',
		['Cancel'] = 'Cancelar',

		-- Load dialog
		['Select game to load...'] = 'Seleccionar juego...',

		-- Equipment
		['Equipment'] = 'Equipo',
		['{quantity} Shield Generators'] = '{quantity} Generadores de Escudo',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Cabinas de Pasajeros Ocupadas',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Cabinas de Pasajeros Vacías',

		-- Ship Information
		['Ship Information'] = 'Info de la Nave',
		['Weight empty:'] = 'Peso en vacío:',
		['{range} light years ({maxRange} max)'] = '{range} años luz ({maxRange} max)',
		['Minimum crew'] = 'Tripulación mínima',
		['Crew cabins'] = 'Cabinas de Tripulación',
		['free'] = 'vacío',
		['max'] = 'máx',

		-- Personal information
		['Personal Information'] = 'Info Personal',
		['Combat'] = 'Combate',
		['Rating:'] = 'Rating:',
		['Kills:'] = 'Muertes:',
		['Military'] = 'Militar',
		['Rank:'] = 'Rango:',
		['Male'] = 'Hombre',
		['Female'] = 'Mujer',
		['Toggle male/female'] = 'Hombre/Mujer',
		['Make new face'] = 'Crear nuevo rostro',

		['Commander'] = 'Capitán',

		['HARMLESS'] = 'Inofensivo',
		['MOSTLY_HARMLESS'] = 'En su mayoría inofensivo',
		['POOR'] = 'Pobre',
		['AVERAGE'] = 'Standard',
		['ABOVE_AVERAGE'] = 'Por encima de la media',
		['COMPETENT'] = 'Competente',
		['DANGEROUS'] = 'Peligroso',
		['DEADLY'] = 'Mortal',
		['ELITE'] = 'ELITE',

		-- Economy & Trade
		['Economy & Trade'] = 'Economía & Comercio',
		['Total: '] = 'Total: ',
		['Fuel tank full.'] = 'Tanque de fuel lleno.',

		-- Missions
		['Mission Details'] = 'Detalles de Misión',
		['No missions.'] = 'Sin Misiones.',
		['INACTIVE'] = 'Inactivo', -- Complement of ACTIVE, COMPLETED and FAILED

		-- Crew Tasks
		['Attempt to repair hull'] = 'Reparar casco',
		['Not enough {alloy} to attempt a repair'] = 'No hay suficiente {alloy} para la reparación',
		['Hull repaired by {name}, now at {repairPercent}%'] = 'Casco reparado por {name}, ahora al {repairPercent}%',
		['Hull repair attempt failed. Hull suffered minor damage.'] = 'Intento de reparación del casco fracasado. El casco ha sufrido daños menores.',
		['Hull does not require repair.'] = 'El casco no requiere reparación.',
		['Destroy enemy ship'] = 'Destruir nave enemiga',
		['You must request launch clearance first, Commander.'] = 'Primero debe solicitar autorización para el despegue, Capitán.',
		['You must launch first, Commander.'] = 'Primero debe despegar, Capitán.',
		['We are in hyperspace, Commander.'] = 'Estamos en el hiperespacio, Capitán.',
		['The ship is under station control, Commander.'] = 'La nave esta bajo control de la estación, Capitán.',
		['You must first select a combat target, Commander.'] = 'Primero debe seleccionar un objetivo de combate, Capitán.',
		['You must first select a suitable navigation target, Commander.'] = 'Primero debe seleccionar un objetivo de navegación válido, Capitán.',
		['There is nobody else on board able to fly this ship.'] = 'No hay nadie mas a bordo capaz de pilotar la nave.',
		['Pilot seat is now occupied by {name}'] = 'Actualmente el puesto de piloto esta ocupado por {name}',
		['Dock at current target'] = 'Atracar en el objetivo actual',

		-- Crew Roster
		['Name'] = 'Nombre',
		['Position'] = 'Puesto',
		['Wage'] = 'Sueldo',
		['Owed'] = 'Deuda',
		['Next paid'] = 'Próxima paga',
		['More info...'] = 'Mas info...',
		['General crew'] = 'Tripulación general',
		['Dismiss'] = 'Despedir',
		['Qualification scores'] = 'Calificación',
		['Engineering:'] = 'Ingeniería',
		['Piloting:'] = 'Pilotaje:',
		['Navigation:'] = 'Navegación:',
		['Sensors:'] = 'Sensores:',
		['Employment'] = 'Ocupación',
		['Negotiate'] = 'Negociar',
		['Crew Roster'] = 'Lista',
		['Give orders to crew'] = 'Dar órdenes',

		-- Taunts
		["I'm tired of working for nothing. Don't you know what a contract is?"] = "Estoy harto de trabajar por nada. Conoce la palabra contrato?",
		["It's been great working for you. If you need me again, I'll be here a while."] = "Ha sido estupendo trabajar para usted. Si me necesita de nuevo, Estaré por aquí un tiempo.",
		["You're going to regret sacking me!"] = "Va a lamentar haberme explotado!",
		["Good riddance to you, too."] = "Buen viaje, igualmente.",

		-- Orbital analysis
		['Orbit'] = 'Orbita',
		['Orbital Analysis'] = 'Análisis Orbital',
		['Located {distance}km from the centre of {name}:'] = 'Localizado a {distance}km del centro de {name}:',
		['Circular orbit speed:'] = 'Velocidad de Orbita Circular',
		['Escape speed:'] = 'Velocidad de escape:',
		['Descent-to-ground speed:'] = 'Velocidad de descenso a tierra:',
		['Notes:'] = 'Notas:',
		ORBITAL_ANALYSIS_NOTES = [[
La velocidad de órbita circular es dada por una velocidad tangencial. La nave se debería mover en una dirección a 90° del eje de la nave/{name}.

La velocidad de descenso es un mínimo absoluto, y es también tangencial. Una velocidad o ángulo mas bajos resultará en un curso que intersecta con la superficie de {name}.

La velocidad de escape funcionará, en teoría, en cualquier dirección, mientras que la nave no colisione con {name} en la ruta.
		]]
	}
})

Translate:Add({
	Magyar = {
		-- Main menu
		['Start at Earth']    = 'Indítás: Föld',
		['Start at New Hope'] = 'Indítás: New Hope',
		['Start at Lave']     = 'Indítás: Lave',
		['Load game']         = 'Játék betöltése',
		['Options']           = 'Beállítások',
		['Quit']              = 'Kilépés',

		-- Generic file dialog
		['Select file...'] = 'Fájlkiválasztás...',
		['Select']         = 'Kiválaszt',
		['Cancel']         = 'Mégsem',

		-- Load dialog
		['Select game to load...'] = 'Válassz betöltendö fájlt...',

		-- Personal Information
		['HARMLESS'] = 'Ártalmatlan',
		['MOSTLY_HARMLESS'] = 'Jobbára ártalmatlan',
		['POOR'] = 'Gyenge',
		['AVERAGE'] = 'Átlagos',
		['ABOVE_AVERAGE'] = 'Átlag feletti',
		['COMPETENT'] = 'Versenyképes',
		['DANGEROUS'] = 'Veszélyes',
		['DEADLY'] = 'Halálos',
		['ELITE'] = 'ELITE',
	}
})

Translate:Add({
	Russian = {
		-- Main menu
		['Start at Earth']    = 'Новый старт: Earth',
		['Start at New Hope'] = 'Новый старт: New Hope',
		['Start at Lave']     = 'Новый старт: Lave',
		['Load game']         = 'Загрузить запись',
		['Options']           = 'Настройки',
		['Quit']              = 'Выход',

		-- Generic file dialog
		['Select file...'] = 'Выберите файл...',
		['Select']         = 'Выбрать',
		['Cancel']         = 'Отмена',
		
		-- Load dialog
		['Select game to load...'] = 'Выберите файл для загрузки...',

		-- Equipment
		['Equipment'] = 'Оборудование',
		['{quantity} Shield Generators'] = 'Генераторы защитного поля {quantity}',
		['{quantity} Occupied Passenger Cabins'] = 'Занятых пассажирских кают {quantity}',
		['{quantity} Unoccupied Passenger Cabins'] = 'Свободных пассажирских кают {quantity}',
		
		-- Ship Information
		['Ship Information'] = 'Информация о корабле',
		['Weight empty:'] = 'Собств.вес корабля:',
		['{range} light years ({maxRange} max)'] = '{range} св.лет (из {maxRange} макс.)',
		['Minimum crew'] = 'Минимальный экипаж',
		['Crew cabins'] = 'Каюты экипажа',
		['free'] = 'свободно',
		['max'] = 'макс.',
		
		-- Personal information
		['Personal Information'] = 'Персональная информация',
		['Combat'] = 'Боевой рейтинг',
		['Rating:'] = 'Рейтинг:',
		['Kills:'] = 'Побед:',
		['Military'] = 'Военный ранг',
		['Rank:'] = 'Ранг:',
		['Male'] = 'Мужчина',
		['Female'] = 'Женщина',
		['Toggle male/female'] = 'Пол муж./жен.',
		['Make new face'] = 'Новое лицо',
		['Commander'] = 'Капитан',

		['HARMLESS'] = 'Безобидный',
		['MOSTLY_HARMLESS'] = 'Почти безобидный',
		['POOR'] = 'Слабый',
		['AVERAGE'] = 'Средний',
		['ABOVE_AVERAGE'] = 'Выше среднего',
		['COMPETENT'] = 'Умелый',
		['DANGEROUS'] = 'Опасный',
		['DEADLY'] = 'Смертельный',
		['ELITE'] = 'ЭЛИТА',
		
		-- Economy & Trade
		['Economy & Trade'] = 'Экономика и торговля',
		['Total: '] = 'Всего: ',
		['Fuel tank full.'] = 'Топливный бак полон.',

		-- Missions
		['Mission Details'] = 'О задании',
		['No missions.'] = 'Нет заданий.',
		["%d days left"] = "Осталось дней: %d",
		['INACTIVE'] = 'Неактивно', -- Complement of ACTIVE, COMPLETED and FAILED
		
				-- Crew Tasks
		['Attempt to repair hull'] = 'Попытка починки корпуса',
		['Not enough {alloy} to attempt a repair'] = 'Недостаточно {alloy} для проведения ремонта',
		['Hull repaired by {name}, now at {repairPercent}%'] = '{name} отремонтировал корпус до {repairPercent}%',
		['Hull repair attempt failed. Hull suffered minor damage.'] = 'Попытка ремонта не удалась. Корпус получил незначительные повреждения.',
		['Hull does not require repair.'] = 'Корпус не требует ремонта.',
		['Destroy enemy ship'] = 'Уничтожить корабль противника',
		['You must request launch clearance first, Commander.'] = 'Сначала мы должны запросить разрешение на взлет, капитан!',
		['You must launch first, Commander.'] = 'Сначала мы должны взлететь, капитан.',
		['We are in hyperspace, Commander.'] = 'Мы в гиперпространстве, капитан!',
		['The ship is under station control, Commander.'] = 'Корабль под контролем станции, капитан.',
		['You must first select a combat target, Commander.'] = 'Вы должны указать боевую цель, капитан!',
		['You must first select a suitable navigation target, Commander.'] = 'Сначала вы должны указать навигационную цель, капитан.',
		['There is nobody else on board able to fly this ship.'] = 'Больше никто на борту не умеет управлять этим кораблем.',
		['Pilot seat is now occupied by {name}'] = 'Обязанности пилота сейчас выполняет {name}.',
		['Dock at current target'] = 'Стыковка с выбранной целью.',

		-- Crew Roster
		['Name'] = 'Имя',
		['Position'] = 'Должность',
		['Wage'] = 'Оклад',
		['Owed'] = 'Долг',
		['Next paid'] = 'След.платеж',
		['More info...'] = 'Доп.информация...',
		['General crew'] = 'Основной экипаж',
		['Dismiss'] = 'Уволить',
		['Qualification scores'] = 'Ур.квалификации',
		['Engineering:'] = 'Инженерия:',
		['Piloting:'] = 'Пилотирование:',
		['Navigation:'] = 'Навигация:',
		['Sensors:'] = 'Сенсоры:',
		['Employment'] = 'Занятость',
		['Negotiate'] = 'Условия',
		['Crew Roster'] = 'Список экипажа',
		['Give orders to crew'] = 'Приказы экипажу',

		-- Taunts
		["I'm tired of working for nothing. Don't you know what a contract is?"] = "Я устал работать даром. Разве вы не знаете, что такое контракт?",
		["It's been great working for you. If you need me again, I'll be here a while."] = "Работать на вас было одним удовольствием. Если вновь захотите меня нанять - некоторое время я буду здесь.",
		["You're going to regret sacking me!"] = "Вы пожалеете о том, что уволили меня!",
		["Good riddance to you, too."] = "Скатертью дорожка!",
		
		-- Orbital analysis
		['Orbit'] = 'Орбита',
		['Orbital Analysis'] = 'Анализ орбиты',
		['Located {distance}km from the centre of {name}:'] = 'Положение: {distance}км от центра {name}:',
		['Circular orbit speed:'] = 'Круговая орбит.скорость',
		['Escape speed:'] = 'Скорость отдаления:',
		['Descent-to-ground speed:'] = 'Скорость снижения:',
		['Notes:'] = 'Примечание:',
		ORBITAL_ANALYSIS_NOTES = [[
Круговая орбитальная скорость дана для тангенциальной скорости. Корабль должен двигаться под 90В° к оси корабля/{name}.

Скорость снижения - минимальная и тангенциальная. Наиболее низка скорость или минимальный угол, которые приведут к пересечению с поверхностью {name}.

Скорость отдаления - теоретическая для любого направления, на котором корабль не пересечется с поверхностью {name}.
		]]
	}
})

Translate:Add({
	Deutsch = {
		-- Main menu
		['Start at Earth']    = 'Starte auf der Erde',
		['Start at New Hope'] = 'Starte auf New Hope',
		['Start at Lave']     = 'Starte auf Lave',
		['Load game']         = 'Lade Spiel',
		['Options']           = 'Optionen',
		['Quit']              = 'Beenden',

		-- Generic file dialog
		['Select file...'] = 'Datei auswählen...',
		['Select']         = 'Auswählen',
		['Cancel']         = 'Abbrechen',

		-- Load dialog
		['Select game to load...'] = 'Spiel zum Laden auswählen...',

		-- Equipment
		['Equipment'] = 'Ausrüstung',
		['{quantity} Shield Generators'] = '{quantity} Schild-Generatoren',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Belegte Passagierkabinen',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Unbelegte Passagierkabinen',
		
		-- Ship Information
		['Ship Information'] = 'Schiffs-Informationen',
		['Weight empty:'] = 'Leergewicht:',
		['{range} light years ({maxRange} max)'] = '{range} Lichtjahre ({maxRange} max)',
		
		-- Personal information
		['Personal Information'] = 'Persönliche Informationen',
		['Combat'] = 'Kampf',
		['Rating:'] = 'Einstufung:',
		['Kills:'] = 'Abschüsse:',
		['Military'] = 'Militärisch',
		['Rank:'] = 'Rang:',
		['Male'] = 'Männlich',
		['Female'] = 'Weiblich',

		['HARMLESS'] = 'Harmlos',
		['MOSTLY_HARMLESS'] = 'Relativ harmlos',
		['POOR'] = 'Schlecht',
		['AVERAGE'] = 'Durchschnittlich',
		['ABOVE_AVERAGE'] = 'Überdurchschnittlich',
		['COMPETENT'] = 'Kompetent',
		['DANGEROUS'] = 'Gefährlich',
		['DEADLY'] = 'Tödlich',
		['ELITE'] = 'ELITE',

		-- Economy & Trade
		['Economy & Trade'] = 'Wirtschaft & Handel',
		['Total: '] = 'Komplett: ',
		['Fuel tank full.'] = 'Treibstofftank ist voll.',

		-- Missions
		['Mission Details'] = 'Missionsdetails',
		['No missions.'] = 'Keine Missionen.',
		['INACTIVE'] = 'Inaktiv', -- Complement of ACTIVE, COMPLETED and FAILED
		
		-- Orbital analysis
		['Orbit'] = 'Bahn',
		['Orbital Analysis'] = 'Bahnanalyse',
		['Located {distance}km from the centre of {name}:'] = '{distance}km vom Zentrum von {name} lokalisiert:',
		['Circular orbit speed:'] = 'Kreisbahn-Geschwindigkeit',
		['Escape speed:'] = 'Fluchtgeschwindigkeit:',
		['Descent-to-ground speed:'] = 'Landegeschwindigkeit:',
		['Notes:'] = 'Notizen:',
		ORBITAL_ANALYSIS_NOTES = [[
Die Kreisbahn-Geschwindigkeit wird für eine tangentiale Geschwindigkeit angegeben. Das schiff sollte in einem 90°-Winkel zur Schiff/{name}-Achse stehen.

Die Landegeschwindigkeit ist ein absolutes Minimum und ist auch tangential. Eine geringere Geschwindigkeit und ein kleinerer Winkel wird in einem Kurs resultieren, der die Oberfläche von {name} schneidet.

Die Fluchtgeschwindigkeit funktioniert theoretisch gesehen in alle Richtungen, solange das Schiff auf dem Weg mit {name} kollidiert.
		]]
	}
})

Translate:Add({
	Czech = {
		-- Main menu
		['Start at Earth']    = 'Start: Earth',
		['Start at New Hope'] = 'Start: New Hope',
		['Start at Lave']     = 'Start: Lave',
		['Load game']         = 'Načíst hru',
		['Options']           = 'Možnosti',
		['Quit']              = 'Ukončit',

		-- Generic file dialog
		['Select file...'] = 'Otevřít soubor...',
		['Select']         = 'Otevřít',
		['Cancel']         = 'Zrušit',

		-- Load dialog
		['Select game to load...'] = 'Vyber uloženou hru...',

		-- Equipment
		['Equipment'] = 'Vybavení',
		['{quantity} Shield Generators'] = '{quantity} generátory štítů',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} obsazených pasažérských kabin',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} volných pasažérských kabin',

		-- Ship Information
		['Ship Information'] = 'Informace o lodi',
		['Weight empty:'] = 'Prázdná váha:',
		['{range} light years ({maxRange} max)'] = '{range} ly ({maxRange} max)',
		['Minimum crew'] = 'Minimální posádka',
		['Crew cabins'] = 'Kabin pro posádku',
		['free'] = 'volných',
		['max'] = 'max',

		-- Personal information
		['Personal Information'] = 'Osobní informace',
		['Combat'] = 'Bojové',
		['Rating:'] = 'Hodnocení:',
		['Kills:'] = 'Zabitých:',
		['Military'] = 'Vojenské',
		['Rank:'] = 'Hodnost:',
		['Male'] = 'Muž',
		['Female'] = 'Žena',
		['Toggle male/female'] = 'Změnit muž/žena',
		['Make new face'] = 'Vytvořit jiný obličej',

		['Commander'] = 'Velitel',

		['HARMLESS'] = 'Neškodný',
		['MOSTLY_HARMLESS'] = 'Převážně neškodný',
		['POOR'] = 'Ubohý',
		['AVERAGE'] = 'Průměrný',
		['ABOVE_AVERAGE'] = 'Nadprůměrný',
		['COMPETENT'] = 'Schopný',
		['DANGEROUS'] = 'Nebezpečný',
		['DEADLY'] = 'Smrtící',
		['ELITE'] = 'ELITA',

		-- Economy & Trade
		['Economy & Trade'] = 'Ekonomika & obchod',
		['Total: '] = 'Celkem: ',
		['Fuel tank full.'] = 'Palivová nádrž plná.',

		-- Missions
		['Mission Details'] = 'Detaily mise',
		['No missions.'] = 'Žádné mise.',
		["%d days left"] = "Zbývá dnů: %d",
		['INACTIVE'] = 'Neprobíhá', -- Complement of ACTIVE, COMPLETED and FAILED

		-- Crew Tasks
		['Attempt to repair hull'] = 'Zkus opravit trup',
		['Not enough {alloy} to attempt a repair'] = 'Nedostatek {alloy} k pokusu o opravu',
		['Hull repaired by {name}, now at {repairPercent}%'] = '{name} opravil trup, stav {repairPercent}%',
		['Hull repair attempt failed. Hull suffered minor damage.'] = 'Neúspěšný pokus o opravu trupu. Trup utrpěl menší poškození.',
		['Hull does not require repair.'] = 'Trup nepotřebuje opravu.',
		['Destroy enemy ship'] = 'Znič nepřátelskou loď',
		['You must request launch clearance first, Commander.'] = 'Nejdříve je nutné vyžádat povolení k odletu, veliteli.',
		['You must launch first, Commander.'] = 'Nejdříve je nutné odstartovat, veliteli.',
		['We are in hyperspace, Commander.'] = 'Jsme v hyperprostoru, veliteli.',
		['The ship is under station control, Commander.'] = 'Loď je pod kontrolou stanice, veliteli.',
		['You must first select a combat target, veliteli.'] = 'Nejdříve je nutné vybrat bojový cíl, veliteli.',
		['You must first select a suitable navigation target, veliteli.'] = 'Nejdříve je nutné vybrat vhodný cíl navigace, veliteli.',
		['There is nobody else on board able to fly this ship.'] = 'Na palube není nikdo jiný schopen pilotovat loď.',
		['Pilot seat is now occupied by {name}'] = 'Nyní pilotuje {name}',
		['Dock at current target'] = 'Přistaň na zvoleném cíli',

		-- Crew Roster
		['Name'] = 'Jméno',
		['Position'] = 'Pozice',
		['Wage'] = 'Mzda',
		['Owed'] = 'Dluh',
		['Next paid'] = 'Další platba',
		['More info...'] = 'Další info...',
		['General crew'] = 'Základní posádka',
		['Dismiss'] = 'Propustit',
		['Qualification scores'] = 'Kvalifikační hodnocení',
		['Engineering:'] = 'Údržba:',
		['Piloting:'] = 'Pilotáž:',
		['Navigation:'] = 'Navigace:',
		['Sensors:'] = 'Senzory:',
		['Employment'] = 'Zaměstnanost',
		['Negotiate'] = 'Vyjenávat',
		['Crew Roster'] = 'Seznam posádky',
		['Give orders to crew'] = 'Dát posádce rozkazy',
		['Total:'] = 'Celkem:',

		-- Taunts
		["I'm tired of working for nothing. Don't you know what a contract is?"] = "Už mě unavuje pracovat pro nic za nic. Víš vůbec co to je smlouva?",
		["It's been great working for you. If you need me again, I'll be here a while."] = "Dobře se mi s tebou pracovalo. Pokud mě budeš zase potřebovat, jsem tu v cukuletu.",
		["You're going to regret sacking me!"] = "Toho budeš litovat, naštvat mě!",
		["Good riddance to you, too."] = "Spánembohem i tobě.",

		-- Orbital analysis
		['Orbit'] = 'Orbita',
		['Orbital Analysis'] = 'Orbitální analýza',
		['Located {distance}km from the centre of {name}:'] = '{distance}km od centra {name}:',
		['Circular orbit speed:'] = 'Kruhová orbitální rychlost',
		['Escape speed:'] = 'Úniková rychlost:',
		['Descent-to-ground speed:'] = 'Přistávací rychlost:',
		['Notes:'] = 'Notes:',
		ORBITAL_ANALYSIS_NOTES = [[
Úhlová orbitální rychlost je vztažena k tangenciální rychlosti. Loď by se měla pohybovat kolmo k ose lodi/{name}.

Descent speed is an absolute minimum, and is also tangential. A slower speed or a lower angle will result in a course which intersects with the surface of {name}.

Úniková rychlost může být (teoreticky) libovolná ve všech směrech, pokud se trajektorie lodi nenáchází na kolizním kurzu s {name}.
		]]
	}
})
