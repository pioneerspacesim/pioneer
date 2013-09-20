-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")

  -- adtext - text shown in the bulletin board list
  -- introtext - shown when the advert is selected (and "Could you repeat request?")
  -- whysomuch - response to "Why so much money?"
  -- howmany - response to "How many of you are there?"
  -- danger - repsponse to "Will I be in any danger?"
  -- successmsg - message sent on successful taxi service
  -- failuremsg - message sent on failed taxi service after docking
  -- wherearewe - message sent on failed taxi service after entering system
  -- single - 1 if the taxi service is for single person
  -- urgency - how urgent the transport is
  -- risk - how risky the mission is. 0 is none. 1 is certain death

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Taxi', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "We are visiting a friend.",
  howmany = "There are {group} of us.",
  danger = "No.",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "We work for {corp} corporation and they are paying.",
  howmany = "There are {group} of us.",
  danger = "No.",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "WANTED: Passage for a small group to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "It's normal business trip.",
  howmany = "There are {group} of us.",
  danger = "No.",
  successmsg = "Thanks for bringing us here. We've now paid in full. Good Luck!",
  failuremsg = "Unacceptable! You took forever. We are not willing to pay you.",
  wherearewe = "Where are we? We've waited enough - take us to the nearest station NOW!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP REQUIRED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "An old rival is trying to kill me.",
  howmany = "It's only me.",
  danger = "I think there is an assassin on my trail, and they may come after you.",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "I'm traveling salesman.",
  howmany = "Just me.",
  danger = "No.",
  successmsg = "Thanks for carrying me here. I've paid in full. Good Luck!",
  failuremsg = "Don't even ask for payment! I'm reporting you to the authorities.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "Didn't you know - I'm a well known dream star.",
  howmany = "It's only me.",
  danger = "You might get some interest from the press. Just ignore them.",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "What have you done! My tour is all spoilt and I lost half of my fans.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Passage to {system} system. Will pay {cash}.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "I'm freelance journalist.",
  howmany = "It's only me.",
  danger = "No.",
  successmsg = "Thank you for the nice trip. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Safe passage to {system} system. Will pay {cash}.",
  introtext = "Hi, my name is {name} and I need safe passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "The Mafia want me dead.",
  howmany = "Me and nobody else.",
  danger = "The Mafia don't take kindly to people helping their enemies.",
  successmsg = "Thanks for carrying me safely here. I've paid in full. Good Luck!",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  whysomuch = "I'm visiting a sick relative.",
  howmany = "Only me.",
  danger = "No.",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  whysomuch = "The Police want me to help them with their enquiries.",
  howmany = "Only me.",
  danger = "The Police may try to stop you.",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "Useless! You took forever. I'm not willing to pay you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I want fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Paying {cash}.",
  whysomuch = "I would rather someone didn't find me.",
  howmany = "Just one.",
  danger = "I think someone is following me.",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "You are so inexperienced pilot. Not going to pay for this.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "FAST SHIP: Passage on a fast ship to {system} system.",
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  whysomuch = "I'm a factory inspector doing my rounds.",
  howmany = "Just one.",
  danger = "Sometimes people don't want to be inspected.",
  successmsg = "Thank you for the fast ride. You have been paid in full.",
  failuremsg = "I'm going to lost my job because of your incompetence. So I need that money more than you.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('English','Taxi', {
  adtext = "SHIP REQUIRED: Passage to {system} system.",
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. Will pay {cash}.",
  whysomuch = "I owe someone some money, and they're after me.",
  howmany = "Just one.",
  danger = "Someone is chasing me.",
  successmsg = "Thank you for the ride. You have been paid in full.",
  failuremsg = "I do not have enough money. Sorry.",
  wherearewe = "Where are we? I've waited enough - take me to the nearest station NOW!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ English = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Why so much money?",
  ["How many of you are there?"] = "How many of you are there?",
  ["How soon you must be there?"] = "How soon you must be there?",
  ["Will I be in any danger?"] = "Will I be in any danger?",
  ["I must be there before "] = "I must be there before ",
  ["We want to be there before "] = "We want to be there before ",
  ["You do not have enough cabin space on your ship."] = "You do not have enough cabin space on your ship.",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["Ok, agreed."] = "Ok, agreed.",
  ["Hey!?! You are going to pay for this!!!"] = "Hey!?! You are going to pay for this!!!",
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["From:"] = "From:",
  ["To:"] = "To:",
  ["Group details:"] = "Group details:",
  ["Deadline:"] = "Deadline:",
  ["Danger:"] = "Danger:",
  ["Distance:"] = "Distance:",

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

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','Taxi', {
  adtext = "POTRZEBNY TRANSPORT: Przelot małej grupy do systemu {system} za {cash}.",
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacimy {cash}.",
  whysomuch = "Odwiedzamy przyjaciela.",
  howmany = "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziękuję za przyjemną podróż. Płacimy całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacimy.",
  wherearewe = "Gdzie my jesteśmy? Nasza cierpliwość się wyczerpała - natychmiast zabierz nas do najbliższej stacji!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "POTRZEBNY TRANSPORT: Przelot małej grupy do systemu {system} za {cash}.",
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacimy {cash}.",
  whysomuch = "Pracujemy dla korporacji {corp} i to oni pokrywają koszty.",
  howmany = "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziękuję za przyjemną podróż. Płacimy całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacimy.",
  wherearewe = "Gdzie my jesteśmy? Nasza cierpliwość się wyczerpała - natychmiast zabierz nas do najbliższej stacji!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "POTRZEBNY TRANSPORT: Przelot małej grupy do systemu {system}. Płacimy {cash}.",
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacimy {cash}.",
  whysomuch = "To zwykła podróż służbowa.",
  howmany = "Jest nas {group}.",
  danger = "Nie.",
  successmsg = "Dziękuję za przyjemną podróż. Płacimy całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacimy.",
  wherearewe = "Gdzie my jesteśmy? Nasza cierpliwość się wyczerpała - natychmiast zabierz nas do najbliższej stacji!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Dawny rywal stara się mnie zabić.",
  howmany = "Tylko ja.",
  danger = "Myślę że płatny morderca jest na moim tropie, możliwe że go spotkamy.",
  successmsg = "Dziękuję za przyjemną podróż. Płacę całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Jestem komiwojażerem",
  howmany = "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dzięki za podwiezienie. Płacę całość umówionej sumy. Powodzenia!",
  failuremsg = "Nawet nie pytaj o zapłatę! Zgłoszę to w odpowiednim urzędzie.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Nie wiesz? Jestem znaną gwiazdą!",
  howmany = "Tylko ja.",
  danger = "Możesz spotkać się z zainteresowaniem ze strony prasy. Po prostu ich ignoruj.",
  successmsg = "Dziękuję za przyjemną podróż. Płacę całość umówionej sumy.",
  failuremsg = "Coś ty narobił! Zniszczyłeś moje tournée, straciłem połowę fanów.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Jestem niezależnym dziennikarzem.",
  howmany = "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dziękuję za przyjemną podróż. Płacę całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Mafia chce mojej śmierci.",
  howmany = "Ja i nikt więcej.",
  danger = "Mafia niezbyt lubi ludzi którzy pomagają jej wrogom.",
  successmsg = "Dziękuję za bezpieczną podróż. Płacę całość umówionej sumy. Powodzenia!",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Lecę do chorego krewnego.",
  howmany = "Tylko ja.",
  danger = "Nie.",
  successmsg = "Dziękuję za szybki transport. Płacę całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Podróż trwała wieczność. Nic ci nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Policji zależy na przesłuchaniu mnie.",
  howmany = "Tylko ja.",
  danger = "Policja może próbować cię powstrzymać.",
  successmsg = "Dziękuję za szybki transport. Płacę całość umówionej sumy.",
  failuremsg = "Bezużyteczny! Podróż trwała wieczność. Nic ci nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Muszę się ukryć.",
  howmany = "Jedna.",
  danger = "Myślę że ktoś mnie śledzi.",
  successmsg = "Dziękuję za szybki transport. Płacę całość umówionej sumy.",
  failuremsg = "Widać że jesteś jeszcze niedoświadczonym pilotem. Nic nie zapłacę.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot szybkim statkiem do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Jestem inspektorem pracy w delegacji.",
  howmany = "Tylko jedna.",
  danger = "Czasami ludzie nie lubią być sprawdzani.",
  successmsg = "Dziękuję za szybki transport. Płacę całość umówionej sumy.",
  failuremsg = "Przez twoją niekompetencje straciłem prace. Więc potrzebuję tych pieniędzy bardziej niż ty.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}), {dist} lś. Płacę {cash}.",
  whysomuch = "Szuka mnie ktoś od kogo pożyczyłem trochę pieniędzy.",
  howmany = "Tylko jedna.",
  danger = "Ktoś próbuje mnie znaleźć.",
  successmsg = "Dzięki za podwiezienie. Płacę całość umówionej sumy.",
  failuremsg = "Przepraszam ale nie mam tyle pieniędzy.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Polski = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Dlaczego tyle pieniędzy?",
  ["How many of you are there?"] = "Ile osób jest w grupie?",
  ["How soon you must be there?"] = "Kiedy musisz tam dotrzeć?",
  ["Will I be in any danger?"] = "Będą jakieś problemy?",
  ["I must be there before "] = "muszę tam być przed ",
  ["We want to be there before "] = "chcemy tam dotrzeć przed ",
  ["You do not have enough cabin space on your ship."] = "Na twoim statku brakuje kabin pasażerskich.",
  ["Could you repeat the original request?"] = "Możesz powtórzyć swoją ofertę?",
  ["Ok, agreed."] = "Zgoda.",
  ["Hey!?! You are going to pay for this!!!"] = "Hej!?! Zapłacisz za to!!!",
  ["ly"] = "lś",

  -- Texts for the missions screen
  ["From:"] = "Z:",
  ["To:"] = "Do:",
  ["Group details:"] = "Osób w grupie:",
  ["Deadline:"] = "Termin:",
  ["Danger:"] = "Zagrożenie:",
  ["Distance:"] = "Dystans:",

 PIRATE_TAUNTS = {
	"Pożałujesz kontaktów z {client}",
	"Masz na pokładzie {client}? To był zły pomysł.",
	"To nie jest twój szczęśliwy dzień! Przygotuj się na śmierć.",
	"Tym razem nie dotrzesz do doku!",
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

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA: Pasaje para un pequeño grupo al Sistema {system}. Se pagará {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje para un grupo pequeño al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "Vamos a visitar a un amigo.",
  howmany = "Seremos {group}.",
  danger = "No.",
  successmsg = "Gracias por el agradable viaje. Se le ha pagado al completo.",
  failuremsg = "Inaceptable! ha tardado una eternidad. No tenemos intención de pagarle.",
  wherearewe = "Dónde estamos? Ya hemos esperado demasiado - llévenos a la estación mas cercana AHORA!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA: Pasaje para un grupo pequeño al Sistema {system}. Se abonará {cash}.",
  introtext = "Saludos, Soy {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Abonaré {cash}.",
  whysomuch = "Trabajamos para la corporación {corp} y ellos pagan.",
  howmany = "Somos {group}.",
  danger = "No.",
  successmsg = "Gracias por el agradable trayecto. Se le ha pagado al completo.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tenemos intención de pagarle.",
  wherearewe = "Dónde nos encontramos? Ya hemos esperado suficiente - llévenos a la estación mas cercana DE INMEDIATO!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA: Pasaje para un grupo pequeño al Sistema {system}. Se pagarán {cash}.",
  introtext = "Hola, Mi nombre es {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "Un viaje de negocios rutinario.",
  howmany = "Somos {group}.",
  danger = "No.",
  successmsg = "Gracias por traernos. Le hemos pagado al completo. Buena suerte!",
  failuremsg = "Inaceptable! Le ha llevado una eternidad. No vamos a pagarle.",
  wherearewe = "Dónde nos encontramos? Ya hemos esperado bastante - déjenos en la estación mas cercana YA!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE REQUIERE NAVE: Pasaje al Sistema {system}. Se pagará {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje al sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Le pagaré {cash}.",
  whysomuch = "Un antiguo enemigo trata de liquidarme.",
  howmany = "Soy solo yo.",
  danger = "Creo que hay un asesino tras de mi, y posiblemente podría ir tras usted.",
  successmsg = "Gracias por el agradable viaje. Se le pagado al completo.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tengo intención alguna de pagarle.",
  wherearewe = "Dónde estamos? Ya he esperado suficiente - lléveme a la estación mas cercana AHORA!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA NAVE: Pasaje al Sistema {system}. Se abonarán {cash}.",
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "Soy viajante.",
  howmany = "Yo solo.",
  danger = "No.",
  successmsg = "Gracias por traerme. Le he pagado lo acordado. Buena suerte!",
  failuremsg = "Ni pregunte por el pago! Voy a dar parte de usted a las autoridades!.",
  wherearewe = "Dónde nos encontramos? Ya he esperado suficiente - déjeme en la estación mas cercana YA!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA: Pasaje al Sistema {system}. Se paga {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "Ah, no lo sabías - Soy una conocida Estrella.",
  howmany = "Soy solo yo.",
  danger = "Podrías despertar cierto interés de la prensa. Símplemente ignórales.",
  successmsg = "Gracias por el agradable trayecto. Se le ha pagado lo acordado.",
  failuremsg = "Pero qué has hecho! Mi Tour se ha arruinado y he perdido a la mitad de mis fans!.",
  wherearewe = "Dónde estamos? He esperado ya demasiado - llévame a la estación mas cercana AHORA!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA NAVE: Pasaje al Sistema {system}. Se abonará {cash}.",
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Abonaré {cash}.",
  whysomuch = "Soy periodista autónomo.",
  howmany = "Soy solo yo.",
  danger = "No.",
  successmsg = "Gracias por el agradable crucero. Se le ha pagado el total.",
  failuremsg = "Inaceptable! Le ha llevado una eternidad!. No tengo intención alguna de pagarle!.",
  wherearewe = "Dónde os encontramos? Ya he esperado bastante - lléveme a la estación mas cercana AHORA!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE REQUIERE NAVE: Pasaje seguro al Sistema {system}. Pagaré {cash}.",
  introtext = "Hola, mi nombre es {name} y necesito pasaje seguro al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Le pagaré {cash}.",
  whysomuch = "La Mafia me quiere fiambre.",
  howmany = "Yo y nadie mas.",
  danger = "A la Mafia no le sienta bien que alguien ayude a sus enemigos.",
  successmsg = "Gracias por traerme con seguridad. Le he pagado el total. Buena suerte!",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No pienso pagarle!.",
  wherearewe = "Pero dónde estamos? Ya he aguardado bastante - lléveme a la estación mas cercana YA!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA NAVE: Pasaje en una nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "Voy a visitar a un familiar enfermo.",
  howmany = "Solo yo.",
  danger = "No.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total acordado.",
  failuremsg = "Inaceptable! Ha tardado una eternidad. No tengo intención de pagar.",
  wherearewe = "Pero dónde nos encontramos? Ya he esperado bastante - déjeme en la estación mas cercana DE INMEDIATO!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA NAVE: Pasaje en nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pagaré {cash}.",
  whysomuch = "La Policiá quiere que colabore en una investigación.",
  howmany = "Yo solo.",
  danger = "Puede que la Policiía trate de pararle.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total.",
  failuremsg = "Inútil! Has tardado una eternidad. No tengo intención de pagar.",
  wherearewe = "Dónde estamos? Ya he esperado bastante - llévame a la estación mas cercana AHORA!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA NAVE: Pasaje en una nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Quiero pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Pago {cash}.",
  whysomuch = "Me gustaría que nadie me encontrara.",
  howmany = "Solo uno.",
  danger = "Creo que alguien me persigue.",
  successmsg = "Gracias por el rápido trayecto. Se le ha pagado el total.",
  failuremsg = "Eres un piloto novato. No voy a pagar por esto.",
  wherearewe = "Dónde estamos? Ya he esperado bastante - lléveme a la estación mas cercana AHORA!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "NAVE RÁPIDA: Pasaje en nave rápida al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Se pagarán {cash}.",
  whysomuch = "Soy Inspector de Fábricas haciendo mi ronda.",
  howmany = "Solo uno.",
  danger = "A veces la gente no gusta de inspecciones oficiales.",
  successmsg = "Gracias por el rápido viaje. Se le ha pagado el total.",
  failuremsg = "Voy a perder el trabajo por su incompetencia!. Ahora necesito el dinero más que usted!.",
  wherearewe = "Dónde estamos? Ya he esperado demasiado - déjeme en la estación mas cercana YA!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE REQUIERE NAVE: Pasaje al Sistema {system}.",
  introtext = "Mi nombre es {name}. Necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly. Se le pagará {cash}.",
  whysomuch = "Le debo pasta a alguien, y va tras de mi.",
  howmany = "Solo uno.",
  danger = "Alguien anda tras de mi.",
  successmsg = "Gracias por el viaje. Se le ha pagado el total.",
  failuremsg = "No tengo suficiente dinero. Lo siento.",
  wherearewe = "Dónde estamos? Ya he esperado demasiado - lléveme a laestación mas cercana AHORA!",
  single = 1,
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
	 "ACME",
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

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "KERESÜNK: Egy kisebb csoportot {system} rendszerbe elszállító hajót. A fizetség {cash} kredit.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. A megbízásod fizetsége {cash} kredit.",

  whysomuch = "Egy barátunkat látogatjuk meg.",
  howmany = "{group} személyről van szó.",
  danger = "Nem.",



  successmsg = "Köszönjük az utat. Már ki is fizettünk.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Örökké! Nem fogunk még fizetni is érte!",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötőbe!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "KERESÜNK: Utazási lehetőséget egy kisebb csoport számára a(z) {system} rendszerbe. Fizetünk érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

  whysomuch = "Mi a(z) {corp} vállalatnak dolgozunk, és ők fizetik az utat.",
  howmany = "{group} személyről van szó.",
  danger = "Nem.",



  successmsg = "Köszönjük az utat. Már ki is fizettünk.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Örökké! Nem fogunk még fizetni is érte!",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötőbe!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "KERESÜNK: Utazási lehetőséget egy kisebb csoport számára a(z) {system} rendszerbe. Fizetünk érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

  whysomuch = "Csak egy egyszerű üzleti út.",
  howmany = "{group} személyről van szó.",
  danger = "Nem.",


  successmsg = "Köszönjük, hogy elhoztál. Már ki is fizettünk. Sok szerencsét a továbbiakban.",
  failuremsg = "Elfogadhatatlan! Meddig tartott ez az út!? Nem fogunk még fizetni is érte.",
  wherearewe = "Hol vagyunk? Eleget vártunk - AZONNAL vigyél minket a legközelebbi kikötőbe!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓ KERESTETIK: Hajót keresek utazáshoz a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. El szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  whysomuch = "Egy régi riválisom meg akar ölni.",
  howmany = "Csak én vagyok.",


  danger = "Úgy gondolom, hogy egy orgyilkos van a nyomomban, szóval így téged is megtámadna, amíg engem szállítasz.",
  successmsg = "Köszönöm az utat. Már ki is fizettem a járandóságod.",
  failuremsg = "Ez elfogadhatatlan! Már mióta úton vagyunk. Nem fogok neked fizetni.",
  wherearewe = "Hol a francban vagyunk? Már eleget vártam - azonnal vigyél a legközelebbi kikötőbe!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓ KERESTETIK: Hajót keresek utazáshoz a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}, szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",
  whysomuch = "Utazó ügynök vagyok.",
  howmany = "Csak én.",
  danger = "Nem.",


  successmsg = "Köszönöm a szállítást. Ki is fizettelek. Sok szerencsét a továbbiakban!",
  failuremsg = "Ne is kérd a fizetséged! Jelentelek a hatóságoknak.",
  wherearewe = "Hol vagyunk? Már eleget várakoztam - AZONNAL vigyél a legközelebbi kikötődokkba!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "KERESEK: Utazási lehetőséget a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}. El akarok jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  whysomuch = "Nem is tudod - én egy ismert álomsztár vagyok.",
  howmany = "Csak én.",




  danger = "Talán a sajtó zaklatni fog miattam. Csak hagyd őket figyelmen kívül.",
  successmsg = "Kösz az utat. Már ki is vagy fizetve.",
  failuremsg = "Mit csináltál! A turnémnak annyi, és elveszítettem a rajongóim felét.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötőbe AZONNAL!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "KERESEK: Hajót, ami elszállít a(z) {system} rendszerbe. Fizetek érte {cash} kreditet.",
  introtext = "Helló, a nevem {name}, és szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek az útért {cash} kreditet.",
  whysomuch = "Szabadúszó újságíró vagyok.",
  howmany = "Csak én vagyok.",
  danger = "Nem.",



  successmsg = "Köszönöm az utat. Már ki is fizettelek érte.",
  failuremsg = "Elfogadhatatlan, meddig tartott! Örökké. Ezért én nem fogok neked még fizetni is.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi űrkikötőbe!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓT KERESEK: Hajót, amely biztonságban elvisz a(z) {system} rendszerbe {cash} kreditért cserébe.",
  introtext = "Helló, a nevem {name}. Szeretnék biztonságban eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  whysomuch = "A maffia meg akar ölni.",
  howmany = "Én és senki más.",




  danger = "A maffia nem veszi jó néven, ha segíted az ellenségeit..",
  successmsg = "Köszönöm, hogy biztonságban elhoztál ide. Kifizettelek, további sok sikert!",
  failuremsg = "Ez elfogadhatatlan, hogy mennyi időt elvesztegettél. Nem fogok még fizetni is ezért.",
  wherearewe = "Hol a pokolban vagyunk? Már eleget várakoztam - vigyél AZONNAL a legközelebbi űrkikötőbe!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓT KERESEK: Olyat, amely gyorsan elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek az útért {cash} kreditet.",

  whysomuch = "Egy beteg rokonomat látogatom meg.",
  howmany = "Csak én.",
  danger = "Nem.",



  successmsg = "Köszönöm a gyors szállítást. Ki is fizettelek érte.",
  failuremsg = "Elfogadhatatlan! Már egy örökkévalóság eltelt az indulás óta. Nem fogok még fizetni is érte.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi űrkikötőbe!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓT KERESEK: Olyat, amely gyorsan elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Szeretnék gyorsan eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  whysomuch = "A rendőrség megkért, hogy segítsek a nyomozásukban.",
  howmany = "Csak én.",




  danger = "A rendőrség lehet hogy meg akar állítani.",
  successmsg = "Köszönöm a gyors utat. Kifizettelek érte.",
  failuremsg = "Használhatatlan! Már mióta jövünk. Nem fogok ezért még fizetni is.",
  wherearewe = "Hol a pokolban vagyunk? Már eleget várakoztam - vigyél AZONNAL a legközelebbi űrkikötőbe!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓT KERESEK: Egy gyors hajót, amely elvinne a(z) {system} rendszerbe.",
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek {cash} kreditet.",

  whysomuch = "Szereném, ha egy bizonyos valaki nem találna rám.",
  howmany = "Csak egy.",




  danger = "Szerintem valaki követ engem.",
  successmsg = "Köszönöm a gyors utat. Azonnal ki is fizetlek érte.",
  failuremsg = "Annyira egy kezdő pilóta vagy, hogy nem várhatod el a fizetséget se.",
  wherearewe = "Hol vagyunk egyáltalán? Már eleget vártam - vigyél AZONNAL a legközelebbi űrkikötőbe!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "GYORS HAJÓT KERESEK: Egy olyat, amely elvinne a(z) {system} rendszerbe.",
  introtext = "Az én nevem {name}. Egy gyors utazást szeretnék a(z) {system} ({sectorx}, {sectory}, {sectorz}), {dist} ly rendszerbe. Fizetek érte {cash} kreditet.",

  whysomuch = "Csak a körutamat járom gyárvizsgálóként.",
  howmany = "Csak egy.",




  danger = "Néhányan nem akarják, hogy vizsgálódjak.",
  successmsg = "Köszönöm a gyors utat, máris kifizetlek érte.",
  failuremsg = "El fogom veszteni a munkámat a képzetlenséged miatt. Így pénzt sem várhatsz el, mert nekem nagyobb szükségem van rá.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötőbe AZONNAL!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Magyar','Taxi', {

  adtext = "HAJÓT KERESEK: Amely elvinne a(z) {system} rendszerbe.",
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",

  whysomuch = "Tartozom valakinek pénzzel, és ezért üldöz engem.",
  howmany = "Csak egy.",




  danger = "Valaki üldöz engem.",
  successmsg = "Köszönöm a fuvart, máris kifizetlek.",
  failuremsg = "Nincs elég pénzem. Bocs.",
  wherearewe = "Hol vagyunk? Már eleget vártam - vigyél a legközelebbi kikötőbe AZONNAL!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Magyar = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Miért ennyi a díjazás?",
  ["How many of you are there?"] = "Hány személyről van szó?",
  ["How soon you must be there?"] = "Milyen hamar kell odaérni?",
  ["Will I be in any danger?"] = "Lesz valamilyen veszély útközben?",
  ["I must be there before "] = "Oda kell érnem még előtte:",
  ["We want to be there before "] = "Oda kell érnünk ezen időpontig:",
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

  ---- RUSSIAN / РУССКИЙ ----

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩЕМ ТРАНСПОРТ: для перелета небольшой группы в систему {system} за {cash}.",
  introtext = "Привет! Меня зовут {name}.\n Я ищу транспорт, чтобы доставить небольшую группу людей в {system} ({sectorx}, {sectory}, {sectorz}).\n Расстояние {dist} св.л., оплата {cash}.",
  whysomuch = "Решил навестить друга.",
  howmany = "Нас в группе всего {group}.",
  danger = "Нет.",
  successmsg = "Спасибо за приятное путешествие! Мы платим всю согласованную сумму.",
  failuremsg = "Это неприемлемо! Полет длился вечность! Мы отказываемся платить.",
  wherearewe = "Где мы находимся? Наше терпение истекло - высадите нас на ближайшей станции!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩЕМ ТРАНСПОРТ: перелет небольшой группы в систему {system} за {cash}.",
  introtext = "Добрый день! Меня зовут {name}.\n Наша группа ищет перевозчика, чтобы попасть в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., заплатим {cash}.",
  whysomuch = "Мы работаем по заданию корпорации {corp}. Эта организация покрывает все расходы.",
  howmany = "Нас в группе всего {group}.",
  danger = "Нет.",
  successmsg = "Спасибо за приятное путешествие! Платим как договаривались.",
  failuremsg = "Это недопустимо! Вы не уложились в срок - нам не за что платить вам.",
  wherearewe = "Где мы?! Не тратьте наше время! - высадите нас где-нибудь!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩЕМ ТРАНСПОРТ: перелет маленькой группы в систему {system}. Заплатим {cash}.",
  introtext = "Приветствую! Моё имя {name}.\n Наша группа ищет перевозчика, чтобы попасть в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., заплатим {cash}.",
  whysomuch = "Обычная деловая поездка.",
  howmany = "Нас в группе всего {group}.",
  danger = "Нет.",
  successmsg = "Было приятно лететь с вами! Деньги уже перечислены.",
  failuremsg = "Мы опоздали! Вы не заработали свои деньги.",
  wherearewe = "Где мы находимся? Нам надоело это затянувшееся путешествие - высадите нас на любой станции!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для перелета в систему {system}. Плачу {cash}.",
  introtext = "Разрешите представиться - {name}.\n Ищу транспорт в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., предлагаю {cash}.",
  whysomuch = "Старый враг пытается достать меня - я чувствую, что он уже близко.",
  howmany = "Только я.",
  danger = "Я думаю, что у меня на хвосте наёмный убийца - возможно мы встретимся с ним.",
  successmsg = "Спасибо! Мы достигли цели. Плачу как договаривались.",
  failuremsg = "Это было неприятное и долгое путешествие! Я останусь при своих деньгах.",
  wherearewe = "Куда мы залетели? Немедленно доставьте меня в любой порт!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для перелета до системы {system}. Оплата {cash}.",
  introtext = "Доброго времени суток! Я {name}.\n Хочу отправиться в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., с меня {cash}.",
  whysomuch = "Я коммивояжер.",
  howmany = "Только я.",
  danger = "Нет.",
  successmsg = "Спасибо, что подвезли! Плачу как договаривались. Удачи!",
  failuremsg = "Даже не спрашивайте о деньгах! Я сообщу о вас в соответствующее ведомство!",
  wherearewe = "Какого чёрта мы тут делаем? Мне всё это надоело -  немедленно доставьте меня на какую-нибудь станцию!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для тура в систему {system}. Цена контракта {cash}.",
  introtext = "Привет-привет! Я {name}.\n Доставьте меня в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., заработаете {cash}.",
  whysomuch = "Не видишь, что ли? Я известная звезда! У меня тур по системам!",
  howmany = "Только я.",
  danger = "Пресса будет охотиться за мной. Просто игнорируйте её.",
  successmsg = "Спасибо, всё отлично! Вот ваши деньги.",
  failuremsg = "Что ты наделал! Испортил моё турне - половина фанатов отвернулась от меня!!",
  wherearewe = "Что это за место?! Это какой-то кошмар! - доставьте меня на ближайшую станцию!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для перелета в систему {system}. Готов заплатить {cash}.",
  introtext = "Приветствую! Моё имя {name}.\n Следую в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., оплата {cash}.",
  whysomuch = "Я независимый журналист, занимаюсь сбором информации для репортажей.",
  howmany = "Только я.",
  danger = "Нет.",
  successmsg = "Спасибо за полёт! Я перевёл плату на ваш счёт.",
  failuremsg = "Нонсенс! Контракт просрочен. Ничего не заплачу.",
  wherearewe = "Где мы? Моё терпение лопнуло - высадите меня на ближайшей цивилизованной планете!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: чтобы отправиться в систему {system}. Оплата {cash}.",
  introtext = "Здравствуйте, я {name}.\n Готовы доставить меня в систему {system} ({sectorx}, {sectory}, {sectorz})?\n Расстояние {dist} св.л., цена контракта {cash}.",
  whysomuch = "Мафия хочет моей смерти, мне необходимо покинуть эту систему.",
  howmany = "Я и никого больше.",
  danger = "Мафия не любит людей, которые помогают её врагам.",
  successmsg = "Спасибо за безопасное путешествие. Вот обещанная оплата. Удачи вам!",
  failuremsg = "Сожалею, но мы слишком долго летали. Я не готов заплатить вам. Прощайте.",
  wherearewe = "Где мы? Я не могу больше ждать, я должен сойти на ближайшей станции.",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для срочного полёта в систему {system}. С меня {cash}.",
  introtext = "Здравствуйте, моё имя {name}.\n Мне надо быстро оказаться в системе {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., выложу {cash}.",
  whysomuch = "Я лечу к больному родственнику",
  howmany = "Никого кроме меня.",
  danger = "Нет.",
  successmsg = "Спасибо за быструю доставку! Вот вся сумма.",
  failuremsg = "Это какой-то кошмар... Мы слишком долго летели - я не буду вам платить.",
  wherearewe = "Где мы, чёрт возьми? Мне некогда ждать - я сойду на ближайшей станции!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для быстрого перелета в систему {system}. Заплачу {cash}.",
  introtext = "Привет! Зови меня {name}.\n Мне надо быстро перебраться в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., получишь {cash}.",
  whysomuch = "Полиция хочет взять у меня интервью.",
  howmany = "Только я.",
  danger = "Полиция может проявить интерес к нам.",
  successmsg = "Спасибо, друг. Вот, с меня бабки.",
  failuremsg = "Чёртово корыто! Мы опоздали. Шиш тебе с маслом, а не бабки!",
  wherearewe = "Какого чёрта мы тут делаем? Меня достало летать с тобой - сваливаю на ближайшей станции.",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для скоростного перелёта в систему {system}! Заплачу {cash}.",
  introtext = "Здрасьте, я {name}.\n Можете быстрее доставить меня в систему {system} ({sectorx}, {sectory}, {sectorz})?\n Расстояние {dist} св.л., даю {cash}.",
  whysomuch = "Мне нужно скрыться!",
  howmany = "Никого кроме меня не будет.",
  danger = "Кто-то следит за мной!",
  successmsg = "Спасибо за быстрый перелёт! Вот, деньги ваши.",
  failuremsg = "По-моему вы просто неудачник! Я не собираюсь платить вам!",
  wherearewe = "Где? Где мы? Мне некогда играть в игры - немедленно ссадите меня на какой-нибудь станции!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: для срочного перелёта в систему {system}. Моё предложение - {cash}.",
  introtext = "Добрый день. Я {name}.\n Необходим скоростной корабль для полёта в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., за работу предлагаю {cash}.",
  whysomuch = "Я промышленный инспектор - совершаю плановый облёт.",
  howmany = "Я лечу один.",
  danger = "Не все люди любят проверки.",
  successmsg = "Благодарю за скорость. Оплата переведена на ваш счёт.",
  failuremsg = "Из-за вашей некомпетентности я потерял работу. Таким образом, в деньгах я нуждаюсь больше вас.",
  wherearewe = "Где мы находимся? Вы разочаровали меня - доставьте меня в ближайший космопорт.",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Russian','Taxi', {
  adtext = "ИЩУ КОРАБЛЬ: цель - перелет в систему {system}. Цена сделки {cash}.",
  introtext = "Зовите меня {name}.\n Я хочу попасть в систему {system} ({sectorx}, {sectory}, {sectorz})\n Расстояние {dist} св.л., заплачу {cash}.",
  whysomuch = "Скрываюсь от кредитора, он меня ищет.",
  howmany = "Только я.",
  danger = "Похоже меня ищут.",
  successmsg = "Спасибо, что подбросили. Вот оплата.",
  failuremsg = "Мне жаль, но у меня нет столько денег.",
  wherearewe = "Где мы находимся? Моё терпение иссякло - немедленно отвезите меня на ближайшую станцию!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Russian = {
  ["Taxi"] = "Пассажиры",
  ["Why so much money?"] = "Цель заключения контракта?",
  ["How many of you are there?"] = "Сколько человек в вашей группе?",
  ["How soon you must be there?"] = "Когда вы должны быть на месте?",
  ["Will I be in any danger?"] = "Предвидятся какие-либо проблемы?",
  ["I must be there before "] = "Я хочу быть там не позднее ",
  ["We want to be there before "] = "Мы хотим быть там не позднее ",
  ["You do not have enough cabin space on your ship."] = "На вашем корабле недостаточно пассажирских кают.",
  ["Could you repeat the original request?"] = "Не могли бы вы повторить ваше предложение?",
  ["Ok, agreed."] = "Хорошо, договорились.",
  ["Hey!?! You are going to pay for this!!!"] = "Эй!! Вы собираетесь платить за это?!",
  ["ly"] = "св.лет",

  -- Texts for the missions screen
  ["From:"] = "Начало маршрута:",
  ["To:"] = "Конец маршрута:",
  ["Group details:"] = "О группе:",
  ["Deadline:"] = "Крайний срок:",
  ["Danger:"] = "Опасность:",
  ["Distance:"] = "Расстояние:",

 PIRATE_TAUNTS = {
	"Ты ответишь за контракт с {client}!!",
	"У вас на борту {client}? Это была плохая идея!",
	"Сегодня не твой день! Готовься к смерти.",
	"В этот раз ты не долетишь до станции!",
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

  ---- GERMAN / DEUTSCH ----

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "GESUCHT: Passage für eine kleine Gruppe zum {system} System. Wir werden {cash} zahlen.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage für eine kleine Gruppe zum {system} ({sectorx}, {sectory}, {sectorz}) System, eine Strecke von {dist} ly. Ich werde {cash} zahlen.",
  whysomuch = "Wir besuchen einen Freund.",
  howmany = "Wir sind {group}.",
  danger = "Nein.",
  successmsg = "Danke für die nette Reise. Du wurdest voll bezahlt.",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Wir haben lange genug gewartet - Bringe uns zur nächsten Station, SOFORT!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "GESUCHT: Passage für eine kleine Gruppe zum {system} System. Wir werden {cash} zahlen.",
  introtext = "Hi, ich bin {name} und brauche eine Passage für eine kleine Gruppe zum {system} ({sectorx}, {sectory}, {sectorz}) System,  {dist} ly entfernt von hier. Ich werde {cash} zahlen.",
  whysomuch = "Wir arbeiten für {corp} Unternehmen und sie zahlen.",
  howmany = "Wir sind {group} Personen.",
  danger = "Nein.",
  successmsg = "Danke für die nette Reise. Du wurdest voll bezahlt.",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Wir haben lange genug gewartet - Bringe uns zur nächsten Station, SOFORT!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "GESUCHT: Passage für eine kleine Gruppe zum {system} System. Wir werden {cash} zahlen.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage für eine kleine Gruppe zum {system} ({sectorx}, {sectory}, {sectorz}) System, das ist eine Entfernung von {dist} ly. Ich werde will {cash} zahlen.",
  whysomuch = "Es ist eine normale Geschäftsreise.",
  howmany = "Wir sind {group} Personen.",
  danger = "No.",
  successmsg = "Danke, dass du uns hier her gebracht hast. Wir haben dich in voller Höhe bezahlt. Viel Glück!",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Wir haben lange genug gewartet - Bringe uns zur nächsten Station, SOFORT!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "BRAUCHE SCHIFF: Passage zum {system} System. Werde {cash} zahlen.",
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly. I will pay {cash}.",
  whysomuch = "Ein alter Rivale versucht mich zu ermorden.",
  howmany = "Ich reise alleine.",
  danger = "Ich vermute, dass mir ein Attentäter auf den Fersen ist und er könnte uns verfolgen.",
  successmsg = "Danke, du hast mir das Leben gerettet. Du wurdest voll bezahlt.",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SUCHE SCHIFF: Passage zum {system} System. Ich zahle {cash}.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly Entfernung. Ich zahle {cash}.",
  whysomuch = "Ich bin ein Handlungsreisender.", --Traveling salesman problem? ;)
  howmany = "Nur ich allein.",
  danger = "Nein.",
  successmsg = "Danke, dass du mich hier her gebracht hast. Ich habe dir den Lohn überwiesen. Viel Glück!",
  failuremsg = "Wage es nicht nach einer Belohnung zu fragen! Ich werde dich der Polizei melden.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "GESUCHT: Passage zum {system} System. Ich zahle {cash}.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly Entfernung. Ich zahle {cash}.",
  whysomuch = "Das weißt du nicht? Ich bin ein bekannter Superstar!",
  howmany = "Ich reise alleine.",
  danger = "Du wirst vielleicht von der Presse belästigt. Ignoriere sie einfach.",
  successmsg = "Danke für die nette Reise. Du wurdest voll bezahlt.",
  failuremsg = "Was hast du getan? Meine Tour ist versaut und ich habe die Hälfte meiner Fans verloren.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "BRAUCHE SCHIFF: Passage zum {system} System. Ich zahle {cash}.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly Entfernung. Ich zahle {cash}.",
  whysomuch = "Ich bin ein selbstständiger Journalist.",
  howmany = "Nur ich alleine.",
  danger = "Nein.",
  successmsg = "Danke für die nette Reise. Du wurdest voll bezahlt.",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SCHIFF GESUCHT: Sichere Passage zum {system} System. Ich werde {cash} zahlen.",
  introtext = "Hi, Ich bin {name} und ich brauche eine Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly Entfernung. Ich zahle {cash}.",
  whysomuch = "Die Mafia will mich tot sehen.",
  howmany = "Ich und niemand anderes.",
  danger = "Die Mafia mag keine Leute, die ihren Feinden hilft.",
  successmsg = "Danke, dass du mich sicher hierher gebracht hast. Hier ist dein Lohn. Viel Glück!",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SHIP WANTED: Passage on a fast ship to {system} system.",
  introtext = "Mein Name ist {name}. Ich brauche eine schnelle Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, etwa {dist} ly entfernt von hier. Ich werde dich mit {cash} entlohnen.",
  whysomuch = "Ich besuche einen kranken Verwandten.",
  howmany = "Nur ich.",
  danger = "Nein.",
  successmsg = "Danke für diesen schnellen Flud. Du wurdest dafür wie abgemacht entlohnt.",
  failuremsg = "Unakzeptabel! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SCHIFF GESUCHT: Passage auf einem schnellen schiff zum {system} System.",
  introtext = "Mein Name ist {name}. Ich brauche eine schnelle Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, etwa {dist} ly entfernt von hier. Ich zahle dir {cash}.",
  whysomuch = "Die Polizei will mich ausfragen.",
  howmany = "Nur ich alleine.",
  danger = "Die Polizei wird vielleicht versuchen, dich zu stoppen.",
  successmsg = "Danke für den schnellen Flug. Hier ist dein Lohn.",
  failuremsg = "Nutzlos! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SUCHE SCHIFF: Passage auf einem schnellen schiff zum {system} System.",
  introtext = "Mein Name ist {name}. Ich brauche eine schnelle Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, etwa {dist} ly entfernt von hier. Zahlung ist {cash}.",
  whysomuch = "Ich will nicht gefunden werden.",
  howmany = "Nur einer.",
  danger = "Ich habe die Vermutung, dass mir jemand folgt.",
  successmsg = "Danke für den schnellen Flug. Hier ist der abgemachte Lohn.",
  failuremsg = "Nutzlos! Du hast ewig gebraucht. Für diese Leistung wollen wir dich nicht zahlen.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "SCHNELLES SCHIFF: Passage auf einem schnellen Schiff zum {system} System.",
  introtext = "Mein Name ist {name}. Ich brauche eine schnelle Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, etwa {dist} ly entfernt von hier. Ich werde {cash} zahlen.",
  whysomuch = "Ich bin ein Fabrikinspektor und mache meine Runde.",
  howmany = "Eine Person.",
  danger = "Manchmal wollen Leute nicht inspiziert werden.",
  successmsg = "Danke für den schnellen Flug. Hier ist der abgemachte Lohn.",
  failuremsg = "Wegen deiner Inkompetenz werde ich meinen Job verlieren. Also werde ich dieses Geld dringender brauchen als du.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Deutsch','Taxi', {
  adtext = "BRAUCHE SCHIFF: Passage zum {system} System.",
  introtext = "Mein Name ist {name}. Ich brauche eine Passage zum {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly entfernt von hier. Ich werde {cash} zahlen.",
  whysomuch = "Ich schulde jemandem Geld und sie sind hinter mir her.",
  howmany = "Nur eine Person.",
  danger = "Jemand jagt mich.",
  successmsg = "Danke für den schnellen Flug. Hier ist der abgemachte Lohn.",
  failuremsg = "Entschuldigung, Ich habe nicht genug Geld.",
  wherearewe = "Wo sind wir? Ich habe lange genug gewartet - Bringe mich zur nächsten Station, SOFORT!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Deutsch = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Warum so viel Geld?",
  ["How many of you are there?"] = "Wie viele Personen seid ihr?",
  ["How soon you must be there?"] = "Wie bald musst du da sein?",
  ["Will I be in any danger?"] = "Gibt es Gefahren?",
  ["I must be there before "] = "Ich mus dort sein vor ",
  ["We want to be there before "] = "Wir wollen dort sein vor ",
  ["You do not have enough cabin space on your ship."] = "Du hast nicht genug Raum in deinem Schiff.",
  ["Could you repeat the original request?"] = "Was war nochmal die Frage?",
  ["Ok, agreed."] = "Ok, ich werde den Flug übernehmen.",
  ["Hey!?! You are going to pay for this!!!"] = "Hey!?! Du wirst dafür bezahlen!!!",
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["From:"] = "Von:",
  ["To:"] = "Nach:",
  ["Group details:"] = "Gruppendetails:",
  ["Deadline:"] = "Deadline:",
  ["Danger:"] = "Gefahr:",
  ["Distance:"] = "Distanz:",

 PIRATE_TAUNTS = {
	"Du wirst deine Kooperation mit {client} bedauern.",
	"Du hast {client} an Bord? Das war keine gute Idee.",
	"Heute ist nicht dein Glückstag. Bereite dich auf deinen Tod vor.",
	"Du wirst heute nicht mehr Docken!",
  },
 CORPORATIONS = {
	 "Sirius",
	 "ACME",
	 "Cool Cola",
	 "Taranis",
	 "Aquarian Schiffbau",
	 "Rockforth",
	 "Amaliel",
	 "Marett Raum",
	 "Vega Linie",
	 "Digital",
	 "Bulk Schiffe",
	 "Arment Aerodynamik"
  },
}, })

  ---- CZECH / ČESKY ----

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM: Místo na lodi pro malou skupinu do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju místo na lodi pro malou skupinu do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Letíme navštívit přítele.",
  howmany = "Počet pasažérů je: {group}",
  danger = "Ne.",
  successmsg = "Děkujeme za hezký výlet. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsme se načekali dost - Vem nás do nejbližší stanice OKAMŽITĚ!",
  single = 0,
  urgency = 0,
  risk = 0.001,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM: Místo na lodi pro malou skupinu do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju místo na lodi pro malou skupinu do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Pracujeme pro firmu {corp}, platí to oni.",
  howmany = "Počet pasažérů je: {group}",
  danger = "Ne.",
  successmsg = "Děkujeme za hezký výlet. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsme se načekali dost - Vem nás do nejbližší stanice OKAMŽITĚ!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM: Místo na lodi pro malou skupinu do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju místo na lodi pro malou skupinu do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jde o normální služební cestu.",
  howmany = "Počet pasažérů je: {group}",
  danger = "Ne.",
  successmsg = "Díky že jsi nás sem vzal. Dostaneš zaplaceno v plné výši. Hodně štěstí!",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsme se načekali dost - Vem nás do nejbližší stanice OKAMŽITĚ!",
  single = 0,
  urgency = 0,
  risk = 0,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "SHÁNÍM LOĎ: Odvoz do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jeden starý rival se mě snaží zabít.",
  howmany = "Jenom já.",
  danger = "Myslím že je mi na stopě najatý vrah a tak může jít i po tobě.",
  successmsg = "Dík za hezký výlet. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsem se načekal dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.13,
  risk = 0.73,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "SHÁNÍM LOĎ: Odvoz do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jsem obchodní cestující.",
  howmany = "Jen já.",
  danger = "Ne.",
  successmsg = "Díky že jsi mě sem vzal. Dostaneš zaplaceno v plné výši. Hodně štěstí!",
  failuremsg = "Ani se neopovažuj požadovat platbu! Nahlásím tě úřadům.",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.3,
  risk = 0.02,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM: Odvoz do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Ty nevíš - Jsem velmi známá celebrita.",
  howmany = "Jenom já.",
  danger = "Možná tě budou otravovat novináři a média. Prostě je ignoruj.",
  successmsg = "Děkuji za hezký výlet. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Co jsi to udělal(a)?!? Moje turné je v čudu i s polovinou fanoušků!",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vem mě do nejbližší stanice OKAMŽITĚ!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Odvoz do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jsem {name} a potřebuju odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jsem nezávislý novinář.",
  howmany = "It's only me.",
  howmany = "Jen já.",
  danger = "Ne.",
  successmsg = "Díky za hezký výlet. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.02,
  risk = 0.07,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Bezpečný odvoz do systému {system}. Zaplatím {cash}.",
  introtext = "Ahoj, jmenuji se {name} a potřebuju bezpečný odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jde po mě mafie, chtějí mou smrt.",
  howmany = "Já a nikdo jiný.",
  danger = "Mafie nemá ráda lidi, kteří pomáhají jejím nepřátelům.",
  successmsg = "Děkuju že jsi mě sem bezpečně dostal. Dostaneš zaplaceno v plné výši. Hodně štěstí!",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.15,
  risk = 1,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Odvoz na rychlé lodi do systému {system}. Zaplatím {cash}.",
  introtext = "Jmenuji se {name}. Potřebuji rychlý odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jedu navštívit nemocného příbuzného.",
  howmany = "Jen já.",
  danger = "Ne.",
  successmsg = "Díky za rychlý let. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.5,
  risk = 0.001,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Odvoz na rychlé lodi do systému {system}.",
  introtext = "Jmenuji se {name}. Potřebuji rychlý odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Shání mě policie, chtějí odpovědi na pár otázek.",
  howmany = "Jen já.",
  danger = "Policie se tě může pokusit zastavit.",
  successmsg = "Díky za rychlý let. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Jsi k ničemu! Trvalo to věčnost a den. Nedostaneš zaplaceno!",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.85,
  risk = 0.20,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Odvoz na rychlé lodi do systému {system}.",
  introtext = "Jmenuji se {name}. Potřebuji rychlý odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Byl bych raděj, aby mě někdo nenašel.",
  howmany = "Jen jeden.",
  danger = "Myslím, že mě někdo sleduje.",
  successmsg = "Díky za rychlý let. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Jsi opravdu nezkušený pilot. Za tohle nezaplatím!",
  wherearewe = "Kde to jsme? Už jsem se čekal(a) dost dlouho - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0.9,
  risk = 0.40,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "RYCHLOU LOĎ: Odvoz na rychlé lodi do systému {system}.",
  introtext = "Jmenuji se {name}. Potřebuji rychlý odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím {cash}.",
  whysomuch = "Jsem podnikový auditor na inspekční cestě.",
  howmany = "Jen jeden.",
  danger = "Někdy se stává, že se lidem inspekce nelíbí.",
  successmsg = "Díky za rychlý let. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Jsi opravdu nekompetentní a já jsem teď bez práce! Takže ty peníze potřebuju víc než ty.",
  wherearewe = "Kde to jsme? Už jsem se čekal(a) dost dlouho - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 1,
  risk = 0.31,
})

