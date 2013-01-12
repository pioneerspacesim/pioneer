-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

  -- adtext - text shown in the bulletin board list
  -- introtext - shown when the advert is selected (and "Could you repeat request?")
  -- expeditiontext - response to "Why so much money?"
  -- howmany - response to "How many of you are there?"
  -- danger - repsponse to "Will I be in any danger?"
  -- successmsg - message sent on successful taxi service
  -- failuremsg - message sent on failed taxi service after docking
  -- wherearewe - message sent on failed taxi service after entering system
  -- planetarysurvey - 1 if the taxi service is for planetarysurvey person
  -- urgency - how urgent the transport is
  -- risk - how risky the mission is. 0 is none. 1 is certain death

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Large ship with an experienced captain for an expedition to the uncharted system: {system}. You will be paid cash.",
  introtext = "Hello, my name is {name}, representative of the {corporation} corporation. We are funding a scientific expedition to: {system} ({sectorx}, {sectory}, {sectorz}) system, at a distance of {dist} ly.  We will pay {cash}.",
  expeditiontext = "This will be a {months} month expedition. The team consists of me and {passengers} researchers and our {tons}t of research equipment. We are hoping to find {element} on one of the planets. If you should accept, know that I am the leader of this expedition. I am in charge and you will follow all orders given to you by me on behalf of {corporation} until the duration of the mission is up.",
  danger = "Not likely. However, there are always dangers in uncharted space and your ship should be suitably prepared.",
  scientist = "{corporation} Overseer"
  successmsg = "Thank you for the nice trip and thank you on behalf of the {corporation} corporation. You have been paid in full.",
  failuremsg = "{Corporation} does not accept laziness. You will be paid half of the agreed upon amount.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "EXPEDITION: {system} system. {cash} contract.",
  introtext = "Hi, I'm {name}. I am a biologist and my team has recieved a grant to explore the possibilities of life in {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "This will be a {months} month expedition. The team consists of me and {passengers} assistants and our {tons}t of equipment. We will take atmosphere samples as well as a soil sample from planets in the system.",
  danger = "Not likely. Just think though, perhaps the system is inhabited. Who knows what we could find!",
  scientist = "{corporation} Head Researcher",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. You will be paid half of the amount",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Large ship. Expedition to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name}. {corporation} is funding a research trip to {system} ({sectorx}, {sectory}, {sectorz}) system, at a distance of {dist} ly. They have hired me to put together the crew and I need an experienced pilot. They will pay {cash}.",
  expeditiontext = "This will be a {months} month journey. There are {passengers} passengers in total and we have {tons}t of equipment and supplies. We need to collect soil samples from planets in the system. What we gather will depend on what we find in the system. ",
  danger = "No. This should be peaceful journey.",
   scientist = "{corporation} Head Scientist"
  successmsg = "Thanks for bringing us here. We've now paid in full. Good Luck!",
  failuremsg = "Useless! {corporation} will have my ass! You took forever. You will not recieve the full amount.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "BRAVE CAPTAIN NEEDED: Looking for adventure? Inquire inside.",
  introtext = "Hi, I'm {name} and I am putting together a crew for a voyage to the uncharted system, {system} ({sectorx}, {sectory}, {sectorz}),  distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "I am hiring on behalf of the {corporation} corporation.  The expedition will be {months} months long and consists of me,  my crew of {passengers}, and our {tons}t of equipment and supplies. We are looking for ... samples from the planets in the system.",
  danger = "Danger? Why would there be any danger? {corporation} guarantees the safety of all of it's employees. Although, we are not responsible for any death and/or dismemberment that may result whilst under employment.",
  scientist = "Head of Personnel , {corporation}"  
  successmsg = "Thank you for the ship and the fast journey. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you the full amount.",
  wherearewe = "Where are we? We've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 0,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name}. I'm with an organization in the business of colonizing new star systems and we have found a system we think may have viable planets. We need planetary data and samples. System is, {system} ({sectorx}, {sectory}, {sectorz}) , at a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "I'm traveling salesman.",
  danger = "No.",
  scientist = 
  successmsg = "Thanks for the samples! I've paid in full. Good Luck!",
  failuremsg = "Useles.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "Didn't you know - I'm a well known dream star.",
  danger = "You might get some interest from the press. Just ignore them.",
  scientist = 
  corporation = 
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "What have you done! My tour is all spoilt and I lost half of my fans.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "I'm freelance journalist.",
  danger = "No.",
  scientist = 
  corporation = 
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Safe passage to {system} system. Will pay {cash}.",
  introtext = "Hi, my name is {name} and I need safe passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  expeditiontext = "The Mafia want me dead.",
  danger = "The Mafia don't take kindly to people helping their enemies.",
  scientist = 
  corporation =  
  successmsg = "Thanks for carrying me safely here. I've paid in full. Good Luck!",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I'm visiting a sick relative.",
  danger = "No.",
  scientist = 
  corporation =  
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "The Police want me to help them with their enquiries.",
  danger = "The Police may try to stop you.",
  scientist = 
  corporation =  
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Useless! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I want fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Paying {cash}.",
  expeditiontext = "I would rather someone didn't find me.",
  danger = "I think someone is following me.",
  scientist = 
  corporation = 
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "You are so inexperienced pilot. Not going to pay for this.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "FAST SHIP: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I'm a factory inspector doing my rounds.",
  danger = "Sometimes people don't want to be inspected.",
  scientist = 
  corporation = 
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "I'm going to lost my job because of your incompetence. So I need that money more than you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('English','Expedition', {
  adtext = "SHIP REQUIRED: Passage to {system} system.",
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  expeditiontext = "I owe someone some money, and they're after me.",
  danger = "Someone is chasing me.",
  scientist = 
  corporation = "
  successmsg = "Thank you for the ride. You have been paid in full.",
  failuremsg = "I do not have enough money. Sorry.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  planetarysurvey = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ English = {
  ["Expedition"] = "Expedition",
  ["Why so much money?"] = "Why so much money?",
  ["What kind of expedition are we talking about?"] = "What kind of expedition are we talking about?",
  ["You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t."] = "You expect us to take you seriously flying that puddle jumper? Please come back when you own a ship larger than 120t.",
  ["When will we arrive back?"] = "When will we arrive back?",
  ["Will I be in any danger?"] = "Will I be in any danger?",
  ["I must be there before "] = "I must be there before ",
  ["We want to be there before "] = "We want to be there before ",
  ["You do not have enough cabin space on your ship."] = "You do not have enough cabin space on your ship.",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["Ok, agreed."] = "Ok, agreed.",
  ["Hey!?! You are going to pay for this!!!"] = "Hey!?! You are going to pay for this!!!",
  ["ly"] = "ly",

  -- Texts for the missions screen
  taximissiondetail = [[
  From:
  To:
  Group details:
  Danger:
  Deadline:

  Distance:]],

 PIRATE_TAUNTS = {
	"You're going to regret dealing with {client}",
	"You have {client} on board? That was a bad idea.",
	"Today isn't your lucky day! Prepare to die.",
	"You're not going to dock today!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 "Cool Cola",
	 "Aquarian Shipbuilding",
	 "Weyland",
	 "Arment Aerodynamics",
	 "Bulk Ships",
	"Digital Inc",
	"Vega Line",
	"Marett Space",
	 "Amaliel",
	 "ACME",
	 "Rockforth",
  },
}, })

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','Expedition', {
  adtext = "POTRZEBNY TRANSPORT: Przelot malej grupy do systemu {system} za {cash}.",
  introtext = "Czesc, nazywam sie {name} i szukam transportu dla malej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Placimy {cash}.",
  expeditiontext = "Odwiedzamy przyjaciela.",
  "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziekuje za przyjemna podróz. Placimy calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplacimy.",
  wherearewe = "Gdzie my jestesmy? Nasza cierpliwosc sie wyczerpala - natychmiast zabierz nas do najblizszej stacji!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "POTRZEBNY TRANSPORT: Przelot malej grupy do systemu {system} za {cash}.",
  introtext = "Czesc, nazywam sie {name} i szukam transportu dla malej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Placimy {cash}.",
  expeditiontext = "Pracujemy dla korporacji {corp} i to oni pokrywaja koszty.",
  "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziekuje za przyjemna podróz. Placimy calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplacimy.",
  wherearewe = "Gdzie my jestesmy? Nasza cierpliwosc sie wyczerpala - natychmiast zabierz nas do najblizszej stacji!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "POTRZEBNY TRANSPORT: Przelot malej grupy do systemu {system}. Placimy {cash}.",
  introtext = "Czesc, nazywam sie {name} i szukam transportu dla malej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Placimy {cash}.",
  expeditiontext = "To zwykla podróz sluzbowa.",
  "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziekuje za przyjemna podróz. Placimy calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplacimy.",
  wherearewe = "Gdzie my jestesmy? Nasza cierpliwosc sie wyczerpala - natychmiast zabierz nas do najblizszej stacji!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Dawny rywal stara sie mnie zabic.",
  "Tylko ja.",
  danger = "Mysle ze platny morderca jest na moim tropie, mozliwe ze go spotkamy.",
  successmsg = "Dziekuje za przyjemna podróz. Place calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Jestem komiwojazerem",
  "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dzieki za podwiezienie. Place calosc umówionej sumy. Powodzenia!",
  failuremsg = "Nawet nie pytaj o zaplate! Zglosze to w odpowiednim urzedzie.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Nie wiesz? Jestem znana gwiazda!",
  "Tylko ja.",
  danger = "Mozesz spotkac sie z zainteresowaniem ze strony prasy. Po prostu ich ignoruj.",
  successmsg = "Dziekuje za przyjemna podróz. Place calosc umówionej sumy.",
  failuremsg = "Cos ty narobil! Zniszczyles moje tournée, stracilem polowe fanów.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Jestem niezaleznym dziennikarzem.",
  "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dziekuje za przyjemna podróz. Place calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Mafia chce mojej smierci.",
  "Ja i nikt wiecej.",
  danger = "Mafia niezbyt lubi ludzi którzy pomagaja jej wrogom.",
  successmsg = "Dziekuje za bezpieczna podróz. Place calosc umówionej sumy. Powodzenia!",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, musze szybko dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Lece do chorego krewnego.",
  "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dziekuje za szybki transport. Place calosc umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróz trwala wiecznosc. Nic ci nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, musze szybko dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Policji zalezy na przesluchaniu mnie.",
  "Tylko ja.",
  danger = "Policja moze próbowac cie powstrzymac.",
  successmsg = "Dziekuje za szybki transport. Place calosc umówionej sumy.",
  failuremsg = "Bezuzyteczny! Podróz trwala wiecznosc. Nic ci nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, musze szybko dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Musze sie ukryc.",
  "Jedna.",
  danger = "Mysle ze ktos mnie sledzi.",
  successmsg = "Dziekuje za szybki transport. Place calosc umówionej sumy.",
  failuremsg = "Widac ze jestes jeszcze niedoswiadczonym pilotem. Nic nie zaplace.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Place {cash}.",
  introtext = "Czesc, nazywam sie {name}, musze szybko dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Jestem inspektorem pracy w delegacji.",
  "Tylko jedna.",
  danger = "Czasami ludzie nie lubia byc sprawdzani.",
  successmsg = "Dziekuje za szybki transport. Place calosc umówionej sumy.",
  failuremsg = "Przez twoja niekompetencje stracilem prace. Wiec potrzebuje tych pieniedzy bardziej niz ty.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Polski','Expedition', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Place {cash}.",
  introtext = "Nazywam sie {name}, chce dostac sie do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} ls. Place {cash}.",
  expeditiontext = "Szuka mnie ktos od kogo pozyczylem troche pieniedzy.",
  "Tylko jedna.",
  danger = "Ktos próbuje mnie znalezc.",
  successmsg = "Dzieki za podwiezienie. Place calosc umówionej sumy.",
  failuremsg = "Przepraszam ale nie mam tyle pieniedzy.",
  wherearewe = "Gdzie my jestesmy? Moja cierpliwosc sie wyczerpala - natychmiast zabierz mnie do najblizszej stacji!",
  planetarysurvey = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Polski = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Dlaczego tyle pieniedzy?",
  ["How many of you are there?"] = "Ile osób jest w grupie?",
  ["How soon you must be there?"] = "Kiedy musisz tam dotrzec?",
  ["Will I be in any danger?"] = "Beda jakies problemy?",
  ["I must be there before "] = "musze tam byc przed ",
  ["We want to be there before "] = "chcemy tam dotrzec przed ",
  ["You do not have enough cabin space on your ship."] = "Na twoim statku brakuje kabin pasazerskich.",
  ["Could you repeat the original request?"] = "Mozesz powtórzyc swoja oferte?",
  ["Ok, agreed."] = "Zgoda.",
  ["Hey!?! You are going to pay for this!!!"] = "Hej!?! Zaplacisz za to!!!",
  ["ly"] = "ls",

  -- Texts for the missions screen
  taximissiondetail = [[
  Z:
  Do:
  Pasazerów:
  Zagrozenie:
  Termin:

  Dystans:]],

 PIRATE_TAUNTS = {
	"Pozalujesz kontaktów z {client}",
	"Masz na pokladzie {client}? To byl zly pomysl.",
	"To nie jest twój szczesliwy dzien! Przygotuj sie na smierc.",
	"Tym razem nie dotrzesz do doku!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 "Cool Cola",
	 "Taranis",
	 
	 "Rockforth",
	 "Amaliel",
	 "Marett Space",
	 "Vega Line",
	 "Digital",
	 "Bulk Ships",
	 "Arment Aerodynamics"
  },
}, })

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA: Pasaje para un pequeño grupo al Sistema {system}. Se pagará {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje para un grupo pequeño al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "Vamos a visitar a un amigo.",
  "Seremos {group}.",
  danger = "No.",
  successmsg = "Gracias por el agradable viaje. Se le ha pagado al completo.",
  failuremsg = "Inaceptable! ha tardado una eternidad. No tenemos intención de pagarle.",
  wherearewe = "Dónde estamos? Ya hemos esperado demasiado - llévenos a la estación mas cercana AHORA!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA: Pasaje para un grupo pequeño al Sistema {system}. Se abonará {cash}.",
  introtext = "Saludos, Soy {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Abonaré {cash}.",
  expeditiontext = "Trabajamos para la corporación {corp} y ellos pagan.",
  "Somos {group}.",
  danger = "No.",
  successmsg = "Gracias por el agradable trayecto. Se le ha pagado al completo.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tenemos intención de pagarle.",
  wherearewe = "Dónde nos encontramos? Ya hemos esperado suficiente - llévenos a la estación mas cercana DE INMEDIATO!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA: Pasaje para un grupo pequeño al Sistema {system}. Se pagarán {cash}.",
  introtext = "Hola, Mi nombre es {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "Un viaje de negocios rutinario.",
  "Somos {group}.",
  danger = "No.",
  successmsg = "Gracias por traernos. Le hemos pagado al completo. Buena suerte!",
  failuremsg = "Inaceptable! Le ha llevado una eternidad. No vamos a pagarle.",
  wherearewe = "Dónde nos encontramos? Ya hemos esperado bastante - déjenos en la estación mas cercana YA!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE REQUIERE NAVE: Pasaje al Sistema {system}. Se pagará {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje al sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Le pagaré {cash}.",
  expeditiontext = "Un antiguo enemigo trata de liquidarme.",
  "Soy solo yo.",
  danger = "Creo que hay un asesino tras de mi, y posiblemente podría ir tras usted.",
  successmsg = "Gracias por el agradable viaje. Se le pagado al completo.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tengo intención alguna de pagarle.",
  wherearewe = "Dónde estamos? Ya he esperado suficiente - lléveme a la estación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA NAVE: Pasaje al Sistema {system}. Se abonarán {cash}.",
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "Soy viajante.",
  "Yo solo.",
  danger = "No.",
  successmsg = "Gracias por traerme. Le he pagado lo acordado. Buena suerte!",
  failuremsg = "Ni pregunte por el pago! Voy a dar parte de usted a las autoridades!.",
  wherearewe = "Dónde nos encontramos? Ya he esperado suficiente - déjeme en la estación mas cercana YA!",
  planetarysurvey = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA: Pasaje al Sistema {system}. Se paga {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "Ah, no lo sabías - Soy una conocida Estrella.",
  "Soy solo yo.",
  danger = "Podrías despertar cierto interés de la prensa. Símplemente ignórales.",
  successmsg = "Gracias por el agradable trayecto. Se le ha pagado lo acordado.",
  failuremsg = "Pero qué has hecho! Mi Tour se ha arruinado y he perdido a la mitad de mis fans!.",
  wherearewe = "Dónde estamos? He esperado ya demasiado - llévame a la estación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA NAVE: Pasaje al Sistema {system}. Se abonará {cash}.",
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Abonaré {cash}.",
  expeditiontext = "Soy periodista autónomo.",
  "Soy solo yo.",
  danger = "No.",
  successmsg = "Gracias por el agradable crucero. Se le ha pagado el total.",
  failuremsg = "Inaceptable! Le ha llevado una eternidad!. No tengo intención alguna de pagarle!.",
  wherearewe = "Dónde os encontramos? Ya he esperado bastante - lléveme a la estación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE REQUIERE NAVE: Pasaje seguro al Sistema {system}. Pagaré {cash}.",
  introtext = "Hola, mi nombre es {name} y necesito pasaje seguro al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Le pagaré {cash}.",
  expeditiontext = "La Mafia me quiere fiambre.",
  "Yo y nadie mas.",
  danger = "A la Mafia no le sienta bien que alguien ayude a sus enemigos.",
  successmsg = "Gracias por traerme con seguridad. Le he pagado el total. Buena suerte!",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No pienso pagarle!.",
  wherearewe = "Pero dónde estamos? Ya he aguardado bastante - lléveme a la estación mas cercana YA!",
  planetarysurvey = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA NAVE: Pasaje en una nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "Voy a visitar a un familiar enfermo.",
  "Solo yo.",
  danger = "No.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total acordado.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tengo intención de pagar.",
  wherearewe = "Pero dónde nos encontramos? Ya he esperado bastante - déjeme en la estación mas cercana DE INMEDIATO!",
  planetarysurvey = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA NAVE: Pasaje en nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  expeditiontext = "La Policiá quiere que colabore en una investigación.",
  "Yo solo.",
  danger = "Puede que la Policiía trate de pararle.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total.",
  failuremsg = "Inútil! Has tardado una eternidad. No tengo intención de pagar.",
  wherearewe = "Dónde estamos? Ya he esperado bastante - llévame a la estación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE BUSCA NAVE: Pasaje en una nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Quiero pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pago {cash}.",
  expeditiontext = "Me gustaría que nadie me encontrara.",
  "Solo uno.",
  danger = "Creo que alguien me persigue.",
  successmsg = "Gracias por el rápido trayecto. Se le ha pagado el total.",
  failuremsg = "Eres un piloto novato. No voy a pagar por esto.",
  wherearewe = "Dónde estamos? Ya he esperado bastante - lléveme a la estación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "NAVE RÁPIDA: Pasaje en nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Se pagarán {cash}.",
  expeditiontext = "Soy Inspector de Fábricas haciendo mi ronda.",
  "Solo uno.",
  danger = "A veces la gente no gusta de inspecciones oficiales.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total.",
  failuremsg = "Voy a perder el trabajo por su incompetencia!. Ahora necesito el dinero más que usted!.",
  wherearewe = "Dónde estamos? Ya he esperado demasiado - déjeme en la estación mas cercana YA!",
  planetarysurvey = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Spanish','Expedition', {
  adtext = "SE REQUIERE NAVE: Pasaje al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Se le pagará {cash}.",
  expeditiontext = "Le debo pasta a alguien, y va tras de mi.",
  "Solo uno.",
  danger = "Alguien anda tras de mi.",
  successmsg = "Gracias por el viaje. Se le ha pagado el total.",
  failuremsg = "No tengo suficiente dinero. Lo siento.",
  wherearewe = "Dónde estamos? Ya he esperado demasiado - lléveme a laestación mas cercana AHORA!",
  planetarysurvey = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Spanish = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Por qué tanto dinero?",
  ["How many of you are there?"] = "Cuántos son?",
  ["How soon you must be there?"] = "Cuándo debe estar allí?",
  ["Will I be in any danger?"] = "Estaré en peligro?",
  ["I must be there before "] = "Debo estar allí antes de ",
  ["We want to be there before "] = "Queremos estar allí antes de ",
  ["You do not have enough cabin space on your ship."] = "No dispone de suficiente espacio en la cabina de su nave.",
  ["Could you repeat the original request?"] = "Podría repetir la petición original?",
  ["Ok, agreed."] = "Ok, de acuerdo.",
  ["Hey!?! You are going to pay for this!!!"] = "Ey!?! Vas a pagar por esto!!!",
 PIRATE_TAUNTS = {
	"Vas a lamentar haber hecho negocios con {client}",
	"Tienes a {client} a bordo? Eso fue una mala idea.",
	"Hoy no es tu día de suerte! Prepárate para morir.",
	"Hoy no vas a atracar!",
  },
 CORPORATIONS = {
	 "Sirius",
	 
	 "Cool Cola",
	 "Taranis",
	 "Astilleros Aquarian",
	 "Rockforth",
	 "Amaliel",
	 "Marett Space",
	 "Vega Line",
	 "Digital",
	 "Bulk Ships",
	 "Arment Aerodynamics"
  },
}, })



