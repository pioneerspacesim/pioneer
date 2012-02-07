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

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "VIAJA al sistema {system}? Se paga dinero por la entrega de un pequeño paquete.",
  introtext = "Qué hay, Soy {name}. Le pagaré {cash} si entrega un pequeño paquete en {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
  whysomuchtext = "Cuando una amiga me visitó se dejó algunas prendas y unos libros antiguos en papel. Me gustaría devolverselos.",
  successmsg = "Gracias por la entrega. Se le ha pagado al contado.",
  failuremsg = "Inaceptable! La entrega ha tardado una eternidad. No tengo intención de pagarle.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "SE BUSCA. Entrega de un paquete al sistema {system}.",
  introtext = "Hola. Soy {name}. Tengo intención de pagar {cash} por una nave que transporte un paquete a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
  whysomuchtext = "No es nada especial.",
  successmsg = "El paquete se ha recibido y se le ha pagado lo acordado.",
  failuremsg = "Estoy frustrado por la tardanza de mi paquete, me niego a pagarle.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "URGENTE. Se necesita una nave rápida para la entrega de un paquete en el sistema {system}.",
  introtext = "Hola. Soy {name}. Y mi intención es pagar {cash} por una nave que transporte un paquete a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
  whysomuchtext = "Es una propuesta de investigación y debe ser entregada en el plazo o no tendremos financiación.",
  successmsg = "Se le ha efectuado el pago completo por la entrega. Gracias.",
  failuremsg = "Creo que fui bastante claro con la fecha tope y estoy muy defraudado por la tardanza. No se le pagará.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "ENTREGA. Documentos al sistema {system}. {cash} para un piloto experimentado.",
  introtext = "Hola. Soy {name}. Tengo la intención de pagar {cash} por una nave que transporte una carga a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
  whysomuchtext = "Ciertos documentos extremadamente sensibles han caido en mis manos, y tengo razones para creer que estoy bajo vigilancia.",
  successmsg = "Se aprecia su servicio discreto y a tiempo. Se le pagará el montante.",
  failuremsg = "Inutil! No volveré a depender de usted en el futuro! Es innecesario decir que no se le pagará ni un crédito.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Spanish','DeliverPackage', {
  adtext = "SERVICIO POSTAL. Se requiere una nave para una entrega en el sistema {system}.",
  introtext = "Saludos. Este es un mensaje automatizado de los Servicios de Mensajería Bedford y {name}. Pagamos {cash} por la carrera a {starport} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
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
  introtext = "Helló, a nevem {name}. Fizetel neked {cash} összeget, ha elviszel egy kisebb csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerben.",
  whysomuchtext = "A múltkor, amikor a barátnőm meglátogatott, itthagyott néhány ruhát és pár régi papírkönyvet. Szeretném visszajuttatni neki.",
  successmsg = "Köszönöm a szállítást, máris megkapod érte a fizetséget.",
  failuremsg = "Elfogadhatatlan! Egy egyszerű szállítás ennyi ideig tartson... nem fogsz fizetséget kapni árte.",
  urgency = 0,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "KERESEK. Egy olyan személyt, aki elvinne egy csomagot a(z) {system} rendszerbe.",
  introtext = "Helló. A nevem {name}. Fizetek {cash} kreditet, ha elszállítasz egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerben.",
  whysomuchtext = "Semmi különleges.",
  successmsg = "A csomag megérkezett, így megkapod a fizetséget a szállításért.",
  failuremsg = "A szállítási késedelmek miatt nem kapsz semmilyen fizetséget a 'munkáért'.",
  urgency = 0.1,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "SÜRGŐS. Egy gyors hajóval kellene csomagot szállítani a(z) {system} rendszerbe.",
  introtext = "Helló, a nevem {name}. Fizetek {cash} kreditet, ha elviszel egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerben.",
  whysomuchtext = "Ez egy kutatási anyag, amelyet időben le kell szállítani, hogy támogatást szerezzek rá.",
  successmsg = "A szállításért máris megkapod a fizetséget. Köszönöm.",
  failuremsg = "Elég egyértelmű voltam a határidővel kapcsolatban, és nem sikerült tartani. Nem kapod meg a fizetséget.",
  urgency = 0.6,
  risk = 0,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "SZÁLLÍTÁS. Dokumentumok szállítása {system} rendszerbe. A tapasztalt pilóta fizetsége {cash} kredit.",
  introtext = "Hello. A nevem {name}. Fizetek {cash} kreditet annak, aki elvisz egy csomagot {starport} kikötőjébe a(z) {system} ({sectorx}, {sectory}, {sectorz}) rendszerben.",
  whysomuchtext = "Néhány különösen titkos dokumentum került a birtokomba, és okom van azt hinni, hogy visszavezetnek hozzám a nyomok.",
  successmsg = "Gyors és diszkrét szolgálatodat örömmel vettem, és fizettem ki.",
  failuremsg = "Használhatatlan vagy! Soha nem fogok benned bízni, és szerintem szükségtelen mondanom, hogy semmilyen fizetséged nem kapsz ezért sem.",
  urgency = 0.4,
  risk = 0.75,
  localdelivery = 0,
})

Translate:AddFlavour('Magyar','DeliverPackage', {
  adtext = "POSTAI SZOLGÁLTATÁS. Szükségünk van egy gyors kézbesítésre a(z) {system} rendszerbe.",
  introtext = "Üdvözlet. Ez egy automatikus üzenet a Beford és {name} postaszolgálattól. Fizetünk {cash} kreditet, ha elszállítasz valamit {starport} kikötőbe a(z)  {system} ({sectorx}, {sectory}, {sectorz}) rendszerben.",
  whysomuchtext = "Boldogan fizetünk kevesebbet is.",
  successmsg = "Gyorsaságodat és diszkréciódat díjazzuk, amely díjazást már át is utaltunk.",
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
  ["Delivery"] = "Delivery",
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
