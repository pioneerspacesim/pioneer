-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")

Translate:Add({
	English = {
		-- Main menu
		['Start at Earth']    = 'Start at Earth',
		['Start at New Hope'] = 'Start at New Hope',
		["Start at Barnard's Star"] = "Start at Barnard's Star",
		['Load game']         = 'Load game',
		['Options']           = 'Options',
		['Quit']              = 'Quit',

		-- Generic file dialog
		['Select file...'] = 'Select file...',
		['Select']         = 'Select',
		['Cancel']         = 'Cancel',
		['Ok']             = 'Ok',

		-- Load dialog
		['Select game to load...'] = 'Select game to load...',
		['Error'] = 'Error',
		['An error has occurred'] = 'An error has occurred',
		['Could not load game: '] = 'Could not load game: ',
		['Load'] = 'Load',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Select a file to save to or enter a new filename',
		['Save'] = 'Save',

		-- Settings screen
		['Return to menu'] = 'Return to menu',
		['Video resolution'] = 'Video resolution',
		['Off'] = 'Off',
		['Multisampling'] = 'Multisampling',
		['Very low'] = 'Very low',
		['Low'] = 'Low',
		['Medium'] = 'Medium',
		['High'] = 'High',
		['Very high'] = 'Very high',
		['Planet detail distance'] = 'Planet detail distance',
		['Planet textures'] = 'Planet textures',
		['Fractal detail'] = 'Fractal detail',
		['City detail level'] = 'City detail level',
		['Display nav tunnels'] = 'Display nav tunnels',
		['Full screen'] = 'Full screen',
		['Use shaders'] = 'Use shaders',
		['Compress Textures'] = 'Compress Textures',
		['Video configuration (restart game to apply)'] = 'Video configuration (restart game to apply)',
		['Mute'] = 'Mute',
		['Master:'] = 'Master:',
		['Music:'] = 'Music:',
		['Effects:'] = 'Effects:',
		['Language (restart game to apply)'] = 'Language (restart game to apply)',
		['Press a key or controller button'] = 'Press a key or controller button',
		['Change Binding'] = 'Change Binding',
		['Invert Mouse Y'] = 'Invert Mouse Y',
		['Enable joystick'] = 'Enable joystick',
		['Control Options'] = 'Control Options',
		['Set'] = 'Set',
		['Video'] = 'Video',
		['Sound'] = 'Sound',
		['Language'] = 'Language',
		['Controls'] = 'Controls',
		['Exit this game'] = 'Exit this game',

		-- Equipment
		['Equipment'] = 'Equipment',
		['{quantity} Shield Generators'] = '{quantity} Shield Generators',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Occupied Passenger Cabins',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Unoccupied Passenger Cabins',
		
		-- Ship Information
		['Ship Information'] = 'Ship Information',
		['Capacity used'] = 'Capacity used',
		['All-up weight'] = 'All-up weight', -- total weight
		['Weight empty:'] = 'Weight empty:',
		['Hyperdrive'] = 'Hyperdrive',
		['Hyperspace range'] = 'Hyperspace range',
		['{range} light years ({maxRange} max)'] = '{range} light years ({maxRange} max)',
		['Minimum crew'] = 'Minimum crew',
		['Crew cabins'] = 'Crew cabins',
		['Front weapon'] = 'Front weapon',
		['Rear weapon'] = 'Rear weapon',
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
		['Jettison'] = 'Jettison',
		['Refuel'] = 'Refuel',

		-- Missions
		['Missions'] = 'Missions',
		['Mission Details'] = 'Mission Details',
		['No missions.'] = 'No missions.',
		["%d days left"] = "Days left: %d",
		['Type'] = 'Type',
		['Client'] = 'Client',
		['Location'] = 'Location',
		['Due'] = 'Due',
		['Reward'] = 'Reward',
		['Status'] = 'Status',
		['ACTIVE'] = 'Active',
		['COMPLETED'] = 'Completed',
		['FAILED'] = 'Failed',
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
		["Start at Barnard's Star"] = "Rozpocznij na Barnard's Star",
		['Load game']         = 'Wczytaj grę',
		['Options']           = 'Opcje',
		['Quit']              = 'Wyjdź',

		-- Generic file dialog
		['Select file...'] = 'Wybierz plik...',
		['Select']         = 'Wybierz',
		['Cancel']         = 'Cofnij',
		['Ok']             = 'Ok',

		-- Load dialog
		['Select game to load...'] = 'Wybierz zapis do wczytania...',
		['Error'] = 'Błąd!',
		['An error has occurred'] = 'Wystąpił błąd',
		['Could not load game: '] = 'Nie można wczytać gry: ',
		['Load'] = 'Wczytaj',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Wybierz plik do zapisu lub podaj nową nazwę pliku',
		['Save'] = 'Zapisz',

		-- Settings screen
		['Return to menu'] = 'Powrót do menu',
		['Video resolution'] = 'Rozdzielczość ekranu',
		['Off'] = 'Wyłączony',
		['Multisampling'] = 'Multisampling',
		['Very low'] = 'Bardzo mała',
		['Low'] = 'Mała',
		['Medium'] = 'Średnia',
		['High'] = 'Duża',
		['Very high'] = 'Bardzo duża',
		['Planet detail distance'] = 'Szczeg. planet',
		['Planet textures'] = 'Tekstury planet',
		['Fractal detail'] = 'Dokł. fraktali',
		['City detail level'] = 'Szczeg. miast',
		['Display nav tunnels'] = 'Wyświetl tunel nawigacyjny',
		['Full screen'] = 'Pełny ekran',
		['Use shaders'] = 'Użyj cieniowania',
		['Compress Textures'] = 'Kompresja Tekstur',
		['Video configuration (restart game to apply)'] = 'Ustawienia obrazu (wymagany restart)',
		['Mute'] = 'Wycisz',
		['Master:'] = 'Głośność:',
		['Music:'] = 'Muzyka:',
		['Effects:'] = 'Efekty:',
		['Language (restart game to apply)'] = 'Język (wymagany restart)',
		['Press a key or controller button'] = 'Naciśnij klawisz lub przycisk kontrolera',
		['Change Binding'] = 'Zmień przypisany klawisz',
		['Invert Mouse Y'] = 'Odwróć Oś Y',
		['Enable joystick'] = 'Włącz sterowanie joystickiem',
		['Control Options'] = 'Opcje sterowania',
		['Set'] = 'Ustaw',
		['Video'] = 'Obraz',
		['Sound'] = 'Dźwięk',
		['Language'] = 'Język',
		['Controls'] = 'Sterowanie',
		['Exit this game'] = 'Wyjdź z gry',

		-- Equipment
		['Equipment'] = 'Wyposażenie',
		['{quantity} Shield Generators'] = 'Genaratory Osłon: {quantity}',
		['{quantity} Occupied Passenger Cabins'] = 'Zajęte kabiny pasażerskie: {quantity}',
		['{quantity} Unoccupied Passenger Cabins'] = 'Wolne kabiny pasażerskie: {quantity}',
		
		-- Ship Information
		['Ship Information'] = 'Informacje o statku',
		['Capacity used'] = 'Użyta przestrzeń',
		['All-up weight'] = 'Masa całkowita', -- total weight
		['Weight empty:'] = 'Masa minimalna:',
		['Hyperdrive'] = 'Hipernapęd',
		['Hyperspace range'] = 'Zasięg w nadprzestrzeni',
		['{range} light years ({maxRange} max)'] = '{range} lat świetlnych ({maxRange} maks.)',
		['Minimum crew'] = 'Minimalna ilość załogi',
		['Crew cabins'] = 'Kabiny załogi',
		['free'] = 'wolne',
		['max'] = 'maks.',
		['Front weapon'] = 'Przednia broń',
		['Rear weapon'] = 'Tylna broń',
		
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
		['Jettison'] = 'Wystrzel',
		['Refuel'] = 'Zatankuj',
		
		-- Missions
		['Missions'] = 'Misje',
		['Mission Details'] = 'Szczegóły misji',
		['No missions.'] = 'Brak misji.',
		["%d days left"] = "Pozostało dni: %d",
		['Type'] = 'Typ',
		['Client'] = 'Klient',
		['Location'] = 'Lokalizacja',
		['Due'] = 'Termin',
		['Reward'] = 'Zapłata',
		['Status'] = 'Status',
		['ACTIVE'] = 'Aktywna',
		['COMPLETED'] = 'Ukończona',
		['FAILED'] = 'Nieukończona',
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
		["Start at Barnard's Star"] = "Comenzar en Barnard's Star",
		['Load game'] = 'Cargar',
		['Options'] = 'Opciones',
		['Quit'] = 'Salir',

		-- Generic file dialog
		['Select file...'] = 'Seleccionar archivo...',
		['Select'] = 'Seleccionar',
		['Cancel'] = 'Cancelar',
		['Ok']     = 'Ok',

		-- Load dialog
		['Select game to load...'] = 'Selecciona juego salvado...',
		['Error'] = 'Error',
		['An error has occurred'] = 'Ha ocurrido un error',
		['Could not load game: '] = 'No se pudo cargar el juego: ',
		['Load'] = 'Cargar',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Seleccione un archivo para Guardar o cree un nuevo nombre de fichero',
		['Save'] = 'Guardar',

		-- Settings screen
		['Return to menu'] = 'Volver al menú',
		['Video resolution'] = 'Resolución de pantalla',
		['Off'] = 'Off',
		['Multisampling'] = 'Antialias',
		['Very low'] = 'Muy bajo',
		['Low'] = 'Bajo',
		['Medium'] = 'Medio',
		['High'] = 'Alto',
		['Very high'] = 'Muy alto',
		['Planet detail distance'] = 'Dist. detalle planetario',
		['Planet textures'] = 'Tex. planetarias',
		['Fractal detail'] = 'Detalle Fractal',
		['City detail level'] = 'Detalle urbano',
		['Display nav tunnels'] = 'Mostrar túnel de navegación',
		['Full screen'] = 'Pantalla completa',
		['Use shaders'] = 'Usar Shaders',
		['Compress Textures'] = 'Comprimir Texturas',
		['Video configuration (restart game to apply)'] = 'Configuración de pantalla (Reiniciar para aplicar)',
		['Mute'] = 'Silenciar',
		['Master:'] = 'Master:',
		['Music:'] = 'Música:',
		['Effects:'] = 'Efectos:',
		['Language (restart game to apply)'] = 'Idioma (reiniciar para aplicar)',
		['Press a key or controller button'] = 'Pulsa una tecla o botón del controlador',
		['Change Binding'] = 'Modificar',
		['Invert Mouse Y'] = 'Invertir Mouse Y',
		['Enable joystick'] = 'Habilitar joystick',
		['Control Options'] = 'Opciones de Control',
		['Set'] = 'Fijar',
		['Video'] = 'Video',
		['Sound'] = 'Sonido',
		['Language'] = 'Idioma',
		['Controls'] = 'Controles',
		['Exit this game'] = 'Abandonar el Juego',

		-- Equipment
		['Equipment'] = 'Equipo',
		['{quantity} Shield Generators'] = '{quantity} Generadores de Escudo',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Cabinas de Pasajeros Ocupadas',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Cabinas de Pasajeros Vacías',

		-- Ship Information
		['Ship Information'] = 'Info de la Nave',
		['Capacity used'] = 'Capacidad usada',
		['All-up weight'] = 'Masa total', -- total weight
		['Weight empty:'] = 'Peso en vacío:',
		['Hyperdrive'] = 'Motor hiperespacial',
		['Hyperspace range'] = 'Autonomía hiperespacial',
		['{range} light years ({maxRange} max)'] = '{range} años luz ({maxRange} max)',
		['Minimum crew'] = 'Tripulación mínima',
		['Crew cabins'] = 'Cabinas de Tripulación',
		['free'] = 'vacío',
		['max'] = 'máx',
		['Front weapon'] = 'Arma frontal',
		['Rear weapon'] = 'Arma de popa',

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
		['Jettison'] = 'Tirar por la borda',
		['Refuel'] = 'Repostar',

		-- Missions
		['Missions'] = 'Misiones',
		['Mission Details'] = 'Detalles de Misión',
		['No missions.'] = 'Sin Misiones.',
		['Type'] = 'Tipo',
		['Client'] = 'Cliente',
		['Location'] = 'Localización',
		['Due'] = 'Deuda',
		['Reward'] = 'Recompensa',
		['Status'] = 'Status',
		['ACTIVE'] = 'Activa',
		['COMPLETED'] = 'Completada',
		['FAILED'] = 'Fallida',
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
		["Start at Barnard's Star"] = "Indítás: Barnard's Star",
		['Load game']         = 'Játék betöltése',
		['Options']           = 'Beállítások',
		['Quit']              = 'Kilépés',

		-- Generic file dialog
		['Select file...'] = 'Fájlkiválasztás...',
		['Select']         = 'Kiválaszt',
		['Cancel']         = 'Mégsem',
		--['Ok']             = '',

		-- Load dialog
		['Select game to load...'] = 'Válassz betöltendö fájlt...',
		--['Load'] = '',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Add meg a játékmentést a kívánt nevet',
		['Save'] = 'Ment',

		-- Settings screen
		--['Return to menu'] = '',
		['Video resolution'] = 'Képernyő felbontása',
		['Off'] = 'Ki',
		--['Multisampling'] = '',
		['Very low'] = 'Min.',
		['Low'] = 'Kicsi',
		['Medium'] = 'Közepes',
		['High'] = 'Magas',
		['Very high'] = 'Magasabb',
		['Planet detail distance'] = 'Bolygórészlettávolság',
		['Planet textures'] = 'Bolygótextúrák',
		['Fractal detail'] = 'Fraktálok',
		['City detail level'] = 'Városrészletesség',
		--['Display nav tunnels'] = '',
		['Full screen'] = 'Teljes képernyő',
		['Use shaders'] = 'Árnyékok használata',
		--['Compress Textures'] = '',
		--['Video configuration (restart game to apply)'] = '',
		--['Mute'] = '',
		['Master:'] = 'Fő hangerő:',
		['Music:'] = 'Zene:',
		['Effects:'] = 'Effektek:',
		['Language (restart game to apply)'] = 'Nyelv (újraindítás szükséges)',
		--['Press a key or controller button'] = '',
		--['Change Binding'] = '',
		['Invert Mouse Y'] = 'Egérirányítás megfordítása',
		['Enable joystick'] = 'Joystick irányítás engedélyezése',
		--['Control Options'] = '',
		--['Set'] = '',
		--['Video'] = '',
		--['Sound'] = '',
		--['Language'] = '',
		['Controls'] = 'Irányítás',
		['Exit this game'] = 'Kilépés',

		-- Ship information
		-- ['Capacity used'] = '',
		['All-up weight'] = 'Teljes súly', -- total weight
		['Hyperdrive'] = 'Hipermotor',
		['Hyperspace range'] = 'Hiperűrtáv',
		['Front weapon'] = 'Elülső fegyver',
		['Rear weapon'] = 'Hátsó fegyver',

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

		-- Economy and Trade
		-- ['Jettison'] = '',
		-- ['Refuel'] = '',

		-- Missions
		['Missions'] = 'Küldetések',
		['Type'] = 'Típus',
		['Client'] = 'Kliens',
		['Location'] = 'Hely',
		['Due'] = 'Lejár',
		['Reward'] = 'Jutalom',
		['Status'] = 'Státusz',
		['ACTIVE'] = 'Aktív',
		['COMPLETED'] = 'Befejezve',
		['FAILED'] = 'Nem sikerült',
	}
})

