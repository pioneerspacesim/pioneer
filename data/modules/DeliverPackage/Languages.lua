-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")

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
  introtext = "Hi, I'm {name}. I'll pay you {cash} if you will deliver a small package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly.",
  whysomuchtext = "When a friend visited me she left behind some clothes and antique paper books. I'd like to have them returned to her.",
  successmsg = "Thank you for the delivery. You have been paid in full.",
  failuremsg = "Unacceptable! You took forever over that delivery. I'm not willing to pay you.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "WANTED. Delivery of a package to the {system} system.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly.",
  whysomuchtext = "It is nothing special.",
  successmsg = "The package has been received and you have been paid in full.",
  failuremsg = "I'm frustrated by the late delivery of my package, and I refuse to pay you.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "URGENT. Fast ship needed to deliver a package to the {system} system.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly.",
  whysomuchtext = "It is a research proposal and must be delivered by the deadline or we may not get funding.",
  successmsg = "You have been paid in full for the delivery. Thank you.",
  failuremsg = "I was quite clear about the deadline and am very disappointed by the late delivery. You will not be paid.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "DELIVERY. Documents to the {system} system. {cash} to an experienced pilot.",
  introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a ship to carry a package to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly.",
  whysomuchtext = "Some extremely sensitive documents have fallen into my hands, and I have reason to believe that the leak has been traced to me.",
  successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
  failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('English','DeliverPackage', {
  adtext = "POSTAL SERVICE. We require a ship for the delivery run to {system} system.",
  introtext = "Greetings. This is an automated message from Bedford and {name} Courier Services. We pay {cash} for the run to {starport} in the {system} ({sectorx}, {sectory}, {sectorz}) system, a distance of {dist} ly.",
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
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["Spaceport:"] = "Spaceport:",
  ["System:"] = "System:",
  ["Deadline:"] = "Deadline:",
  ["Danger:"] = "Danger:",
  ["Distance:"] = "Distance:",

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
  introtext = "Cześć, nazywam się {name}. Zapłacę {cash} jeśli dostarczysz małą paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) ({dist} lś) .",
  whysomuchtext = "Przyjaciółka po swojej wizycie, zostawiła u mnie ubrania i starodruki. Chcę je do niej odesłać.",
  successmsg = "Dziękuję za dostawę. Płacę całość umówionej sumy.",
  failuremsg = "Niedopuszczalne! Dostawa zajęła ci całą wieczność. Nic ci nie zapłacę.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PRZESYŁKA. Dostarczenie paczki do systemu {system}.",
  introtext = "Cześć, nazywam się {name}. Oferuję {cash} dla statku który przewiezie paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) ({dist} lś).",
  whysomuchtext = "To nic szczególnego.",
  successmsg = "Odebrano paczkę, płacę całość umówionej sumy.",
  failuremsg = "Wyrażam frustrację z tak późnej dostawy, odmawiam zapłaty.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PILNE. Potrzebny szybki statek. Dostarczenie paczki do systemu {system}.",
  introtext = "Cześć, nazywam się {name}. Oferuję {cash} dla statku który szybko dostarczy paczkę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) ({dist} lś) .",
  whysomuchtext = "To jest projekt badawczy i musi być dostarczony w terminie inaczej nie dostaniemy dofinansowania.",
  successmsg = "Płacę całość umówionej sumy. Dziękuję.",
  failuremsg = "Było jasne że zależy mi na dotrzymaniu terminu, rozczarowałeś mnie bardzo spóźnioną dostawą. Nie zapłacę ci.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "DORĘCZ. Dokumenty do systemu {system}. {cash} dla doświadczonego pilota.",
  introtext = "Cześć, nazywam się {name}. Zapłacę {cash} za dostarczenie dokumentów do  {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) ({dist} lś) .",
  whysomuchtext = "Pewne bardzo wrażliwe dokumenty wpadły w moje ręce, mam powody sądzić że śledzą mnie jako źródło przecieku.",
  successmsg = "Bardzo cenię twój czas i dyskrecję. Płacę całość umówionej sumy.",
  failuremsg = "Bezużyteczny! Nigdy więcej nie będę na tobie polegać! Chyba nie muszę dodawać że nic nie zapłacę.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "KURIER. Poszukujemy kuriera do systemu {system}.",
  introtext = "Pozdrowienia. To automatyczna wiadomość z Firmy Kurierskiej Bedford i {name}. Zapłacimy {cash} za dostawę do {starport} w systemie {system} ({sectorx}, {sectory}, {sectorz}) ({dist} lś).",
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
  whysomuchtext = "To jest dużo? Chyba przemyślę swoją ofertę!",
  successmsg = "Wspaniale, natychmiast zaczynam rozładunek. Jeszcze raz dziękuję.",
  failuremsg = "Co to jest? Aha, zajęło ci to tyle czasu, że udało mi się zapomnieć o tej przesyłce!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "LOKALNY KURIER. Dostarczenie małej paczki do {starport}.",
  introtext = "Cześć, nazywam się {name} i zapłacę {cash} za dostarczenie paczki do {starport}.",
  whysomuchtext = "Nie sądzę by to było dużo.",
  successmsg = "Dziękuję za paczkę, płacę całość umówionej sumy.",
  failuremsg = "Przewóz osobiście byłby zdecydowanie szybszy. Nie zapłacę ci.",
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
  failuremsg = "Oferta zawierała specjalną stawkę za szybką dostawę! Odmawiam zapłaty.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Polski','DeliverPackage', {
  adtext = "PILNA PRZESYŁKA. Przewóz nietrwałych produktów do {starport}.",
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
  ["ly"] = "lś",

  -- Texts for the missions screen
  ["Spaceport:"] = "Port kosmiczny:",
  ["System:"] = "System:",
  ["Deadline:"] = "Termin:",
  ["Danger:"] = "Zagrożenie:",
  ["Distance:"] = "Dystans:",

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

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "VIAJA al sistema {system}? Se paga dinero por la entrega de un pequeño paquete.",
  introtext = "Qué hay, Soy {name}. Le pagaré {cash} si entrega un pequeño paquete en {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly).",
  whysomuchtext = "Cuando una amiga me visitó se dejó algunas prendas y unos libros antiguos en papel. Me gustaría devolverselos.",
  successmsg = "Gracias por la entrega. Se le ha pagado al contado.",
  failuremsg = "Inaceptable! La entrega ha tardado una eternidad. No tengo intención de pagarle.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "SE BUSCA. Entrega de un paquete al sistema {system}.",
  introtext = "Hola. Soy {name}. Tengo intención de pagar {cash} por una nave que transporte un paquete a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz})) ({dist} ly).",
  whysomuchtext = "No es nada especial.",
  successmsg = "El paquete se ha recibido y se le ha pagado lo acordado.",
  failuremsg = "Estoy frustrado por la tardanza de mi paquete, me niego a pagarle.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "URGENTE. Se necesita una nave rápida para la entrega de un paquete en el sistema {system}.",
  introtext = "Hola. Soy {name}. Y mi intención es pagar {cash} por una nave que transporte un paquete a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz})) ({dist} ly).",
  whysomuchtext = "Es una propuesta de investigación y debe ser entregada en el plazo o no tendremos financiación.",
  successmsg = "Se le ha efectuado el pago completo por la entrega. Gracias.",
  failuremsg = "Creo que fui bastante claro con la fecha tope y estoy muy defraudado por la tardanza. No se le pagará.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "ENTREGA. Documentos al sistema {system}. {cash} para un piloto experimentado.",
  introtext = "Hola. Soy {name}. Tengo la intención de pagar {cash} por una nave que transporte una carga a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz})) ({dist} ly).",
  whysomuchtext = "Ciertos documentos extremadamente sensibles han caido en mis manos, y tengo razones para creer que estoy bajo vigilancia.",
  successmsg = "Se aprecia su servicio discreto y a tiempo. Se le pagará el montante.",
  failuremsg = "Inutil! No volveré a depender de usted en el futuro! Es innecesario decir que no se le pagará ni un crédito.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "SERVICIO POSTAL. Se requiere una nave para una entrega en el sistema {system}.",
  introtext = "Saludos. Este es un mensaje automatizado de los Servicios de Mensajería Bedford y {name}. Pagamos {cash} por la carrera a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz})) ({dist} ly).",
  whysomuchtext = "Estaríamos encantados de pagarle menos dinero.",
  successmsg = "Se aprecia su entrega discreta y a tiempo. Se le ha pagado el montante completo.",
  failuremsg = "Se ha anotado el registro de su nave, vamos a rechazar cualquier solicitud de trabajo suya en el futuro.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "MUDANZA. Traslado de soportes físicos a {starport}.",
  introtext = "Encantado de conocerle. Soy {name} y estoy dispuesto a pagar {cash} a alguien con una nave que me ayude a trasladar mis pertenencias a {starport}. No hay prisa, no son más que algunos restos de la mudanza.",
  whysomuchtext = "Es mucho? Debería replantearme la oferta!",
  successmsg = "Oh maravilloso. Empezaré a desembalar inmediatamente. Gracias de nuevo.",
  failuremsg = "Qué es esto? Oh, ha tardado tanto que me había olvidado de que lo había enviado!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "MENSAJERO DE CORTA DISTANCIA. Entrega de un paquete pequeño en {starport}.",
  introtext = "Qué hay. Soy {name} y pagaré {cash} por una nave que entregue este paquete en {starport}.",
  whysomuchtext = "No creo que sea mucho.",
  successmsg = "Gracias por el paquete, Se le ha pagado el montante completo.",
  failuremsg = "Yo lo habría entregado mas rápido. No le voy a pagar.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "CARGAMENTO INTERPLANETARIO. Transporte de carga local a {starport}.",
  introtext = "Hola. Necesitamos estas cajas entregadas en {starport} tan pronto como sea posible. El pago estandar para este envío es de {cash}.",
  whysomuchtext = "Las tarifas estandar, trabajamos con el mercado.",
  successmsg = "Excelente, hemos añadido los fondos en su cuenta.",
  failuremsg = "Nuestros clientes no se van a alegrar de esto. No espere pago alguno.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "ENTREGA INMEDIATA. Se requiere la entrega express de un objeto a {starport}.",
  introtext = "Mi nombre es {name} y necesito la entrega de este objeto a un amigo en {starport} pronto, Le pagaré {cash} créditos si lo tiene allí en un tiempo razonable.",
  whysomuchtext = "Es muy urgente.",
  successmsg = "Su entrega a tiempo es apreciada, en consecuencia he acreditado su cuenta.",
  failuremsg = "Las condiciones eran una rápida entrega! Me niego a pagarle por esto.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "PAQUETE PENDIENTE. Envío urgente de alimentos perecederos a {starport}.",
  introtext = "Saludos, vamos retrasados con nuestro envio de productos y necesitamos entregarlos a {starport} urgentemente. Le pagaremos {cash} por las molestias.",
  whysomuchtext = "Nuestro sustento depende de esto.",
  successmsg = "Magnífico! Empezaremos a desembalar urgentemente. Le actualizaré la cuenta inmediatamente.",
  failuremsg = "Está todo estropeado, No vale para nada! No podemos pagarle.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Spanish = {
  ["I highly doubt it."] = "Lo dudo mucho.",
  ["Not any more than usual."] = "No mas de lo habitual.",
  ["This is a valuable package, you should keep your eyes open."] = "Es un paquete valioso, debería mantener los ojos abiertos.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "Podría ser peligroso, debería prepararse adecuadamente.",
  ["This is very risky, you will almost certainly run into resistance."] = "Es muy arriesgado, Encontrará resistencia.",
  ["It must be delivered by "] = "La fecha de entrega es ",
  ["Delivery"] = "Entrega",
  ["Excellent. I will let the recipient know you are on your way."] = "Excelente. Haré saber al receptor que está en camino.",
  ["Why so much money?"] = "Por qué tanto dinero?",
  ["How soon must it be delivered?"] = "Cual es la fecha de entrega?",
  ["Will I be in any danger?"] = "Correré peligro?",
  ["Could you repeat the original request?"] = "Podría repetir la petición original?",
  ["Ok, agreed."] = "Vale, De acuerdo.",
 PIRATE_TAUNTS = {
  "Vas a lamentar el hacer negocios con {client}",
	"Al parecer mi cheque ha llegado!",
	"Trabajas para {client}? Eso ha sido una mala idea.",
	"La carga y la vida, piloto!",
	"I'm sure this will bring a pretty penny on the market",
	"Hoy no es tu día de suerte! Prepárate para morir.",
	"Di a mi viejo amigo {client} que les veré en el infierno!",
	"Ese paquete hoy no va a llegar a su destino.",
	"Hoy no vas a llegar a {location}!",
	"Pagarás por ese cargamento, pero con tu vida.",
  },
}, })


 ---- HUNGARIAN / MAGYAR ----

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "ÚTBAN VAGY a(z) {system} rendszerbe? Egy kis csomag elviteléért pénzt fizetek.",
  introtext = "Helló, a nevem {name}. Fizetek neked {cash} összeget, ha elviszel egy kisebb csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly) rendszerben.",
  whysomuchtext = "A múltkor, amikor a barátnőm meglátogatott, itthagyott néhány ruhát és pár régi papírkönyvet. Szeretném visszajuttatni neki.",
  successmsg = "Köszönöm a szállítást, máris megkapod érte a fizetséget.",
  failuremsg = "Elfogadhatatlan! Egy egyszerű szállítás ennyi ideig tartson... nem fogsz fizetséget kapni érte.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "KERESEK. Egy olyan személyt, aki elvinne egy csomagot a(z) {system} rendszerbe.",
  introtext = "Helló. A nevem {name}. Fizetek {cash} kreditet, ha elszállítasz egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly) rendszerben.",
  whysomuchtext = "Semmi különleges.",
  successmsg = "A csomag megérkezett, így megkapod a fizetséget a szállításért.",
  failuremsg = "A szállítási késedelmek miatt nem kapsz semmilyen fizetséget a 'munkáért'.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "SÜRGŐS. Egy gyors hajóval kellene csomagot szállítani a(z) {system} rendszerbe.",
  introtext = "Helló, a nevem {name}. Fizetek {cash} kreditet, ha elviszel egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly) rendszerben.",
  whysomuchtext = "Ez egy kutatási anyag, amelyet időben le kell szállítani, hogy támogatást szerezzek rá.",
  successmsg = "A szállításért máris megkapod a fizetséget. Köszönöm.",
  failuremsg = "Elég egyértelmű voltam a határidővel kapcsolatban, és nem sikerült tartani. Nem kapod meg a fizetséget.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "SZÁLLÍTÁS. Dokumentumok szállítása {system} rendszerbe. A tapasztalt pilóta fizetsége {cash} kredit.",
  introtext = "Hello. A nevem {name}. Fizetek {cash} kreditet annak, aki elvisz egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly) rendszerben.",
  whysomuchtext = "Néhány különösen titkos dokumentum került a birtokomba, és okom van azt hinni, hogy visszavezetnek hozzám a nyomok.",
  successmsg = "Gyors és diszkrét szolgálatodat örömmel vettem, és fizettem ki.",
  failuremsg = "Használhatatlan vagy! Soha nem fogok benned bízni, és szerintem szükségtelen mondanom, hogy semmilyen fizetséged nem kapsz ezért sem.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "POSTAI SZOLGÁLTATÁS. Szükségünk van egy gyors kézbesítésre a(z) {system} rendszerbe.",
  introtext = "Üdvözlet. Ez egy automatikus üzenet a Beford és {name} postaszolgálattól. Fizetünk {cash} kreditet, ha elszállítasz valamit {starport} kikötőbe a(z)  {system} ({sectorx}, {sectory}, {sectorz}) ({dist} ly) rendszerben.",
  whysomuchtext = "Boldogan fizetünk kevesebbet is.",
  successmsg = "Gyorsaságodat és diszkréciódat díjazzuk, amely díjazást már át is utaltuk.",
  failuremsg = "Megjegyeztük a hajód számát, így ne várd, hogy újra alkalmazni fog szolgálatunk téged.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "KÖLTÖZTETÉS. Tárgyak, alkatrészek szállítása a(z) {starport} űrkikötőbe.",
  introtext = "Örülök, hogy találkoztunk. A nevem {name}, és hajlandó vagyok fizetni {cash} kreditet, ha segítesz elszállítani a cuccaimat {starport} kikötőjébe. Semmi fontos, csak néhány hátramaradt dolog költözésből.",
  whysomuchtext = "Hm, ez sok pénz? Talán újra kéne gondolnom az összeget!",
  successmsg = "Na, szuper. Már kezdek is kipakolni. Kösz ismét.",
  failuremsg = "Mi a fenék ezek? Óh, olyan soká tartott idehozni a cuccaimat, hogy elfelejtettem, hogy én küldtem!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "RÖVID TÁVOLSÁGRA FUTÁR. Egy kis csomag szállítását kérném {starport} kikötőjébe.",
  introtext = "Helló. A nevem {name}, és fizetek {cash} kreditet annak, aki elviszi ezt a csomagot {starport} kikötőjébe.",
  whysomuchtext = "Szerintem nem sok ez.",
  successmsg = "Köszönöm a csomagot, máris kifizetlek.",
  failuremsg = "Még én is gyorsabb lettem volna. Nem fizetek neked semmit.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "BOLYGÓKÖZI SZÁLLÍTÁS. Némi rakomány szállítása {starport} kikötőjébe.",
  introtext = "Helló. Szeretnék, ha ezeket a dobozokat elvinnéd {starport} kikötőjébe, amilyen gyorsan tudod. A szállítás szokványos fizetsége {cash} kredit.",
  whysomuchtext = "Szokványos fizetség, a piaci árak ilyenek.",
  successmsg = "Kitünő munka, máris átutalom a fizetséged.",
  failuremsg = "Az ügyfeleink nem fognak ennek örülni, így ne is várj fizetséget.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "KÖZELI CÉLÁLLOMÁS. Gyors kiszállítás szükséges a(z) {starport} közeli űrkikötőbe.",
  introtext = "A nevem {name}. Szeretném ezt a tárgyat azonnal eljuttatni egy barátomnak {starport} kikötőjébe. Fizetek a szállításért {cash} kreditet, ha elfogadható gyorsasággal végrehajtod a munkát.",
  whysomuchtext = "Rettentően sürgős.",
  successmsg = "Gyors és pontos szállításod pontos fizetséget is érdemel.",
  failuremsg = "A gyors szállításért kaptál volna prémiumot! Ezt viszont nem fogom kifizetni.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "CSOMAGSZÁLLÍTÁS. Sürgős, romlandó áru szállítása {starport} kikötőjébe.",
  introtext = "Üdvözöllek, sürgősen el szeretnénk juttatni a szállítmányunkat {starport} kikötőjébe. Fizetünk a kényelmetlenségekért {cash} kreditet.",
  whysomuchtext = "A túlélésünk múlik rajta.",
  successmsg = "Nagyszerű! Máris elkezdjük kipakolni. Már át is utaltam a fizetséged.",
  failuremsg = "Mindennek annyi, már nem lehet használni ezeket! Nem tudunk és nem is akarunk ezért fizetni neked.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Magyar = {
  ["I highly doubt it."] = "Azt erősen kétlem.",
  ["Not any more than usual."] = "Nem több, mint általában.",
  ["This is a valuable package, you should keep your eyes open."] = "Ez egy értékes csomag, szóval légy körültekintő.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "Lehet, hogy veszélyes, szóval legyél nagyon felkészült.",
  ["This is very risky, you will almost certainly run into resistance."] = "Hihetetlenül veszélyes, biztos, hogy ellenállásba fogsz ütközni.",
  ["It must be delivered by "] = "Le kell szállítani ",
  ["Delivery"] = "Szállítás",
  ["Excellent. I will let the recipient know you are on your way."] = "Kitünő. Tudatom a címzettel, hogy úton van a csomag.",
  ["Why so much money?"] = "Miért ennyi a munkadíj?",
  ["How soon must it be delivered?"] = "Mikorra kell leszállítani?",
  ["Will I be in any danger?"] = "Fenyeget valamilyen veszély közben?",
  ["Could you repeat the original request?"] = "Megismételnéd az eredeti feladatot?",
  ["Ok, agreed."] = "Oké, elvállalom.",
 PIRATE_TAUNTS = {
  "Megbánod még, hogy {client} fizeti a munkádat!",
	"Úgy tűnik, megjött a fizetési csekkem!",
	"Nocsak, {client} a megbízód? Ez egy nagyon rossz ötlet volt.",
	"A rakományod és az életed!",
	"Úgy gondolom, ez megér némi pénzt a feketepiacon",
	"A mai nap nem a szerencsenapod! Készülj a halálra.",
	"Mondd meg régi barátomnak: {client}... viszlát a pokolban!",
	"Ez a csomag nem fog ma elérni a célállomásra.",
	"Nem fogsz eljutni {location} célodhoz!",
	"Ezért a csomagért megfizetsz, méghozzá az életeddel.",
  },
}, })

 ---- RUSSIAN / Русский ----

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "ПОСЫЛКА. В систему {system} необходимо доставить небольшую посылку. Оплата {cash}.",
  introtext = "Здравствуйте, меня зовут {name}.\n Заплачу {cash} за доставку небольшой посылки на станцию {starport} в системе {system} ({sectorx}, {sectory}, {sectorz}). Расстояние {dist} св.л.",
  whysomuchtext = "Мне надо отправить некоторые вещи своему другу.",
  successmsg = "Спасибо за доставку! Оплата как договаривались.",
  failuremsg = "Недопустимо! Мы уже и не ждали доставки! - я не буду платить.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "ДОСТАВКА. Необходимо доставить пакет в систему {system}. Оплата {cash}.",
  introtext = "Приветствую, я {name}.\n Заплачу {cash} капитану корабля за быструю доставку пакета на станцию {starport} в системе {system} ({sectorx}, {sectory}, {sectorz}) . Расстояние {dist} св.л.",
  whysomuchtext = "Ничего особенного - просто доставка пакета.",
  successmsg = "Вы доставили пакет, я оплатил оговоренную сумму.",
  failuremsg = "Меня разочаровала задержка поставки - я не буду платить.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "СРОЧНО. Нужен быстрый корабль для доставки посылки в систему {system}. Оплата {cash}.",
  introtext = "Здравствуйте, моё имя {name}. Заплачу {cash} капитану, который быстро доставит посылку на станцию {starport} в системе {system} ({sectorx}, {sectory}, {sectorz}) . Расстояние {dist} св.л.",
  whysomuchtext = "Мне нужно срочно доставить исследовательский проект заказчику, иначе мы не получим финансирование.",
  successmsg = "Деньги перечислены на ваш счёт. Спасибо!",
  failuremsg = "Кажется мною было сказано, что от этой посылки зависит наше финансирование!\n Вы разочаровали меня! Оплаты не будет.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "ДОСТАВКА. Необходим опытный пилот для доставки документов в систему {system}. Оплата {cash}.",
  introtext = "Здравствуйте, я {name}. Заплачу {cash} за доставку документов на станцию {starport} в системе {system} ({sectorx}, {sectory}, {sectorz}) . Расстояние {dist} св.л.",
  whysomuchtext = "Мне в руки попали очень важные документы. Есть основания полагать, что об этом скоро узнают не те люди.",
  successmsg = "Я оценил вашу скорость и профессионализм. Оплата согласно контракту!",
  failuremsg = "Это возмутительно! Мы больше не будем иметь с вами дел! Излишне говорить, что оплаты не будет?.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "КУРЬЕР. Требуется для полёта в систему {system}. Оплата {cash}.",
  introtext = "Доброго времени суток!\n Это автоматическое сообщение от Курьерской Фирмы Bedford & {name}. Заплатим {cash} за доставку груза на станцию {starport} в системе {system} ({sectorx}, {sectory}, {sectorz}). Расстояние {dist} св.л.",
  whysomuchtext = "Мы всего лишь делаем наше дело.",
  successmsg = "Мы ценим вашу пунктуальность и профессионализм. Деньги переведены на счёт.",
  failuremsg = "Ваш идентификационный номер был записан. Не рассчитывайте на работу в будущем..",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "ПЕРЕВОЗКА. Необходимо доставить оборудование со склада на станцию {starport}. Оплата {cash}.",
  introtext = "Приятно познакомиться. Я {name}. \n Заплачу {cash} за перевозку оборудования на станцию {starport}.",
  whysomuchtext = "Мне надо доставить оборудование, без спешки.",
  successmsg = "Великолепно, немедленно начинаю загрузку! Спасибо большое!",
  failuremsg = "Что это?! А, это вы? Я уже успел забыть о грузе...",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "МЕЖПЛАНЕТНЫЙ КУРЬЕР. Доставка небольшой посылки на станцию {starport}. Оплата {cash}.",
  introtext = "Приветствую, я {name}. Заплачу {cash} за доставку посылки на станцию {starport}.",
  whysomuchtext = "Нужны курьерские услуги, готов немного заплатить.",
  successmsg = "Спасибо за доставку, плачу как договорились.",
  failuremsg = "Надо было отправить посылку с голубем - быстрее бы дошла. Я не собираюсь платить.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "МЕЖПЛАНЕТНАЯ ПЕРЕВОЗКА. Нужен транспорт для доставки груза из местного магазина на станцию {starport}. Оплата {cash}.",
  introtext = "Привет!\n Контейнеры должны быть доставлены на станцию {starport} так быстро, как только можно. Стандартная ставка за услугу - {cash}.",
  whysomuchtext = "Доставка груза за стандартную ставку.",
  successmsg = "ОК, деньги переведены на ваш счёт.",
  failuremsg = "Наши клиенты не обрадуются вашей задержке. Оплаты не будет.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "МЕСТНАЯ ДОСТАВКА. Требуется быстро доставить груз на станцию {starport}. Оплата {cash}.",
  introtext = "Зовите меня {name}.\n Мне нужно быстро доставить груз другу на станцию {starport}.\n Заплачу {cash}, если успеете в разумные сроки.",
  whysomuchtext = "Требуется очень срочно доставить груз.",
  successmsg = "Я ценю быструю доставку, деньги ваши.",
  failuremsg = "Была установлена специальная ставка именно за скорость! Я не буду платить.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Russian','DeliverPackage', {
  adtext = "МЕСТНАЯ ДОСТАВКА. Перевозка скоропортящихся продуктов на станцию {starport}. Оплата {cash}.",
  introtext = "Привет, нам нужно срочно доставить скоропортящиеся продукты на станцию {starport}. Заплатим {cash} за скорость.",
  whysomuchtext = "От этого зависит наше благосостояние.",
  successmsg = "Впечатляет! Немедленно приступаем к разгрузке. Деньги ваши.",
  failuremsg = "Чёрт, всё протухло! Тут уже не за что платить!",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Russian = {
  ["I highly doubt it."] = "Я сомневаюсь в этом.",
  ["Not any more than usual."] = "Не больше, чем обычно.",
  ["This is a valuable package, you should keep your eyes open."] = "Это ценный груз - будьте внимательны.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "Вас могут поджидать опасности - убедитесь, что готовы к этому.",
  ["This is very risky, you will almost certainly run into resistance."] = "Дело довольно рискованное - скорее всего за грузом будут охотиться.",
  ["It must be delivered by "] = "Груз должен быть доставлен до ",
  ["Delivery"] = "Доставка",
  ["Excellent. I will let the recipient know you are on your way."] = "Отлично! Я сообщу получателю, что вы вылетаете.",
  ["Why so much money?"] = "Цель заключения контракта?",
  ["How soon must it be delivered?"] = "Каковы сроки доставки?",
  ["Will I be in any danger?"] = "Предвидятся какие-либо проблемы?",
  ["Could you repeat the original request?"] = "Не могли бы вы повторить ваше предложение?",
  ["Ok, agreed."] = "Хорошо, договорились.",
  ["ly"] = "св.лет›",  

  -- Texts for the missions screen
  ["Spaceport:"] = "Станция получателя:",
  ["System:"] = "Система получателя:",
  ["Deadline:"] = "Срок доставки:",
  ["Danger:"] = "Опасность:",
  ["Distance:"] = "Расстояние:",

  PIRATE_TAUNTS = {
  "Ты ответишь за контракт с {client}!!",
	"Похоже, что прибыл мой приз!",
	"Работаешь на {client}? Это была плохая идея!",
	"Твой кошелёк и твоя жизнь, пилот!!!",
	"Я уверен, что удастся поживиться десятком кредо.",
	"Сегодня не твой день! Готовься к смерти.",
	"Передай моему старому дружку {client}, что мы встретимся в аду!",
	"Эта посылка не дойдет до адресата!",
	"Тебе не долететь до {location}!",
	"Ты заплатишь за эту посылку - своей жизнью!",
  },
}, })

  ---- GERMAN / DEUTSCH ----

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "AUF DEM WEG zum {system} System? Geld für die Ablieferung eines kleinen Paketes.",
  introtext = "Hi, ich bin {name}. Ich zahle dir {cash}, wenn du dieses kleine Paket für mich bei {starport} im {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly enfernt, ablieferst.",
  whysomuchtext = "Als mich eine Freundin besucht hat, hat sie einige Kleider und antike Bücher hier vergessen. Ich möchte, dass sie sie wiederbekommt.",
  successmsg = "Danke für die Lieferung. Du wurdest voll bezahlt.",
  failuremsg = "Unakzeptabel! Du hast ewig für die Lieferung gebraucht. Ich werde dich nicht bezahlen.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "GESUCHT. Lieferung eines Paketes zum {system} System.",
  introtext = "Hallo. Ich bin {name}. Ich werde demjenigen {cash} zahlen, der ein Paket für mich nach {starport} im {system} ({sectorx}, {sectory}, {sectorz}) System, in einer Entfernung von {dist} ly bringt.",
  whysomuchtext = "Es ist nichts spezielles.",
  successmsg = "Das Paket wurde empfangen, du wurdest dafür entlohnt.",
  failuremsg = "Ich bin frustriert von der späten Lieferung und ich werde dich nicht bezahlen.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "DRINGEND. Schnelles Schiff gesucht, das ein Paket zum {system} System bringen kann.",
  introtext = "Hallo. Ich bin {name}. Ich werde demjenigen {cash} zahlen, der ein Paket für mich nach {starport} im {system} ({sectorx}, {sectory}, {sectorz}) System, in einer Entfernung von {dist} ly bringt.",
  whysomuchtext = "Es ist ein Forschungsansatz, der schnell geliefert werden muss. Es muss bis zur deadline geliefert sein, oder wir bekommen keine Zuschüsse.",
  successmsg = "Du hast deinen vollen Lohn erhalten. Wir danken dir.",
  failuremsg = "Ich denke, ich war ziemlich deutlich, was die deadline angeht und bin über die späte Lieferung sehr enttäuscht. Du wirst nicht bezahlt werden.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "LIEFERUNG. Dokumente zum {system} System. {cash} für einen erfahrenen Piloten.",
  introtext = "Hallo. Ich bin {name}. Ich werde {cash} für ein Schiff zahlen, das ein Paket nach {starport} im {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly entfernt, bringen kann.",
  whysomuchtext = "Einige extrem heikle Dokumente sind mir in die Hände gefallen und ich habe Grund zur Annahme, dass das Leck bis zu mir zurückverfolgt worden ist.",
  successmsg = "Ich weiß deine püntkliche und diskrete Lieferung zu Schätzen. Du erhälst deinen vollen Lohn.",
  failuremsg = "Nutzlos! Ich werde mich nie mehr auf dich stützen! Natürlich werde ich dich nicht bezahlen.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "POSTDIENST. Wir brauchen ein Schiff für eine Lieferung zum {system} System.",
  introtext = "Hallo. Dies ist eine automatische Nachricht vom Bedford und {name} Kurierdienst. Wir zahlen {cash} für eine Lieferung nach {starport} im {system} ({sectorx}, {sectory}, {sectorz}) System, {dist} ly entfernt.",
  whysomuchtext = "Wir können auch weniger zahlen, kein Problem für uns.",
  successmsg = "Ich weiß deine püntkliche und diskrete Lieferung zu Schätzen. Du erhälst deinen vollen Lohn.",
  failuremsg = "Die Registrierungs-ID deines Schiffes wurde notiert, wir werden all deine späteren Bewerbungen ablehnen.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "UMZUG. Transport von Haushaltswagen nach {starport} aus einem Lager.",
  introtext = "Ich bin erfreut dich zu treffen. Ich bin {name} und ich biete {cash} für jemanden mit einem Schiff, der mir helfen kann, meine Sachen nach {starport} zu bringen. Keine Eile, das ist nur, was vom Umzug noch übrig ist.",
  whysomuchtext = "Ist das viel? Ich sollte mein Angebot überdenken!",
  successmsg = "Oh, wunderbar. Ich werde sofort Ausladen. Danke nochmal.",
  failuremsg = "Was ist das? Oh, Du hast so lange gebraucht, dass ich vergessen habe, das überhaupt gesendet zu haben!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "KURZSTRECKEN-KURIER. Lieferung eines kleinen Paketes nach {starport}.",
  introtext = "Hi. Ich bin {name} und ich zahle {cash}, wenn jemand dieses Paket nach {starport} bringt.",
  whysomuchtext = "Das ist doch nicht viel.",
  successmsg = "Danke für das Paket. Hier ist deine Bezahlung.",
  failuremsg = "Ich hätte es selbst schneller liefern können. Ich werde dich nicht bezahlen.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "INTERPLATETARISCHE FRACHT. Lokale Frachtlieferung nach {starport}.",
  introtext = "Hallo. Diese Kisten müssen schnellstmöglich nach {starport} geliefert werden. Standardbezahlung für diese Strecke ist {cash}.",
  whysomuchtext = "Standardtarif. Wir arbeiten mit dem Markt.",
  successmsg = "Exzellent, wir haben dir den Lohn auf dein Konto überwiesen.",
  failuremsg = "Unsere Kunden werden damit nicht zufrieden sein. Erwarte keine Bezahlung.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "KURZSTRECKEN-LIEFERUNG. Brauche schnelle Lieferung nach {starport}.",
  introtext = "Mein Name ist {name} und dieser Gegenstand muss schnell zu einem Freund nach {starport} geliefert werden, ich werde dir  {cash}, wenn du es in einer angemessenen Zeit dort ablieferst.",
  whysomuchtext = "Es ist wirklich dringend.",
  successmsg = "Ich schätze deine schnelle Lieferung. Ich habe dir den abgemachten Betrag überwiesen.",
  failuremsg = "Das Geld wurde für eine SCHNELLE Lieferung versprochen! Ich weigere mich für das hier zu Bezahlen.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Deutsch','DeliverPackage', {
  adtext = "PACKET-ABWURF. Dringende Auslieferung von verderblichen Waren nach {starport}.",
  introtext = "Hallo, wir liegen mit der Auslieferung unserer Produkte zurück und brauchen eine schnelle Lieferung nach {starport}. Wir werden die Unannehmlichkeiten mit {cash} entlohnen.",
  whysomuchtext = "Unsere Existenz hängt davon ab.",
  successmsg = "Großartig! Wir werden sofort auspacken. Ich habe das Geld schon mal überwiesen.",
  failuremsg = "Alles ist verdorben, alles Abfall! Wir können und wollen dich nicht entlohnen.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Deutsch = {
  ["I highly doubt it."] = "Ich bezweifele das.",
  ["Not any more than usual."] = "Nicht mehr als üblich.",
  ["This is a valuable package, you should keep your eyes open."] = "Das ist ein wertvolles Paket, du solltest die Augen offenhalten.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "Es könnte gefährlich sein, du solltest besser gut ausgerüstet sein.",
  ["This is very risky, you will almost certainly run into resistance."] = "Das ist sehr riskant, du wirst mit hoher Wahrscheinlichkeit auf Wiederstand treffen.",
  ["It must be delivered by "] = "Es muss geliefert werden bis ",
  ["Delivery"] = "Lieferung",
  ["Excellent. I will let the recipient know you are on your way."] = "Exzellent. Ich werde den Empfänger wissen lassen, dass du auf dem Weg bist.",
  ["Why so much money?"] = "Warum so viel Geld?",
  ["How soon must it be delivered?"] = "Bis wann muss es geliefert sein?",
  ["Will I be in any danger?"] = "Wird es gefährlich sein?",
  ["Could you repeat the original request?"] = "Was war nochmal die anfängliche Frage??",
  ["Ok, agreed."] = "Ok, ich bin dabei.",
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["Spaceport:"] = "Raumhafen:",
  ["System:"] = "System:",
  ["Deadline:"] = "Deadline:",
  ["Danger:"] = "Gefahr:",
  ["Distance:"] = "Distanz:",

 PIRATE_TAUNTS = {
  "Du wirst es bedauern, dich mit {client} zusammengetan zu haben",
	"Sieht so aus, als wäre gerade mein Lohnscheck angekommen!",
	"Du arbeitest für {client}? Das war keine gute Idee.",
	"Deine Fracht und dein Leben, Pilot!",
	"Ich bin sicher, man kann damit auf dem Markt ne Menge Kohle machen.",
	"Heute ist nicht dein Glückstag. Stell dich auf deinen Tod ein.",
	"Sag meinem alten Freund {client}, dass wir uns in der Hölle sehen!",
	"Dieses Paket wird heute nicht seinen Ankunftsort erreichen.",
	"Du gehst heute nicht nach {location}!",
	"Du wirst für diese Fracht mit deinem Leben bezahlen.",
  },
}, })

  ---- CZECH / ČESKY ----

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "LETÍTE DO systému {system}? Zaplatím vám za doručení malého balíku.",
  introtext = "Ahoj, jsem {name}. Zaplatím vám {cash} když doručíte malý balík do stanice {starport} v systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly.",
  whysomuchtext = "Kdysi mě navštívila kamarádka a zanechala tu nějaké oblečení a starobylé papírové knihy. Rád(a) bych aby je dostala zpět.",
  successmsg = "Děkuji za doručení balíku. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Nepřijatelné! To doručení ti trvalo věčnost. Nedostaneš zaplaceno!",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "HLEDÁM. Doručení balíku do systému {system}.",
  introtext = "Zdravím. Jsem {name}. Zaplatím vám {cash}, když lodí vezmete balík do stanice {starport} v systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly.",
  whysomuchtext = "It is nothing special.",
  whysomuchtext = "Nejde o nic zvláštního.",
  successmsg = "Balík byl převzat, dostaneš zaplaceno v plné výši.",
  failuremsg = "Jsem zklamaný(á) že doručení trvalo tak dlouho. Odmítám ti zaplatit.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "URGENTNÍ. Hledá se rychlá loď k doručení balíku do systému {system}.",
  introtext = "Ahoj. Jsem {name}. Zaplatím vám {cash}, když lodí vezmete balík do stanice {starport} v systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly.",
  whysomuchtext = "Jde o výzkumný projekt a musí být rychle doručen. Je nutné dodržet lhůtu pro doručení, jinak bude ohroženo financování projektu.",
  successmsg = "Dostal jste zaplaceno v plné výši. Děkuji.",
  failuremsg = "Jsem velmi zklamaný(á), myslím, že jsem dodržení lhůty doručení dostatečně zdůraznil(a). Nedostaneš zaplaceno!",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "DORUČOVÁNÍ. Dokumenty do systému {system}. {cash} zkušenému pilotovi.",
  introtext = "Ahoj. Jsem {name}. Zaplatím vám {cash}, když lodí vezmete balík do stanice {starport} v systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly.",
  whysomuchtext = "Dostaly se mi do rukou extrémně citlivé dokumenty a mám důvod se domnívat, že stopy vedou zpět ke mě.",
  successmsg = "Velmi oceňuji tvůj rychlý a diskrétní přístup. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Jsi k ničemu! Už se na tebe nikdy nespolehnu! Je zbytečné dodávat, že nedostaneš zaplaceno.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "POŠTOVNÍ SLUŽBA. Hledá se loď pro rozvážku do systému {system}.",
  introtext = "Zdravím. Toto je automatická zpráva z Bedford & {name} zásilkové služby. Zaplatíme {cash} za rozvážku do stanice {starport} v systému {system} ({sectorx}, {sectory}, {sectorz}), vzdáleném {dist} ly.",
  whysomuchtext = "No můžeme zaplatit i míň, to pro nás nebude problém.",
  successmsg = "Velmi oceňujeme tvůj rychlý a diskrétní přístup. Dostaneš zaplaceno v plné výši.",
  failuremsg = "Registrační ID tvé lodi si značíme, veškeré budoucí pokusy o spolupráci budou zamítnuty.",
  urgency = 0.1,
  risk = 0.1,
  localdelivery = 0,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "STĚHOVÁNÍ. Přeprava věcí z úschovny do stanice {starport}.",
  introtext = "Rád(a) vás poznávám. Jsem {name} a zaplatím {cash} pokud mi někdo s lodí pomůže přepravit můj majetek do stanice {starport}. Není třeba spěchat, jde jen o pár zbytků po stěhování z domu.",
  whysomuchtext = "Je to moc? Tak to bych si svou nabídku měl(a) ještě promyslet!",
  successmsg = "Oh skvělé. Hned začnu vybalovat. Ještě jednou díky.",
  failuremsg = "Co je tohle? Oh, trvalo ti to tak dlouho, že jsem na to už zapoměl(a)!",
  urgency = 0.1,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "KURÝR na KRÁTKÉ TRASY. Doručení malého balíku do stanice {starport}.",
  introtext = "Ahoj. Jsem {name} a zaplatím {cash} když lodí vezmete tento balík do stanice {starport}.",
  whysomuchtext = "Nemyslím si že je to moc.",
  successmsg = "Děkuji za doručení, dostaneš zaplaceno v plné výši.",
  failuremsg = "To už by to rychlej doručila moje babička. Nezaplatím.",
  urgency = 0.2,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "MEZIPLANETÁRNÍ NÁKLAD. Doprava kontejnerů do stanice {starport}.",
  introtext = "Zdravím. Potřebujeme dopravit tyto kontejnery do stanice {starport} tak rychle, jak je to jen možné. Standardní platba za přepravu činí {cash}.",
  whysomuchtext = "To je standardní tarif na trhu.",
  successmsg = "Výborně, poslali jsme platbu na váš účet.",
  failuremsg = "S takovým přístupem nebudou naši zákazníci spokojeni. Nečekej že ti zaplatíme.",
  urgency = 0.4,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "DORUČOVÁNÍ v SOUSEDSTVÍ. Požadováno rychlé doručení předmětu do stanice {starport}.",
  introtext = "Jmenuji se {name} a potřebuji pronto doručit tento předmět příteli do stanice {starport}. Zaplatím ti {cash}, když to zvládneš v rozumném čase.",
  whysomuchtext = "Protože to opravdu spěchá.",
  successmsg = "Tvůj rychlý přístup velmi oceňuji. Platba je proto již na tvém účtu.",
  failuremsg = "Tomu říkáš rychlé doručení? Odmítám za to ještě platit!",
  urgency = 0.6,
  risk = 0,
  localdelivery = 1,
})

