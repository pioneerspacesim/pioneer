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

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','Taxi', {
  adtext = "SE BUSCA: Pasaje para un pequeño grupo al Sistema {system}. Se pagará {cash}.",
  introtext = "Hola, Soy {name} y necesito pasaje para un grupo pequeño al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Saludos, Soy {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Abonaré {cash}.",
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
  introtext = "Hola, Mi nombre es {name} y necesito pasaje para un pequeño grupo al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Hola, Soy {name} y necesito pasaje al sistema {system} ({sectorx}, {sectory}, {sectorz}). Le pagaré {cash}.",
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
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Hola, Soy {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Saludos, Mi nombre es {name} y necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Abonaré {cash}.",
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
  introtext = "Hola, mi nombre es {name} y necesito pasaje seguro al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Le pagaré {cash}.",
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
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pagaré {cash}.",
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
  introtext = "Mi nombre es {name}. Quiero pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Pago {cash}.",
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
  introtext = "Mi nombre es {name}. Necesito pasaje rápido al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Se pagarán {cash}.",
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
  introtext = "Mi nombre es {name}. Necesito pasaje al Sistema {system} ({sectorx}, {sectory}, {sectorz}). Se le pagará {cash}.",
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
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. A megbízásod fizetsége {cash} kredit.",

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
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

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
  introtext = "Helló, a nevem {name}. Szeretném, ha elvinnél egy kisebb csoportot a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetnénk a szállításért {cash} kreditet.",

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
  introtext = "Helló, a nevem {name}. El szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",

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
  introtext = "Helló, a nevem {name}, szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",
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
  introtext = "Helló, a nevem {name}. El akarok jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",

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
  introtext = "Helló, a nevem {name}, és szeretnék eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek az útért {cash} kreditet.",
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
  introtext = "Helló, a nevem {name}. Szeretnék biztonságban eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",

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
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek az útért {cash} kreditet.",

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
  introtext = "A nevem {name}. Szeretnék gyorsan eljutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",

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
  introtext = "A nevem {name}. Gyorsan el szeretnék jutni a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek {cash} kreditet.",

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
  introtext = "Az én nevem {name}. Egy gyors utazást szeretnék a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerbe. Fizetek érte {cash} kreditet.",

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