---- HUNGARIAN / MAGYAR ----

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "KERESÜNK: Egy kisebb csoportot {system} rendszerbe elszállító hajót. A fizetség {cash} kredit.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. A megbízásod fizetsége {cash} kredit.",

  expeditiontext = "Egy barátunkat látogatjuk meg.",
  "{group} személyrol van szó.",
  danger = "Nem.",



  successmsg = "Köszönjük az utat. Már ki is fizettünk.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Örökké! Nem fogunk még fizetni is érte!",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötobe!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "KERESÜNK: Utazási lehetoséget egy kisebb csoport számára a(z) {system} rendszerbe. Fizetünk érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

  expeditiontext = "Mi a(z) {corp} vállalatnak dolgozunk, és ok fizetik az utat.",
  "{group} személyrol van szó.",
  danger = "Nem.",



  successmsg = "Köszönjük az utat. Már ki is fizettünk.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Örökké! Nem fogunk még fizetni is érte!",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötobe!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "KERESÜNK: Utazási lehetoséget egy kisebb csoport számára a(z) {system} rendszerbe. Fizetünk érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

  expeditiontext = "Csak egy egyszeru üzleti út.",
  "{group} személyrol van szó.",
  danger = "Nem.",


  successmsg = "Köszönjük, hogy elhoztál. Már ki is fizettünk. Sok szerencsét a továbbiakban.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Nem fogunk még fizetni is érte.",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötobe!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓ KERESTETIK: Hajót keresek utazáshoz a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. El szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  expeditiontext = "Egy régi riválisom meg akar ölni.",
  "Csak én vagyok.",


  danger = "Úgy gondolom, hogy egy orgyilkos van a nyomomban, szóval így téged is megtámadna, amíg engem szállítasz.",
  successmsg = "Köszönöm az utat. Már ki is fizettem a járandóságod.",
  failuremsg = "Ez elfogadhatatlan! Már mióta úton vagyunk. Nem fogok neked fizetni.",
  wherearewe = "Hol a francban vagyunk? Már eleget vártam - azonnal vigyél a legközelebbi kikötobe!",
  planetarysurvey = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓ KERESTETIK: Hajót keresek utazáshoz a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}, szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",
  expeditiontext = "Utazó ügynök vagyok.",
  "Csak én.",
  danger = "Nem.",


  successmsg = "Köszönöm a szállítást. Ki is fizettelek. Sok szerencsét a továbbiakban!",
  failuremsg = "Ne is kérd a fizetséged! Jelentelek a hatóságoknak.",
  wherearewe = "Hol vagyunk? Már eleget várakoztam - AZONNAL vigyél a legközelebbi kikötodokkba!",
  planetarysurvey = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "KERESEK: Utazási lehetoséget a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. El akarok jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  expeditiontext = "Nem is tudod - én egy ismert álomsztár vagyok.",
  "Csak én.",




  danger = "Talán a sajtó zaklatni fog miattam. Csak hagyd oket figyelmen kívül.",
  successmsg = "Kösz az utat. Már ki is vagy fizetve.",
  failuremsg = "Mit csináltál! A turnémnak annyi, és elveszítettem a rajongóim felét.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötobe AZONNAL!",
  planetarysurvey = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "KERESEK: Hajót, ami elszállít a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}, és szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek az útért {cash} kreditet.",
  expeditiontext = "Szabadúszó újságíró vagyok.",
  "Csak én vagyok.",
  danger = "Nem.",



  successmsg = "Köszönöm az utat. Már ki is fizettelek érte.",
  failuremsg = "Elfogadhatatlan, meddig tartott! Örökké. Ezért én nem fogok neked még fizetni is.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi urkikötobe!",
  planetarysurvey = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓT KERESEK: Hajót, amely biztonságban elvisz a(z) {system} rendszerbe {cash} kreditért cserébe.",
  introtext = "Helló, a nevem {name}. Szeretnék biztonságban eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  expeditiontext = "A maffia meg akar ölni.",
  "Én és senki más.",




  danger = "A maffia nem veszi jó néven, ha segíted az ellenségeit..",
  successmsg = "Köszönöm, hogy biztonságban elhoztál ide. Kifizettelek, további sok sikert!",
  failuremsg = "Ez elfogadhatatlan, hogy mennyi idot elvesztegettél. Nem fogok még fizetni is ezért.",
  wherearewe = "Hol a pokolban vagyunk? Már eleget várakoztam - vigyél AZONNAL a legközelebbi urkikötobe!",
  planetarysurvey = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓT KERESEK: Olyat, amely gyorsan elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek az útért {cash} kreditet.",

  expeditiontext = "Egy beteg rokonomat látogatom meg.",
  "Csak én.",
  danger = "Nem.",



  successmsg = "Köszönöm a gyors szállítást. Ki is fizettelek érte.",
  failuremsg = "Elfogadhatatlan! Már egy örökkévalóság eltelt az indulás óta. Nem fogok még fizetni is érte.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi urkikötobe!",
  planetarysurvey = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓT KERESEK: Olyat, amely gyorsan elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Szeretnék gyorsan eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  expeditiontext = "A rendorség megkért, hogy segítsek a nyomozásukban.",
  "Csak én.",




  danger = "A rendorség lehet hogy meg akar állítani.",
  successmsg = "Köszönöm a gyors utat. Kifizettelek érte.",
  failuremsg = "Használhatatlan! Már mióta jövünk. Nem fogok ezért még fizetni is.",
  wherearewe = "Hol a pokolban vagyunk? Már eleget várakoztam - vigyél AZONNAL a legközelebbi urkikötobe!",
  planetarysurvey = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓT KERESEK: Egy gyors hajót, amely elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek {cash} kreditet.",

  expeditiontext = "Szereném, ha egy bizonyos valaki nem találna rám.",
  "Csak egy.",




  danger = "Szerintem valaki követ engem.",
  successmsg = "Köszönöm a gyors utat. Azonnal ki is fizetlek érte.",
  failuremsg = "Annyira egy kezdo pilóta vagy, hogy nem várhatod el a fizetséget se.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi urkikötobe!",
  planetarysurvey = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "GYORS HAJÓT KERESEK: Egy olyat, amely elvinne a(z) {system} rendszerbe.",
  introtext = "Az én nevem {name}. Egy gyors utazást szeretnék a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  expeditiontext = "Csak a körutamat járom gyárvizsgálóként.",
  "Csak egy.",




  danger = "Néhányan nem akarják, hogy vizsgálódjak.",
  successmsg = "Köszönöm a gyors utat, máris kifizetlek érte.",
  failuremsg = "El fogom veszteni a munkámat a képzetlenséged miatt. Így pénzt sem várhatsz el, mert nekem nagyobb szükségem van rá.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötobe AZONNAL!",
  planetarysurvey = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Magyar','Expedition', {

  adtext = "HAJÓT KERESEK: Amely elvinne a(z) {system} rendszerbe.",
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",

  expeditiontext = "Tartozom valakinek pénzzel, és ezért üldöz engem.",
  "Csak egy.",




  danger = "Valaki üldöz engem.",
  successmsg = "Köszönöm a fuvart, máris kifizetlek.",
  failuremsg = "Nincs elég pénzem. Bocs.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötobe AZONNAL!",
  planetarysurvey = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Magyar = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Miért ennyi a díjazás?",
  ["How many of you are there?"] = "Hány személyrol van szó?",
  ["How soon you must be there?"] = "Milyen hamar kell odaérni?",
  ["Will I be in any danger?"] = "Lesz valamilyen veszély útközben?",
  ["I must be there before "] = "Oda kell érnem még elotte:",
  ["We want to be there before "] = "Oda kell érnünk ezen idopontig:",
  ["You do not have enough cabin space on your ship."] = "Nincs elég utaskabin a hajódon.",
  ["Could you repeat the original request?"] = "Megismételnéd az eredeti feladatot?",
  ["Ok, agreed."] = "Rendben, elvállalom.",
  ["Hey!?! You are going to pay for this!!!"] = "Hé?! Ezért fizetni fogsz!!",
 PIRATE_TAUNTS = {




	"Meg fogod bánni, hogy {client} megbízott téged!",
	"A fedélzeten van {client} ? Na, ez egy igen rossz ötlet.",
	"A mai nap nem a szerencsenapod. Készülj a halálra.",
	"Ma már nem fogsz sehol dokkolni!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 "Cool Cola",
	 "Taranis",
	 "Aquarian Shipbuilding",
	 "Rockforth",
	 "Amaliel",
	 "Marett Space",
	 "Vega Line",
	 "Digital",
	 "Bulk Ships",
	 "Arment Aerodynamics"
  },
}, })

  ---- RUSSIAN / ??????? ----