Translate:Add({
	Russian = {
		-- Main menu
		['Start at Earth']    = 'Новый старт: Earth',
		['Start at New Hope'] = 'Новый старт: New Hope',
		["Start at Barnard's Star"] = "Новый старт: Barnard's Star",
		['Load game']         = 'Загрузить запись',
		['Options']           = 'Настройки',
		['Quit']              = 'Выход',

		-- Generic file dialog
		['Select file...'] = 'Выберите файл...',
		['Select']         = 'Выбрать',
		['Cancel']         = 'Отмена',
		--['Ok']             = '',
		
		-- Load dialog
		['Select game to load...'] = 'Выберите файл для загрузки...',
		--['Load'] = '',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Выберите файл для записи или введите новое имя',
		['Save'] = 'Записать',

		-- Settings screen
		--['Return to menu'] = '',
		['Video resolution'] = 'Разрешение экрана',
		['Off'] = 'Выкл.',
		--['Multisampling'] = '',
		['Very low'] = 'Оч.низкая',
		['Low'] = 'Низкая',
		['Medium'] = 'Средняя',
		['High'] = 'Высокая',
		['Very high'] = 'Оч.высокая',
		['Planet detail distance'] = 'Детал.планет',
		['Planet textures'] = 'Текстуры планет',
		['Fractal detail'] = 'Детал.частиц',
		['City detail level'] = 'Детал.городов',
		['Display nav tunnels'] = 'Вкл. навигационный тоннель',
		['Full screen'] = 'Включить полноэкранный режим',
		['Use shaders'] = 'Использовать шейдеры',
		['Compress Textures'] = 'Сжатие текстур',
		--['Video configuration (restart game to apply)'] = '',
		--['Mute'] = '',
		['Master:'] = 'Общая:',
		['Music:'] = 'Музыка:',
		['Effects:'] = 'Эффекты:',
		['Language (restart game to apply)'] = 'Язык игры (треб.перезапуск игры)',
		--['Press a key or controller button'] = '',
		--['Change Binding'] = '',
		['Invert Mouse Y'] = 'Инвертировать мышь по оси Y',
		['Enable joystick'] = 'Включить джойстик',
		--['Control Options'] = '',
		--['Set'] = '',
		--['Video'] = '',
		--['Sound'] = '',
		--['Language'] = '',
		['Controls'] = 'Управление',
		['Exit this game'] = 'Покинуть игру',

		-- Equipment
		['Equipment'] = 'Оборудование',
		['{quantity} Shield Generators'] = 'Генераторы защитного поля {quantity}',
		['{quantity} Occupied Passenger Cabins'] = 'Занятых пассажирских кают {quantity}',
		['{quantity} Unoccupied Passenger Cabins'] = 'Свободных пассажирских кают {quantity}',
		
		-- Ship Information
		['Ship Information'] = 'Информация о корабле',
		['Capacity used'] = 'Занято в грузовом отсеке',
		['All-up weight'] = 'Общий вес корабля и груза', -- total weight
		['Weight empty:'] = 'Собств.вес корабля:',
		['Hyperdrive'] = 'Установленный двигатель',
		['Hyperspace range'] = 'Возможный радиус прыжка',
		['{range} light years ({maxRange} max)'] = '{range} св.лет (из {maxRange} макс.)',
		['Minimum crew'] = 'Минимальный экипаж',
		['Crew cabins'] = 'Каюты экипажа',
		['free'] = 'свободно',
		['max'] = 'макс.',
		['Front weapon'] = 'Носовая турель',
		['Rear weapon'] = 'Кормовая турель',
		
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
		['Jettison'] = 'Сброс груза',
		['Refuel'] = 'Дозаправиться',

		-- Missions
		['Missions'] = 'Список заданий',
		['Mission Details'] = 'О задании',
		['No missions.'] = 'Нет заданий.',
		["%d days left"] = "Осталось дней: %d",
		['Type'] = 'Тип',
		['Client'] = 'Заказчик',
		['Location'] = 'Система',
		['Due'] = 'Сроки',
		['Reward'] = 'Награда',
		['Status'] = 'Статус',
		['ACTIVE'] = 'Активно',
		['COMPLETED'] = 'Завершено',
		['FAILED'] = 'Провалено',
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
		["Start at Barnard's Star"] = "Starte auf Barnard's Star",
		['Load game']         = 'Lade Spiel',
		['Options']           = 'Optionen',
		['Quit']              = 'Beenden',

		-- Generic file dialog
		['Select file...'] = 'Datei auswählen...',
		['Select']         = 'Auswählen',
		['Cancel']         = 'Abbrechen',
		--['Ok']             = '',

		-- Load dialog
		['Select game to load...'] = 'Spiel zum Laden auswählen...',
		--['Load'] = '',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Wählen Sie den Dateinamen',
		['Save'] = 'Speichern',

		-- Settings screen
		--['Return to menu'] = '',
		['Video resolution'] = 'Grafikauflösung',
		['Off'] = 'Aus',
		--['Multisampling'] = '',
		['Very low'] = 'Sehr niedrig',
		['Low'] = 'Niedrig',
		['Medium'] = 'Mittel',
		['High'] = 'Hoch',
		['Very high'] = 'Sehr hoch',
		['Planet detail distance'] = 'Detailentfernung für Planeten',
		['Planet textures'] = 'Planeten Texturen',
		['Fractal detail'] = 'Detailstufe für Fraktale',
		['City detail level'] = 'Detailstufe für Städte',
		['Display nav tunnels'] = 'Navigationstunnel anzeigen',
		['Full screen'] = 'Vollbildmodus',
		['Use shaders'] = 'Pixel-Shader aktivieren',
		['Compress Textures'] = 'Texturen komprimieren',
		--['Video configuration (restart game to apply)'] = '',
		--['Mute'] = '',
		['Master:'] = 'Master:',
		['Music:'] = 'Musik:',
		['Effects:'] = 'Effekte:',
		['Language (restart game to apply)'] = 'Sprachauswahl (Programm muss neu gestartet werden)',
		--['Press a key or controller button'] = '',
		--['Change Binding'] = '',
		['Invert Mouse Y'] = 'Maus Y-Achse invertieren',
		['Enable joystick'] = 'Joystick-Steuerung aktivieren',
		--['Control Options'] = '',
		--['Set'] = '',
		--['Video'] = '',
		--['Sound'] = '',
		--['Language'] = '',
		['Controls'] = 'Steuerung',
		['Exit this game'] = 'Spiel beenden',

		-- Equipment
		['Equipment'] = 'Ausrüstung',
		['{quantity} Shield Generators'] = '{quantity} Schild-Generatoren',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} Belegte Passagierkabinen',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} Unbelegte Passagierkabinen',
		
		-- Ship Information
		['Ship Information'] = 'Schiffs-Informationen',
		['Capacity used'] = 'Ausgelastete Kapazität',
		['All-up weight'] = 'Gesamtgewicht', -- total weight
		['Weight empty:'] = 'Leergewicht:',
		['Hyperdrive'] = 'Hyperraumantrieb',
		['Hyperspace range'] = 'Hyperraum Reichweite',
		['{range} light years ({maxRange} max)'] = '{range} Lichtjahre ({maxRange} max)',
		['Front weapon'] = 'Vorne',
		['Rear weapon'] = 'Achtern',
		
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
		['Jettison'] = 'Abwerfen',
		['Refuel'] = 'Auftanken',

		-- Missions
		['Missions'] = 'Missionen',
		['Mission Details'] = 'Missionsdetails',
		['No missions.'] = 'Keine Missionen.',
		['Type'] = 'Typ',
		['Client'] = 'Auftraggeber',
		['Location'] = 'Ort',
		['Due'] = 'Fällig',
		['Reward'] = 'Belohnung',
		['Status'] = 'Status',
		['ACTIVE'] = 'Offen',
		['COMPLETED'] = 'Abgeschlossen',
		['FAILED'] = 'Gescheitert',
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
		["Start at Barnard's Star"] = "Start: Barnard's Star",
		['Load game']         = 'Načíst hru',
		['Options']           = 'Možnosti',
		['Quit']              = 'Ukončit',

		-- Generic file dialog
		['Select file...'] = 'Otevřít soubor...',
		['Select']         = 'Otevřít',
		['Cancel']         = 'Zrušit',
		--['Ok']             = '',

		-- Load dialog
		['Select game to load...'] = 'Vyber uloženou hru...',
		--['Load'] = '',

		-- Save dialog
		['Select a file to save to or enter a new filename'] = 'Vyber soubor do kterého uložit, nebo zadej nový název',
		['Save'] = 'Uložit',

		-- Settings screen
		--['Return to menu'] = '',
		['Video resolution'] = 'Rozlišení obrazovky',
		['Off'] = 'Vypnuto',
		--['Multisampling'] = '',
		['Very low'] = 'Velmi nízké',
		['Low'] = 'Nízké',
		['Medium'] = 'Střední',
		['High'] = 'Vysoké',
		['Very high'] = 'Velmi vysoké',
		['Planet detail distance'] = 'Detaily planet',
		['Planet textures'] = 'Textury planet',
		['Fractal detail'] = 'Detaily fraktálů',
		['City detail level'] = 'Detaily měst',
		['Display nav tunnels'] = 'Zobrazovat navigační tunel',
		['Full screen'] = 'Celá obrazovka',
		['Use shaders'] = 'Použít shadery',
		['Compress Textures'] = 'Komprimovat textury',
		--['Video configuration (restart game to apply)'] = '',
		--['Mute'] = '',
		['Master:'] = 'Hlavní:',
		['Music:'] = 'Hudba:',
		['Effects:'] = 'Efekty:',
		['Language (restart game to apply)'] = 'Jazyk (nutný restart hry)',
		--['Press a key or controller button'] = '',
		--['Change Binding'] = '',
		['Invert Mouse Y'] = 'Invertovat osu Y',
		['Enable joystick'] = 'Zapnout ovládání joystickem',
		--['Control Options'] = '',
		--['Set'] = '',
		--['Video'] = '',
		--['Sound'] = '',
		--['Language'] = '',
		--['Controls'] = '',
		['Exit this game'] = 'Ukončit hru',

		-- Equipment
		['Equipment'] = 'Vybavení',
		['{quantity} Shield Generators'] = '{quantity} generátory štítů',
		['{quantity} Occupied Passenger Cabins'] = '{quantity} obsazených pasažérských kabin',
		['{quantity} Unoccupied Passenger Cabins'] = '{quantity} volných pasažérských kabin',

		-- Ship Information
		['Ship Information'] = 'Informace o lodi',
		['Capacity used'] = 'Využitá kapacita',
		['All-up weight'] = 'Celková hmotnost', -- total weight
		['Weight empty:'] = 'Prázdná váha:',
		['Hyperdrive'] = 'Hypermotor',
		['Hyperspace range'] = 'Hyperprostorový dosah',
		['{range} light years ({maxRange} max)'] = '{range} ly ({maxRange} max)',
		['Minimum crew'] = 'Minimální posádka',
		['Crew cabins'] = 'Kabin pro posádku',
		['free'] = 'volných',
		['max'] = 'max',
		['Front weapon'] = 'Přední zbraň',
		['Rear weapon'] = 'Zadní zbraň',

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
		['Jettison'] = 'Odhodit',
		['Refuel'] = 'Natankovat',

		-- Missions
		['Missions'] = 'Mise',
		['Mission Details'] = 'Detaily mise',
		['No missions.'] = 'Žádné mise.',
		["%d days left"] = "Zbývá dnů: %d",
		['Type'] = 'Typ',
		['Client'] = 'Klient',
		['Location'] = 'Pozice',
		['Due'] = 'Splatnost',
		['Reward'] = 'Odměna',
		['Status'] = 'Status',
		['ACTIVE'] = 'Probíhá',
		['COMPLETED'] = 'Dokončený',
		['FAILED'] = 'Neúspěšný',
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