Translate:AddFlavour('Czech','Taxi', {
  adtext = "HLEDÁM LOĎ: Odvoz do systému {system}.",
  introtext = "Jmenuji se {name}. Potřebuju odvoz do systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly. Zaplatím ti {cash}.",
  whysomuch = "Dlužím nějaké peníze a jdou po mě.",
  howmany = "Jen jeden.",
  danger = "Někdo mě pronásleduje.",
  successmsg = "Díky za odvoz, dostaneš zaplaceno v plné výši.",
  failuremsg = "Nemám dost pěněz. Promiň.",
  wherearewe = "Kde to jsme? Už jsem se načekal(a) dost - Vysaď mě na nejbližší stanici OKAMŽITĚ!",
  single = 1,
  urgency = 0,
  risk = 0.17,
})

Translate:Add({ Czech = {
  ["Taxi"] = "Taxi",
  ["Why so much money?"] = "Proč tolik peněz?",
  ["How many of you are there?"] = "Kolik vás je?",
  ["How soon you must be there?"] = "Dokdy tam musíte dorazit?",
  ["Will I be in any danger?"] = "Ocitnu se v nějakém nebezpečí?",
  ["I must be there before "] = "Musím tam dorazit do ",
  ["We want to be there before "] = "Musíme tam dorazit do ",
  ["You do not have enough cabin space on your ship."] = "Na palubě lodi nemáš dostatek místa pro pasažéry.",
  ["Could you repeat the original request?"] = "Můžete mi znovu zopakovat váš požadavek?",
  ["Ok, agreed."] = "OK, souhlasím.",
  ["Hey!?! You are going to pay for this!!!"] = "Hej!?! Za tohle zaplatíš!!!",
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["From:"] = "Odkud:",
  ["To:"] = "Kam:",
  ["Group details:"] = "Detail skupiny:",
  ["Deadline:"] = "Termín:",
  ["Danger:"] = "Nebezpečí:",
  ["Distance:"] = "Vzdálenost:",

 PIRATE_TAUNTS = {
	"Budeš litovat, že jsis něco začal s {client}",
	"Ty máš na palubě {client}? To nebyl dobrý nápad.",
	"Dnes není tvůj šťastný den! Připrav se na smrt.",
	"Ty už dnes do cíle nedorazíš!",
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