Translate:AddFlavour('Russian','Expedition', {
  adtext = "???? ?????????: ??? ???????? ????????? ?????? ? ??????? {system} ?? {cash}.",
  introtext = "??????! ???? ????? {name}.\n ? ??? ?????????, ????? ????????? ????????? ?????? ????? ? {system} ({sectorx}, {sectory}, {sectorz}).\n ?????????? {dist} ??.?., ?????? {cash}.",
  expeditiontext = "????? ????????? ?????.",
  "??? ? ?????? ????? {group}.",
  danger = "???.",
  successmsg = "??????? ?? ???????? ???????????! ?? ?????? ??? ????????????? ?????.",
  failuremsg = "??? ???????????! ????? ?????? ????????! ?? ???????????? ???????.",
  wherearewe = "??? ?? ?????????? ???? ???????? ??????? - ???????? ??? ?? ????????? ???????!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "???? ?????????: ??????? ????????? ?????? ? ??????? {system} ?? {cash}.",
  introtext = "?????? ????! ???? ????? {name}.\n ???? ?????? ???? ???????????, ????? ??????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ???????? {cash}.",
  expeditiontext = "?? ???????? ?? ??????? ?????????? {corp}. ??? ??????????? ????????? ??? ???????.",
  "??? ? ?????? ????? {group}.",
  danger = "???.",
  successmsg = "??????? ?? ???????? ???????????! ?????? ??? ??????????????.",
  failuremsg = "??? ???????????! ?? ?? ????????? ? ???? - ??? ?? ?? ??? ??????? ???.",
  wherearewe = "??? ???! ?? ??????? ???? ?????! - ???????? ??? ???-??????!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "???? ?????????: ??????? ????????? ?????? ? ??????? {system}. ???????? {cash}.",
  introtext = "???????????! ??? ??? {name}.\n ???? ?????? ???? ???????????, ????? ??????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ???????? {cash}.",
  expeditiontext = "??????? ??????? ???????.",
  "??? ? ?????? ????? {group}.",
  danger = "???.",
  successmsg = "???? ??????? ?????? ? ????! ?????? ??? ???????????.",
  failuremsg = "?? ????????! ?? ?? ?????????? ???? ??????.",
  wherearewe = "??? ?? ?????????? ??? ??????? ??? ???????????? ??????????? - ???????? ??? ?? ????? ???????!",
  planetarysurvey = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ? ??????? {system}. ????? {cash}.",
  introtext = "????????? ????????????? - {name}.\n ??? ????????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ????????? {cash}.",
  expeditiontext = "?????? ???? ???????? ??????? ???? - ? ????????, ??? ?? ??? ??????.",
  "?????? ?.",
  danger = "? ?????, ??? ? ???? ?? ?????? ??????? ?????? - ???????? ?? ?????????? ? ???.",
  successmsg = "???????! ?? ???????? ????. ????? ??? ??????????????.",
  failuremsg = "??? ???? ?????????? ? ?????? ???????????! ? ???????? ??? ????? ???????.",
  wherearewe = "???? ?? ????????? ?????????? ????????? ???? ? ????? ????!",
  planetarysurvey = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ?? ??????? {system}. ?????? {cash}.",
  introtext = "??????? ??????? ?????! ? {name}.\n ???? ??????????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ? ???? {cash}.",
  expeditiontext = "? ???????????.",
  "?????? ?.",
  danger = "???.",
  successmsg = "???????, ??? ????????! ????? ??? ??????????????. ?????!",
  failuremsg = "???? ?? ??????????? ? ???????! ? ?????? ? ??? ? ??????????????? ?????????!",
  wherearewe = "?????? ????? ?? ??? ??????? ??? ??? ??? ??????? -  ?????????? ????????? ???? ?? ?????-?????? ???????!",
  planetarysurvey = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???? ? ??????? {system}. ???? ????????? {cash}.",
  introtext = "??????-??????! ? {name}.\n ????????? ???? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ??????????? {cash}.",
  expeditiontext = "?? ??????, ??? ??? ? ????????? ??????! ? ???? ??? ?? ????????!",
  "?????? ?.",
  danger = "?????? ????? ????????? ?? ????. ?????? ??????????? ??.",
  successmsg = "???????, ??? ???????! ??? ???? ??????.",
  failuremsg = "??? ?? ???????! ???????? ??? ????? - ???????? ??????? ??????????? ?? ????!!",
  wherearewe = "??? ??? ?? ??????! ??? ?????-?? ??????! - ????????? ???? ?? ????????? ???????!",
  planetarysurvey = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ? ??????? {system}. ????? ????????? {cash}.",
  introtext = "???????????! ??? ??? {name}.\n ?????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ?????? {cash}.",
  expeditiontext = "? ??????????? ?????????, ????????? ?????? ?????????? ??? ??????????.",
  "?????? ?.",
  danger = "???.",
  successmsg = "??????? ?? ?????! ? ??????? ????? ?? ??? ????.",
  failuremsg = "???????! ???????? ?????????. ?????? ?? ???????.",
  wherearewe = "??? ??? ??? ???????? ??????? - ???????? ???? ?? ????????? ?????????????? ???????!",
  planetarysurvey = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ????? ??????????? ? ??????? {system}. ?????? {cash}.",
  introtext = "????????????, ? {name}.\n ?????? ????????? ???? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})?\n ?????????? {dist} ??.?., ???? ????????? {cash}.",
  expeditiontext = "????? ????? ???? ??????, ??? ?????????? ???????? ??? ???????.",
  "? ? ?????? ??????.",
  danger = "????? ?? ????? ?????, ??????? ???????? ?? ??????.",
  successmsg = "??????? ?? ?????????? ???????????. ??? ????????? ??????. ????? ???!",
  failuremsg = "???????, ?? ?? ??????? ????? ??????. ? ?? ????? ????????? ???. ????????.",
  wherearewe = "??? ??? ? ?? ???? ?????? ?????, ? ?????? ????? ?? ????????? ???????.",
  planetarysurvey = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ?????? ? ??????? {system}. ? ???? {cash}.",
  introtext = "????????????, ??? ??? {name}.\n ??? ???? ?????? ????????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ?????? {cash}.",
  expeditiontext = "? ???? ? ???????? ????????????",
  "?????? ????? ????.",
  danger = "???.",
  successmsg = "??????? ?? ??????? ????????! ??? ??? ?????.",
  failuremsg = "??? ?????-?? ??????... ?? ??????? ????? ?????? - ? ?? ???? ??? ???????.",
  wherearewe = "??? ??, ???? ??????? ??? ??????? ????? - ? ????? ?? ????????? ???????!",
  planetarysurvey = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ???????? ? ??????? {system}. ??????? {cash}.",
  introtext = "??????! ???? ???? {name}.\n ??? ???? ?????? ??????????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ???????? {cash}.",
  expeditiontext = "??????? ????? ????? ? ???? ????????.",
  "?????? ?.",
  danger = "??????? ????? ???????? ??????? ? ???.",
  successmsg = "???????, ????. ???, ? ???? ?????.",
  failuremsg = "??????? ??????! ?? ????????. ??? ???? ? ??????, ? ?? ?????!",
  wherearewe = "?????? ????? ?? ??? ??????? ???? ??????? ?????? ? ????? - ???????? ?? ????????? ???????.",
  planetarysurvey = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ??????????? ???????? ? ??????? {system}! ??????? {cash}.",
  introtext = "????????, ? {name}.\n ?????? ??????? ????????? ???? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})?\n ?????????? {dist} ??.?., ??? {cash}.",
  expeditiontext = "??? ????? ????????!",
  "?????? ????? ???? ?? ?????.",
  danger = "???-?? ?????? ?? ????!",
  successmsg = "??????? ?? ??????? ???????! ???, ?????? ????.",
  failuremsg = "??-????? ?? ?????? ?????????! ? ?? ????????? ??????? ???!",
  wherearewe = "???? ??? ??? ??? ??????? ?????? ? ???? - ?????????? ??????? ???? ?? ?????-?????? ???????!",
  planetarysurvey = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ??? ???????? ???????? ? ??????? {system}. ??? ??????????? - {cash}.",
  introtext = "?????? ????. ? {name}.\n ????????? ?????????? ??????? ??? ?????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ?? ?????? ????????? {cash}.",
  expeditiontext = "? ???????????? ????????? - ???????? ???????? ?????.",
  "? ???? ????.",
  danger = "?? ??? ???? ????? ????????.",
  successmsg = "????????? ?? ????????. ?????? ?????????? ?? ??? ????.",
  failuremsg = "??-?? ????? ???????????????? ? ??????? ??????. ????? ???????, ? ??????? ? ???????? ?????? ???.",
  wherearewe = "??? ?? ?????????? ?? ???????????? ???? - ????????? ???? ? ????????? ?????????.",
  planetarysurvey = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Russian','Expedition', {
  adtext = "??? ???????: ???? - ??????? ? ??????? {system}. ???? ?????? {cash}.",
  introtext = "?????? ???? {name}.\n ? ???? ??????? ? ??????? {system} ({sectorx}, {sectory}, {sectorz})\n ?????????? {dist} ??.?., ??????? {cash}.",
  expeditiontext = "????????? ?? ?????????, ?? ???? ????.",
  "?????? ?.",
  danger = "?????? ???? ????.",
  successmsg = "???????, ??? ??????????. ??? ??????.",
  failuremsg = "??? ????, ?? ? ???? ??? ??????? ?????.",
  wherearewe = "??? ?? ?????????? ??? ???????? ??????? - ?????????? ???????? ???? ?? ????????? ???????!",
  planetarysurvey = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Russian = {
  ["Taxi"] = "?????????",
  ["Why so much money?"] = "???? ?????????? ??????????",
  ["How many of you are there?"] = "??????? ??????? ? ????? ???????",
  ["How soon you must be there?"] = "????? ?? ?????? ???? ?? ??????",
  ["Will I be in any danger?"] = "??????????? ?????-???? ?????????",
  ["I must be there before "] = "? ???? ???? ??? ?? ??????? ",
  ["We want to be there before "] = "?? ????? ???? ??? ?? ??????? ",
  ["You do not have enough cabin space on your ship."] = "?? ????? ??????? ???????????? ???????????? ????.",
  ["Could you repeat the original request?"] = "?? ????? ?? ?? ????????? ???? ????????????",
  ["Ok, agreed."] = "??????, ????????????.",
  ["Hey!?! You are going to pay for this!!!"] = "??!! ?? ??????????? ??????? ?? ????!",
  ["ly"] = "??.???",

  -- Texts for the missions screen
  taximissiondetail = [[
  ?????? ????????:
  ????? ????????:
  ? ??????:
  ?????????:
  ??????? ????:
  
  ??????????:]],

 PIRATE_TAUNTS = {
	"?? ???????? ?? ???????? ? {client}!!",
	"? ??? ?? ????? {client}? ??? ???? ?????? ????!",
	"??????? ?? ???? ????! ???????? ? ??????.",
	"? ???? ??? ?? ?? ???????? ?? ???????!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 "Cool Cola",
	 "Taranis",
	 "Aquarian Shipbuilding",
	 "Rockforth",
	 "Amaliel",
	 "Marett Space",
	 "Vega Line",
	 "Digital",
	 "Bulk Ships",
	 "Arment Aerodynamics"
  },
}, })