Translate:AddFlavour('Czech','DeliverPackage', {
  adtext = "SPĚŠNÝ BALÍK. Urgentní přeprava zboží podléhajícího rychlé zkáze do stanice {starport}.",
  introtext = "Zdravím, jsme pozadu s odbavováním našich zakázek, takže je potřebujeme fofrem dopravit do stanice {starport}. Za problémy s tím spojené ti zaplatíme {cash}.",
  whysomuchtext = "Závisí na tom naše živobytí.",
  successmsg = "Senzační! Okamžitě začneme vybalovat. Ihned zadám příkaz k platbě na tvůj účet.",
  failuremsg = "Všechno se zkazilo a můžeme to akorát tak vyhodit! Za to ti nemůžeme a ani nechceme zaplatit.",
  urgency = 0.8,
  risk = 0,
  localdelivery = 1,
})

Translate:Add({ Czech = {
  ["I highly doubt it."] = "O tom vážně pochybuji.",
  ["Not any more than usual."] = "Ne víc než normálně.",
  ["This is a valuable package, you should keep your eyes open."] = "Je to velmi cenný balík, měj oči otevřené.",
  ["It could be dangerous, you should make sure you're adequately prepared."] = "Může to být nebezpečné. Ujisti se, že jsi na to dostatečně připraven(a).",
  ["This is very risky, you will almost certainly run into resistance."] = "Je to velmi riskantní, zcela jistě se dostaneš do potíží.",
  ["It must be delivered by "] = "Doručení musí proběhnout do ",
  ["Delivery"] = "Doručení",
  ["Excellent. I will let the recipient know you are on your way."] = "Výborně. Uvědomím příjemce že jsi na cestě.",
  ["Why so much money?"] = "Proč tolik peněz?",
  ["How soon must it be delivered?"] = "Dokdy musí být zásilka doručena?",
  ["Will I be in any danger?"] = "Ocitnu se v nějakém nebezpečí?",
  ["Could you repeat the original request?"] = "Můžete mi znovu zopakovat váš požadavek?",
  ["Ok, agreed."] = "OK, souhlasím.",
  ["ly"] = "ly",

  -- Texts for the missions screen
  ["Spaceport:"] = "Stanice:",
  ["System:"] = "Systém:",
  ["Deadline:"] = "Termín:",
  ["Danger:"] = "Nebezpečí:",
  ["Distance:"] = "Vzdálenost:",

  PIRATE_TAUNTS = {
	"Budeš litovat, že jsis něco začal s {client}",
	"Vypadá to, že můj výplatní šek právě dorazil!",
	"Ty pracuješ pro {client}? To nebyl dobrý nápad.",
	"Tvůj náklad a tvůj život, pilote!",
	"Jsem si jistý, že s tímhle se dá na trhu vydělat pěkná škvára",
	"Dnes není tvůj šťastný den! Připrav se na smrt.",
	"Vyřiď mému starému příteli {client}, že se uvidíme v pekle!",
	"Tak tenhle balík už do svého cíle dnes nedorazí.",
	"Ty už dnes do {location} nedorazíš!",
	"Za ten náklad zaplatíš, svým životem!",
  },
}, })
