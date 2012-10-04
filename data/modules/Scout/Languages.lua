-- Copyright © 2008-2012 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

	-- adtext - text shown in the bulletin board list
	-- introtext - shown when the advert is selected (and "Could you repeat request?")
	-- whysomuchtext - response to "Why so much?"
	-- successmsg - message sent on successful mission
	-- failuremsg - message sent on failed mission
	-- urgency - how urgent the mission is. 0 is surface mail. 1 is overnight
	-- risk - how risky the mission is. 0 is letters from mother. 1 is certain death
	-- local - 1 if the mission is to the local (this) system, 0 otherwise

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Scout', {
		adtext = "Searching Pilot for reconnaissance in the {system} system",
		introtext = "Hello, my name is {name}. We have had some blips near {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system. I'm empowered to pay you {cash} for a sensor sweep of the area.",
		whysomuchtext = "It's our standard fee for such services.",
		successmsg = "Thank you for transmitting the information. The agreed fee has been transfered to your account",
		failuremsg = "Because of your breach of contract, I had to dispatch another vessel. Your unreliability will be noted!",
		urgency = 0,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "Scout needed in the {system} system.",
		introtext = "Hello. I'm {name}. I'm willing to pay {cash} for a sensor sweep of the area near {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "If you want I can offer you less...?",
		successmsg = "I've received your data, you should be getting the money any instant now.",
		failuremsg = "I might as well have sent a snail there. I'm not paying for outdated information!",
		urgency = 0.1,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "URGENT. Data about the {system} system needed!",
		introtext = "My name is {name}, I'm a journalist currently writing a story about the {system} ({sectorx}, {sectory}, {sectorz}) system. There's some hints I need to verify in the vicinity of {systembody}. I'm willing to pay {cash} to anyone who can get me data about the area before my deadline runs up.",
		whysomuchtext = "I'm a renowned journalist, not one of those wannabes writing for handout newspapers. I know that accurate information comes at a price.",
		successmsg = "Thanks a lot, that's just the source material I needed for my article. Your money is on the way!",
		failuremsg = "I could not finish the article on time because I did not have the data to back up some points. No money for me, no money for you either.",
		urgency = 0.6,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "RECON. in the {system} system. {cash} to an experienced pilot.",
		introtext = "Hello. I'm {name}, information is my business. I'm willing to pay {cash} for a sensor sweep of {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "I know there's something going on there, but I don't know what. In my profession, that's not good.",
		successmsg = "Your timely and discrete service is much appreciated. You have been paid in full.",
		failuremsg = "Useless! I will never depend on you again! Needless to say, you will not be paid for this.",
		urgency = 0.4,
		risk = 0.75,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "Help us keep the {system} system orderly!",
		introtext = "Greetings. This is Lieutenant {name} from AdAstra security services. We pay {cash} for data about {systembody} in the {system} ({sectorx}, {sectory}, {sectorz}) system.",
		whysomuchtext = "We need to check out some rumors. Usually they are unsubstantial, but sometimes they turn out to be true.",
		successmsg = "Well done, your money is being transfered.",
		failuremsg = "Your ship registration has been noted, we will reject all further applications for work from you.",
		urgency = 0.1,
		risk = 0.1,
		localscout = 0,
})

Translate:AddFlavour('English','Scout', {
		adtext = "{system} administration offices need your help to keep their files up to date!",
		introtext = "Pleased to meet you, I am secretary {name}, {system} administration, and I'm willing to pay {cash} for current data about {systembody}. No rush, we simply need to keep our files up to date.",
		whysomuchtext = "This is a government job. It's not OUR money.",
		successmsg = "Thank you for helping us keeping our information current. Your pay is transfered as we speak.",
		failuremsg = "Am I supposed to update our outdated information with other outdated information? This is unacceptable I'm afraid.",
		urgency = 0.1,
		risk = 0,
		localscout = 1,
})

Translate:AddFlavour('English','Scout', {
		adtext = "{system} police needs your help to keep order!",
		introtext = "I am captain {name}, {system} police. We urgently need information about {systembody}. I'll pay you {cash} credits if I get the information in a reasonable time.",
		whysomuchtext = "We can't allow such dangers to our citizens in our neighbourhood. We need to know what's going on there, and we need to know it soon!",
		successmsg = "Your prompt report is appreciated. Your money has been transfered.",
		failuremsg = "I needed that information some time ago! I refuse to pay you.",
		urgency = 0.6,
		risk = 0.4,
		localscout = 1,
})

