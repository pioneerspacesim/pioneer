  -- adtext - text shown in the bulletin board list
	-- introtext - shown when the advert is selected (and "Could you repeat request?")
	-- whysomuchtext - response to "Why so much?"
	-- successmsg - message sent on successful delivery
	-- failuremsg - message sent on failed delivery
	-- urgency - how urgent the delivery is. 0 is surface mail. 1 is overnight
	-- risk - how risky the mission is. 0 is letters from mother. 1 is certain death
	-- local - 1 if the delivery is to the local (this) system, 0 otherwise

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "GOING TO the {system} system? Money paid for delivery of a small package.",
  introtext = "Hi, I'm {name}. I'll pay you {cash} if you will deliver a small package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
  whysomuchtext = "When a friend visited me she left behind some clothes and antique paper books. I'd like to have them returned to her.",
  successmsg = "Thank you for the delivery. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever over that delivery. I'm not willing to pay you.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "WANTED. Delivery of a package to the {system} system.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
  whysomuchtext = "It is nothing special.",
  successmsg = "The package has been received and you have been paid in full.",
  failuremsg = "I'm frustrated by the late delivery of my package, and I refuse to pay you.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "URGENT. Fast ship needed to deliver a package to the {system} system.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
  whysomuchtext = "It is a research proposal and must be delivered by the deadline or we may not get funding.",
  successmsg = "You have been paid in full for the delivery. Thank you.",
  failuremsg = "I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "DELIVERY. Documents to the {system} system. {cash} to an experienced pilot.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
  whysomuchtext = "Some extremely sensitive documents have fallen into my hands, and I have reason to believe that the leak has been traced to me.",
  successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
  failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "POSTAL SERVICE. We require a ship for the delivery run to {system} system.",
  introtext = "Greetings. This is an automated message from Bedford and {name} Courier Services. We pay {cash} for the run to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
  whysomuchtext = "We would be happy to pay you less money.",
  successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
  failuremsg = "Your ship registration has been noted, we will reject all further applications for work from you.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "MOVING HOME. Move of hardware to {starport} from storage.",
  introtext = "Nice to meet you. I am {name} and I'm willing to pay {cash} for someone with a ship to help me move my belongings to {starport}. No rush, they are just some leftovers from moving house.",
  whysomuchtext = "Is it a lot? I should rethink my offer!",
  successmsg = "Oh wonderful. I'll start unloading immediately. Thanks again.",
  failuremsg = "What are these? Oh, you took so long that I forgot I'd even sent this!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "SHORT-RANGE COURIER. Delivery of a small package to {starport}.",
  introtext = "Hi. I'm {name} and I will pay {cash} for a ship to deliver this package to {starport}.",
  whysomuchtext = "I don't think it's a lot.",
  successmsg = "Thank you for the package, you have been paid in full.",
  failuremsg = "I could have delivered it faster myself. I'm not paying you.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "INTER-PLANETARY CARGO. Freight of local cargo to {starport}.",
  introtext = "Hello. We need these crates delivered to {starport} as soon as possible. Standard payment for this shipment is {cash}.",
  whysomuchtext = "Standard rates, we work with the market.",
  successmsg = "Excellent, we've credited the funds into your account.",
  failuremsg = "Our customers are not going to be happy with this. Do not expect to be paid.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "NEARBY DELIVERY. Require quick delivery of an item to {starport}.",
  introtext = "My name is {name} and I need this item delivered to a friend at {starport} pronto, I'll pay you {cash} credits if you get it there in a reasonable time.",
  whysomuchtext = "It's really urgent.",
  successmsg = "Your prompt delivery is appreciated, I have credited your account accordingly.",
  failuremsg = "You were offered a premium for quick delivery! I refuse to pay for this.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "PACKAGE DROP. Urgent dispatch of perishables to {starport}.",
  introtext = "Greetings, we're behind with our produce shipment and need it delivered to {starport} urgently. We'll pay you {cash} for your troubles.",
  whysomuchtext = "Our livelyhood depends on it.",
  successmsg = "Grand! We'll start unpacking immediately. I'll have your account updated right away.",
  failuremsg = "It's all spoilt, this is of no use to anyone! We cannot and will not pay you.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ English = {
  ["I highly doubt it."] = "I highly doubt it.",
  ["Not any more than usual."] = "Not any more than usual.",
  ["This is a valuable package, you should keep your eyes open."] = "This is a valuable package, you should keep your eyes open.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "It could be dangerous, you should make sure you're adequately prepared.",
  ["This is very risky, you will almost certainly run into resistance."] = "This is very risky, you will almost certainly run into resistance.",
  ["It must be delivered by "] = "It must be delivered by ",
  ["Delivery"] = "Delivery",
  ["Excellent. I will let the recipient know you are on your way."] = "Excellent. I will let the recipient know you are on your way.",
  ["Why so much money?"] = "Why so much money?",
  ["How soon must it be delivered?"] = "How soon must it be delivered?",
  ["Will I be in any danger?"] = "Will I be in any danger?",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["Ok, agreed."] = "Ok, agreed.",
 PIRATE_TAUNTS = {
  "You're going to regret dealing with {client}",
	"Looks like my paycheck has arrived!",
	"You're working for {client}? That was a bad idea.",
	"Your cargo and your life, pilot!",
	"I'm sure this will bring a pretty penny on the market",
	"Today isn't your lucky day! Prepare to die.",
	"Tell my old friend {client} that I'll see them in hell!",
	"That package isn't going to reach it's destination today.",
	"You're not getting to {location} today!",
	"You'll pay for that cargo, with your life.",
  },
}, })

  ---- POLISH / POLSKI ----
  
Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "LECISZ DO systemu {system}? Zapłata za dostarczenie małej paczki.",
  introtext = "Cześć, nazywam się {name}. Zapłacę {cash} jeśli dostarczysz małą paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) .",
  whysomuchtext = "Przyjaciółka po swojej wizycie, zostawiła u mnie ubrania i starodruki. Chciałbym je do niej odesłać.",
  successmsg = "Dziękuję za dostawę. Płacę całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Dostawa zajęła ci całą wieczność. Nic ci nie zapłacę.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PRZESYŁKA. Dostarczenie paczki do systemu {system}.",
  introtext = "Cześć, nazywam się {name}. Oferuję {cash} dla statku który przewiezie paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) .",
  whysomuchtext = "To nic szczególnego.",
  successmsg = "Odebrano paczkę, płacę całość umówionej sumy.",
  failuremsg = "Jestem sfrustrowany tak późną dostawą, odmawiam zapłaty.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PILNE. Potrzebny szybki statek. Dostarczenie paczki do systemu {system}.",
  introtext = "Cześć, nazywam się {name}. Oferuję {cash} dla statku który szybko dostarczy paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) .",
  whysomuchtext = "To jest projekt badawczy i musi być dostarczony w terminie inaczej nie dostaniemy dofinansowania.",
  successmsg = "Płacę całość umówionej sumy. Dziękuję.",
  failuremsg = "Jasno określiłem że zależy mi na dotrzymaniu terminu, jestem bardzo rozczarowany spóźnioną dostawą. Nie zapłacę ci.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "DORĘCZ. Dokumenty do systemu {system}. {cash} dla doświadczonego pilota.",
  introtext = "Cześć, nazywam się {name}. Zapłacę {cash} za dostarczenie dokumentów do  {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) .",
  whysomuchtext = "Pewne bardzo wrażliwe dokumenty wpadły w moje ręce, mam powody sądzić że jestem śledzony przez źródło przecieku.",
  successmsg = "Bardzo cenię twój czas i dyskrecję. Płacę całość umówionej sumy.",
  failuremsg = "Bezużyteczny! Nigdy więcej nie będę na tobie polegać! Chyba nie muszę dodawać że nic nie zapłacę.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "KURIER. Poszukujemy kuriera do systemu {system}.",
  introtext = "Pozdrowienia. To automatyczna wiadomość z Firmy Kurierskiej Bedford i {name}. Zapłacimy {cash} za dostawę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}).",
  whysomuchtext = "Będziemy szczęśliwi mogąc zapłacić ci mniej pieniędzy.",
  successmsg = "Doceniamy twoją punktualność i dyskrecję. Płacimy całość umówionej sumy.",
  failuremsg = "Twój numer identyfikacyjny został zapamiętany, nie licz na znalezienie u nas pracy w przyszłości.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PRZEPROWADZKA. Przewiezienie sprzętu z magazynu do {starport}.",
  introtext = "Miło mi cię poznać. Nazywam się {name} i zapłacę {cash} komuś kto ma statek, za pomoc w przewiezieniu moich rzeczy do {starport}. Bez pośpiechu, to tylko pozostałości po przeprowadzce.",
  whysomuchtext = "To jest dużo? Powinienem przemyśleć swoją ofertę!",
  successmsg = "Wspaniale, natychmiast zaczynam rozładunek. Jeszcze raz dziękuję.",
  failuremsg = "Co to jest? Aha, zajęło ci to tyle czasu,że zdążyłem zapomnieć o tej przesyłce!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "LOKALNY KURIER. Dostarczenie małej paczki do {starport}.",
  introtext = "Cześć, nazywam się {name} i zapłacę {cash} za dostarczenie paczki do {starport}.",
  whysomuchtext = "Nie sądzę by to było dużo.",
  successmsg = "Dziękuję za paczkę, płacę całość umówionej sumy.",
  failuremsg = "Przewiózł bym to szybciej samemu. Nie zapłacę ci.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "FRACHT MIĘDZYPLANETARNY. Transport z lokalnego magazynu do {starport}.",
  introtext = "Cześć. Skrzynie trzeba dostarczyć do {starport}, tak szybko jak to możliwe. Typowa stawka za tą usługę wynosi {cash}.",
  whysomuchtext = "To typowa stawka na rynku zaopatrzenia.",
  successmsg = "Doskonale, Przelaliśmy pieniądze na twoje konto.",
  failuremsg = "Nasi klienci nie będą z tego zadowoleni. Nie spodziewaj się zapłaty.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "LOKALNA PRZESYŁKA. Szybkie dostarczenie elementu do {starport}.",
  introtext = "Nazywam się {name}, potrzebuję szybko dostarczyć pewien element przyjacielowi w {starport}, zapłacę {cash} jeśli dotrzesz tam w rozsądnym czasie.",
  whysomuchtext = "To naprawdę pilne.",
  successmsg = "Doceniam szybką dostawę, przelewam uzgodnioną sumę.",
  failuremsg = "Oferowałem specjalną stawkę za szybką dostawę! Odmawiam zapłaty.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PILNA PSZESYŁKA. Przewóz nietrwałych produktów do {starport}.",
  introtext = "Witamy, zalegamy z wysyłką naszych produktów i potrzebujemy ich szybkiej dostawy do  {starport}. Zapłacimy {cash} za fatygę.",
  whysomuchtext = "Od tego zależy nasze utrzymanie.",
  successmsg = "Imponujące! Natychmiast rozpoczynamy rozładunek. Przelew jest właśnie realizowany.",
  failuremsg = "Wszystko popsute, bezużyteczne! Nie możemy i nie mamy zamiaru płacić.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Polski = {
  ["I highly doubt it."] = "Bardzo w to wątpię.",
  ["Not any more than usual."] = "Nie większe niż zazwyczaj.",
  ["This is a valuable package, you should keep your eyes open."] = "To cenny ładunek, powinieneś mieć oczy szeroko otwarte.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "To może być niebezpieczne, upewnij się że jesteś odpowiednio przygotowany.",
  ["This is very risky, you will almost certainly run into resistance."] = "To bardzo ryzykowne, niemal na pewno napotkasz opór.",
  ["It must be delivered by "] = "Umowa obowiązuje do ",
  ["Delivery"] = "Przesyłka",
  ["Excellent. I will let the recipient know you are on your way."] = "Wspaniale. Dam znać odbiorcy że jesteś w drodze.",
  ["Why so much money?"] = "Dlaczego tyle pieniędzy?",
  ["How soon must it be delivered?"] = "Jaki jest termin dostawy?",
  ["Will I be in any danger?"] = "Jest jakieś zagrożenie?",
  ["Could you repeat the original request?"] = "Możesz powtórzyć swoją ofertę?",
  ["Ok, agreed."] = "Zgoda.",
  PIRATE_TAUNTS = {
  "Pożałujesz kontaktów z {client}",
	"Wygląda na to, że przybyła moja nagroda!",
	"Pracujesz dla {client}? To zły pomysł.",
	"Twój ładunek i twoje życie, pilocie!",
	"Jestem pewien że będzie z tego parę groszy.",
	"To nie jest twój szczęśliwy dzień! Przygotuj się na śmierć.",
	"Powiedz mojemu staremu przyjacielowi {client}, że spotkamy się w piekle!",
	"Ta przesyłka nie dotrze do miejsca przeznaczenia.",
	"Dziś nie dolecisz do {location}!",
	"Zapłacisz za ten ładunek, swoim życiem.",
  },
}, })