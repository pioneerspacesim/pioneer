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
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage for a small group to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, I'm {name} and I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "Hi, my name is {name} and I need safe passage to {system} ({sectorx}, {sectory}, {sectorz}) system. I will pay {cash}.",
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
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",
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
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",
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
  introtext = "My name is {name}. I want fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Paying {cash}.",
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
  introtext = "My name is {name}. I need fast passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",
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
  introtext = "My name is {name}. I need passage to {system} ({sectorx}, {sectory}, {sectorz}) system. Will pay {cash}.",
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
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacimy {cash}.",
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
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacimy {cash}.",
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
  introtext = "Cześć, nazywam się {name} i szukam transportu dla małej grupy osób do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacimy {cash}.",
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
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
  whysomuch = "Nie wiesz? Jestem znaną gwiazdą!",
  howmany = "Tylko ja.",
  danger = "Możesz spotkać się z zainteresowaniem ze strony prasy. Po prostu ich ignoruj.",
  successmsg = "Dziękuję za przyjemną podróż. Płacę całość umówionej sumy.",
  failuremsg = "Coś ty narobił! Zniszczyłeś moje turne, straciłem połowę fanów.",
  wherearewe = "Gdzie my jesteśmy? Moja cierpliwość się wyczerpała - natychmiast zabierz mnie do najbliższej stacji!",
  single = 1,
  urgency = 0.1,
  risk = 0.05,
})

Translate:AddFlavour('Polski','Taxi', {
  adtext = "SZUKAM STATKU: Przelot do systemu {system}. Płacę {cash}.",
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Cześć, nazywam się {name}, muszę szybko dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
  introtext = "Nazywam się {name}, chcę dostać się do systemu {system} ({sectorx}, {sectory}, {sectorz}). Płacę {cash}.",
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