Translate:Add({ English = {
  ["I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared."] = "I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared.",
  ["This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area."] = "This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area.",
  ["A ship has vanished in the area. I suspect pirate activity."] = "A ship has vanished in the area. I suspect pirate activity.",
  ["Several ships have been lost in the area, including my last scout. I really need to know what's going on."] = "Several ships have been lost in the area, including my last scout. I really need to know what's going on.",
  ["I have reports from passing ships that confirm pirate attacks. What I need to know is how strong they are. You are certain to meet hostiles."] = "I have reports from passing ships that confirm pirate attacks. What I need to know is how strong they are. You are certain to meet hostiles.",
  ["I need the information by "] = "I need the information by ",
  ["Recon"] = "Recon",
  ["Excellent. I will await your report."] = "Excellent. I will await your report.",
  ["Why so much money?"] = "Why so much money?",
  ["When do you need the data?"] = "When do you need the data?",
  ["Will I be in any danger?"] = "Will I be in any danger?",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["Ok, agreed."] = "Ok, agreed.",
  ["Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes"] = "Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes",
  ["Sensor sweep interrupted, too far from target!"] = "sensor sweep interrupted, too far from target!",
  ["Sensor sweep complete, data stored."] = "Sensor sweep complete, data stored.",
  ["computer"] = "computer",
 PIRATE_TAUNTS = {
	"Looky here, it's payday!",
	"All your ship are belong to us!",
	"You won't get back with that sensor data!",
  },
}, })

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','Scout', {
		adtext = "Poszukuję Pilota dla rekonesansu w systemie {system}",
		introtext = "Witam, nazywam się {name}. Odbieramy jakieś sygnały niedaleko {systembody} w systemie {system} ({sectorx}, {sectory}, {sectorz}). Upoważniono mnie do wypłacenia {cash} za skanowanie sensorem w tym obszarze.",
		whysomuchtext = "To standardowa zapłata za taką usługę.",
		successmsg = "Dziękuję ci za transmisję informacji. Uzgodniona kwota będzie przelana na twoje konto",
		failuremsg = "Z twojej winy zrywamy kontrakt, musieliśmy wysłać kolejny statek. Odnotujemy twoją niekompetencję!",
		urgency = 0,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "Potrzebny zwiadowca do systemu {system}.",
		introtext = "Witam. Jestem {name}. Zapłacę ci {cash} za skanowanie sensorem niedaleko {systembody} w systemie {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Jeśli chcesz to mogę zaoferować ci mniej...?",
		successmsg = "Odebraliśmy twoje dane, za chwilę otrzymasz pieniądze.",
		failuremsg = "Równie dobrze można było wysłać tam ślimaka. Nie zapłacę za przestarzałe informacje!",
		urgency = 0.1,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "PILNE. Potrzebne dane o systemie {system}!",
		introtext = "Nazywam się {name}, jestem dziennikarzem i właśnie piszę materiał o systemie {system} ({sectorx}, {sectory}, {sectorz}). Potrzebuję zweryfikować niektóre wątki dotyczące okolic {systembody}. Mogę zapłacić {cash} każdemu, kto dostarczy mi te dane jeszcze przed ostatecznym terminem druku.",
		whysomuchtext = "Jestem znanym dziennikarzem, ale nie z tych pragnących pisać dla brukowców. Bo wiem że prawdziwe informacje są w cenie.",
		successmsg = "Wielkie dzięki, właśnie takiego materiału źródłowego szukam do mojego artykułu. Twoje pieniądze są już w drodze!",
		failuremsg = "Artykuł nie został ukończony na czas ponieważ brakło podparcia dla niektórych z kwestii. Brak pieniędzy dla mnie, brak też pieniędzy dla ciebie.",
		urgency = 0.6,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "REKONESANS. W systemie {system} . {cash} dla doświadczonego pilota.",
		introtext = "Witam. Jestem {name}, dla mnie informacja to pieniądz. Mogę zapłacić {cash} za skanowanie sensorem nad {systembody} w systemie {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Wiem że coś tam się dzieje, ale nie wiem dokładnie co. W mojej profesji, to niedopuszczalne.",
		successmsg = "Twoja punktualna i dyskretna usługa zostanie doceniona. Wypłacam całą należność.",
		failuremsg = "Nieprzydatne! Więcej liczyć na ciebie nie będę! Nie ma co więcej mówić, nie zapłacę ci za to.",
		urgency = 0.4,
		risk = 0.75,
		localscout = 0,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "Pomóż nam utrzymać porządek w systemie {system}!",
		introtext = "Pozdrawiam. Tu porucznik {name} ze służby ochrony AdAstra. Zapłacimy {cash} za dane o {systembody} w systemie {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Musimy sprawdzić pewne pogłoski. Zwykle okazują się nieistotne, ale czasem mogą być też prawdziwe.",
		successmsg = "Świetnie, twoje wynagrodzenie będzie przelane.",
		failuremsg = "Twój numer rejestracyjny zostaje zapisany, będziemy odrzucać twoje dalsze podania o pracę.",
		urgency = 0.1,
		risk = 0.1,
		localscout = 0,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "Biura administracji w {system} potrzebują twojej pomocy przy aktualizacji dokumentacji!",
		introtext = "Miło cię widzieć, jestem {name}, sekretarz w administracji {system}. Chcę zapłacić {cash} za aktualne dane o {systembody}. Bez pośpiechu, po prostu musimy posiadać aktualną dokumentację.",
		whysomuchtext = "To jest praca rządowa. To nie są NASZE pieniądze.",
		successmsg = "Dziękuję ci za pomoc w uzyskaniu aktualnych informacji. Twoja należność została przelana zgodnie z obietnicą.",
		failuremsg = "Mam uaktualniać nasze przedawnione informacje inną przedawnioną informacją? Obawiam się że jest to niedopuszczalne.",
		urgency = 0.1,
		risk = 0,
		localscout = 1,
})

Translate:AddFlavour('Polski','Scout', {
		adtext = "Policja w {system} potrzebuje twojej pomocy w utrzymaniu porządku!",
		introtext = "Jestem kapitan {name}, policja {system} . Potrzebujemy pilnie informacji o {systembody}. Zapłacę ci {cash} jeśli uzyskam informacje w rozsądnym czasie.",
		whysomuchtext = "Nie pozwalamy by w sąsiedztwie powstawało zagrożenie dla naszych obywateli . Chcemy wiedzieć co tam się dzieje i musimy wiedzieć to wkrótce!",
		successmsg = "Twoje szybkie sprawozdanie jest cenne. Twoje pieniądze zostaną przelane.",
		failuremsg = "Ta informacja była potrzebna dawno temu! Odmawiam zapłaty.",
		urgency = 0.6,
		risk = 0.4,
		localscout = 1,
})

Translate:Add({ Polski = {
  ["I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared."] = "Podejrzewam tam jakąś niezarejestrowaną działalność. Prawdopodobnie nic wielkiego, ale lepiej być przygotowanym.",
  ["This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area."] = "To tylko rutynowa kontrola. Jeżeli istniałoby faktycznie ryzyko, sądzę że usłyszelibyśmy o atakach w tym rejonie.",
  ["A ship has vanished in the area. I suspect pirate activity."] = "W rejonie przepadł statek. Podejrzewam działalność pirata.",
  ["Several ships have been lost in the area, including my last scout. I really need to know what's going on."] = "Kilka statków zaginęło w tym rejonie, włącznie z moim ostatnim zwiadowcą. Naprawdę muszę wiedzieć co się dzieje.",
  ["I have reports from passing ships that confirm pirate attacks. What I need to know is how strong they are. You are certain to meet hostiles."] = "Mam sprawozdania od przelatujących tam statków co potwierdzają ataki pirata. Stąd wiem jak bardzo jest silny. Możesz być pewny zagrożenia.",
  ["I need the information by "] = "Potrzebuję informacji do ",
  ["Recon"] = "Rekonesans",
  ["Excellent. I will await your report."] = "Wspaniale. Oczekuję na twój raport.",
  ["Why so much money?"] = "Dlaczego tyle pieniędzy?",
  ["When do you need the data?"] = "Do kiedy potrzebujesz tych danych?",
  ["Will I be in any danger?"] = "Jest jakieś zagrożenie?",
  ["Could you repeat the original request?"] = "Możesz powtórzyć swoją ofertę?",
  ["Ok, agreed."] = "Zgoda.",
  ["Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes"] = "Dystans osiągnięty, rozpoczęto skanowanie sensorem dalekiego zasięgu. Utrzymaj orbitę przynajmniej 60 minut",
  ["Sensor sweep interrupted, too far from target!"] = "Przerwano skanowanie sensorem, zbyt daleko od celu!",
  ["Sensor sweep complete, data stored."] = "Skanowanie sensorem zakończone, zebrano dane.",
  ["computer"] = "komputer",
 PIRATE_TAUNTS = {
	"Co my tu mamy, to dzień zapłaty!",
	"Cały twój statek należy do nas!",
	"Nie wrócisz z danymi z tego sensora!",
  },
}, })

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','Scout', {
		adtext = "Se busca piloto para reconocimiento en el sistema {system}.",
		introtext = "Hola, me llamo {name}. Hemos detectado señales de radar cerca de {systembody} en el sistema {system} ({sectorx}, {sectory}, {sectorz}). Estoy facultado para pagarle {cash} por un sensor de barrido del area.",
		whysomuchtext = "Es nuestra cuota estandar para un servicio como este.",
		successmsg = "Gracias por la transmisión. La cuota acordada ha sido transferida a su cuenta",
		failuremsg = "Por su incumplimiento de contrato, debo mandar otra nave. Su falta de fiabilidad se tomará en cuenta!",
		urgency = 0,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "Se necesita reconocimiento en el sistema {system}.",
		introtext = "Saludos. Soy {name}. Tengo la intención de pagar {cash} por un barrido del area cercana a {systembody} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Si quieres puedo ofrecer menos...?",
		successmsg = "He recibido los datos, deberías estar recibiendo el dinero a partir de ahora.",
		failuremsg = "Podría haber mandado un caracol a hacer el trabajo. No voy a pagar por información desfasada!",
		urgency = 0.1,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "URGENTE. Necesarios Datos sobre el sistema {system}!",
		introtext = "Mi nombre es {name}, Soy periodista, actualmente estoy escribiendo sobre el sistema {system} ({sectorx}, {sectory}, {sectorz}). Existen algunas pistas que debo verificar en las inmediaciones de {systembody}. Estoy dispuesto a pagar {cash} a alguien que pueda proporcionarme datos referentes al area antes de mi fecha límite.",
		whysomuchtext = "Soy un periodista reconocido, no como esos aspirantes que escriben en folletos. Soy consciente de que la información precisa tiene un precio.",
		successmsg = "Mil gracias, esto es justo el material que necesitaba para mi artículo. Su dinero está en camino!",
		failuremsg = "No he podido terminar el artículo porque no tenía los datos de apoyo. No hay dinero para mi, tampoco para usted.",
		urgency = 0.6,
		risk = 0,
		localscout = 0,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "RECON. En el sistema {system}. {cash} para un piloto con experiencia.",
		introtext = "Hola. Soy {name}, la información es mi negocio. Estoy en disposición de pagar {cash} por un barrido de sensores de {systembody} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Se que algo se cuece por allí, pero no se el qué. En mi profesión, eso no es bueno.",
		successmsg = "Su servicio puntual y discreto es muy apreciado. Se le ha pagado el total.",
		failuremsg = "Inutil! No volveré a depender de usted! No hace falta decir que no se le va a pagar por esto.",
		urgency = 0.4,
		risk = 0.75,
		localscout = 0,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "Ayudanos a mantener el sistema {system} en orden!",
		introtext = "Saludos. Al habla el teniente {name} de Servicios de Seguridad AdAstra. Pagamos {cash} por datos de {systembody} en el sistema {system} ({sectorx}, {sectory}, {sectorz}).",
		whysomuchtext = "Tenemos que revisar algunos rumores. Suelen ser insustanciales, pero en ocasiones son ciertos.",
		successmsg = "Bien hecho, tu dinero esta siendo transferido.",
		failuremsg = "El registro de su nave ha sido anotado, vamos a rechazar todas sus solicitudes de trabajo en un futuro.",
		urgency = 0.1,
		risk = 0.1,
		localscout = 0,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "Oficinas administrativas {system} requiere de su ayuda para mantener sus archivos actualizados!",
		introtext = "Encantado de conocerle, soy el secretario {name}, administración {system}, y estoy dispuesto a pagar {cash} por datos actualizados de {systembody}. Sin prisa, simplemente queremos tener los archivos al día.",
		whysomuchtext = "Es un trabajo gubernamental. No es NUESTRO dinero.",
		successmsg = "Gracias por ayudarnos a mantener nuestros datos al día. Su pago esta siendo transferido como acordamos.",
		failuremsg = "Se supone que debo acualizar nuestra información desfasada con otra información desfasada? Me temo que es inaceptable.",
		urgency = 0.1,
		risk = 0,
		localscout = 1,
})

Translate:AddFlavour('Spanish','Scout', {
		adtext = "La policía de {system} necesita su ayuda para el mantenimiento del orden!",
		introtext = "Soy el capitán {name}, policía de {system}. Urgentemente necesitamos información sobre {systembody}. Le pagaré {cash} créditos si dispongo de la información en un tiempo razonable.",
		whysomuchtext = "No podemos permitir peligros tales para el ciudadano en nuestra propia vecindad. Necesitamos saber que está ocurriendo por allí, y necesitamos saberlo pronto!",
		successmsg = "Su informe puntual es apreciado. Su dinero ha sido transferido.",
		failuremsg = "Necesitaba esa información hace tiempo! Me niego a pagarle.",
		urgency = 0.6,
		risk = 0.4,
		localscout = 1,
})

Translate:Add({ Spanish = {
  ["I suspect that there is some unregistered activity going on. Nothing big probably, but you'd better be prepared.."] = "Sospecho que hay algun tipo de actividad no registrada. probablemente nada grande, pero mejor esté preparado",
  ["This is just a routine check. If there was a substantial risk, I think we would have heard of attacks in the area."] = "Es solo una comprobación de rutina. Si hubiera riesgo sustancial, pienso que habríamos escuchado de ataques en el area.",
  ["A ship has vanished in the area. I suspect pirate activity."] = "A ship has vanished in the area. I suspect pirate activity.",
  ["Several ships have been lost in the area, including my last scout. I really need to know what's going on."] = "Se han perdido varias naves en el area, incluyendo mi ultimo scout. Necesito saber a ciencia cierta qué está sucediendo.",
  ["I have reports from passing ships that confirm pirate attacks.  What I need to know is how strong they are . You are certain to meet hostiles."] = "Tengo informes por parte de naves de paso que confirman ataques piratas. Lo que necesito saber es cómo son de fuertes. De por seguro enfrentarse a hostiles.",
  ["I need the information by "] = "Necesito la información por ",
  ["Recon"] = "Recon",
  ["Excellent. I will await your report."] = "Excelente. Esperaré su informe.",
  ["Why so much money?"] = "Por qué tanto dinero?",
  ["When do you need the data?"] = "Cuándo necesita los datos?",
  ["Will I be in any danger?"] = "Me enfrentaré a algun tipo de peligro?",
  ["Could you repeat the original request?"] = "Podría repetir la petición original?",
  ["Ok, agreed."] = "Ok, de acuerdo.",
  ["Distance reached, starting long range sensor sweep. Maintain orbit for at least 60 minutes"] = "Distancia alcanzada, inicializando sensor de barrido de larga distancia. Mantenga órbita durante al menos 60 minutos",
  ["Sensor sweep interrupted, too far from target!"] = "Sensor de barrido interrumpido, objetico demasiado lejos!",
  ["Sensor sweep complete, data stored."] = "Sensor de barrido completo, datos almacenados.",
  ["computer"] = "Ordenador",
 PIRATE_TAUNTS = {
	"Vaya, vaya, día de paga!",
	"Tu nave nos pertenece!",
	"No vas a volver con esa info de sensor!",
  },
}, })