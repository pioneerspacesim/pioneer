-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Translate = import("Translate")

  -- adtext - text shown in the bulletin board list
	-- introtext - shown when the advert is selected (and "Could you repeat request?")
	-- successmsg - message sent on successful assassination
	-- failuremsg - message sent on failed assassination
	-- failuremsg2 - message sent on assassination done by someone else

  ---- ENGLISH / ENGLISH ----

Translate:AddFlavour('English','Assassination', {
  adtext = "WANTED: Removal of {target} from the {system} system.",
  introtext = "Hi, I'm {name}. I'll pay you {cash} to get rid of {target}.",
  successmsg = "News of {target}'s long vacation gratefully received. Well done, I have initiated your full payment.",
  failuremsg = "I am most displeased to find that {target} is still alive. Needless to say you will receive no payment.",
  failuremsg2 = "{target}'s removal was not done by you. No payment this time.",
})

Translate:AddFlavour('English','Assassination', {
  adtext = "WANTED: Someone to kill {target} from the {system} system.",
  introtext = "I need {target} taken out of the picture. I'll pay you {cash} to do this.",
  successmsg = "I am most sad to hear of {target}'s demise. You have been paid in full.",
  failuremsg = "I hear that {target} is in good health. This pains me.",
  failuremsg2 = "{target}'s demise was not caused by you, so do not ask for payment.",
})

Translate:AddFlavour('English','Assassination', {
  adtext = "REMOVAL: {target} is no longer wanted in the {system} system.",
  introtext = "I am {name}, and I will pay you {cash} to terminate {target}.",
  successmsg = "You have been paid in full for the completion of that important contract.",
  failuremsg = "It is most regrettable that {target} is still live and well. You will receive no payment as you did not complete your contract.",
  failuremsg2 = "Contract was completed by someone else. Be faster next time!",
})

Translate:AddFlavour('English','Assassination', {
  adtext = "TERMINATION: Someone to eliminate {target}.",
  introtext = "The {target} must be reduced to space dust. I'll award you {cash} to do this.",
  successmsg = "{target} is dead. Here is your award.",
  failuremsg = "You will pay for not eliminating {target}!",
  failuremsg2 = "Are you asking money for job done by someone else? Get lost.",
})

Translate:AddFlavour('English','Assassination', {
  adtext = "RETIREMENT: Someone to retire {target}.",
  introtext = "For {cash} we wish to encourage {target} to stop work permanently.",
  successmsg = "News of {target}'s retirement delightfully obtained. Here is your money.",
  failuremsg = "{target} is still breathing and I'm not giving money to you.",
  failuremsg2 = "Retirement of {target} was done by someone else.",
})

Translate:AddFlavour('English','Assassination', {
  adtext = "BIOGRAPHICAL: Some admirers wish {target} dead.",
  introtext = "We wish {target} to have a fitting career end in the {system} system for {cash}.",
  successmsg = "Message of {target}'s ending career happily acquired. Here is your {cash}.",
  failuremsg = "We found out that {target} is nonetheless operative. This saddens us.",
  failuremsg2 = "{target} was neutralized by someone else.",
})

Translate:Add({ English = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "It must be done after {target} leaves {spaceport}. Do not miss this opportunity.",
  ["Assassination"] = "Assassination",
  ["Excellent."] = "Excellent.",
  ["Return here on the completion of the contract and you will be paid."] = "Return here on the completion of the contract and you will be paid.",
  ["Where can I find {target}?"] = "Where can I find {target}?",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["How soon must it be done?"] = "How soon must it be done?",
  ["How will I be paid?"] = "How will I be paid?",
  ["Ok, agreed."] = "Ok, agreed.",
  ["ly"] = "ly",

  -- Texts for the missions screen   
  ["Target name:"] = "Target name:",
  ["Spaceport:"] = "Spaceport:",
  ["System:"] = "System:",
  ["Ship:"] = "Ship:",
  ["Ship ID:"] = "Ship ID:",
  ["Target will be leaving spaceport at:"] = "Target will be leaving spaceport at:",
  ["Distance:"] = "Distance:",

  TITLE = {
		"Admiral",
		"Ambassador",
		"Brigadier",
		"Cadet",
		"Captain",
		"Cardinal",
		"Colonel",
		"Commandant",
		"Commodore",
		"Corporal",
		"Ensign",
		"General",
		"Judge",
		"Lawyer",
		"Lieutenant",
		"Marshal",
		"Merchant",
		"Officer",
		"Private",
		"Professor",
		"Prosecutor",
		"Provost",
		"Seaman",
		"Senator",
		"Sergeant",
	 },
}, })

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','Assassination', {
  adtext = "POSZUKIWANY: Do usunięcia {target} z systemu {system}.",
  introtext = "Cześć, Jestem {name}. Zapłacę {cash} za pozbycie się {target}.",
  successmsg = "Cieszą mnie wieści o wysłaniu {target}'s na długie wakacje. Dobra robota, rozpoczynam przelew pieniędzy.",
  failuremsg = "Wyrażam wielkie rozczarowanie że {target} wciąż żyje. Chyba nie muszę mówić, że nie zobaczysz żadnych pieniędzy.",
  failuremsg2 = "Nie ty pozbyłeś się {target}'s. Tym razem bez zapłaty.",
})

Translate:AddFlavour('Polski','Assassination', {
  adtext = "POTRZEBNY: Ktoś do zabicia {target} z systemu {system}.",
  introtext = "Szukam kogoś kto wyeliminuje {target}. Zapłacę za to {cash}.",
  successmsg = "Zasmuciły mnie wieści o zgonie {target}'s. Płacę całość umówionej sumy.",
  failuremsg = "Słyszałem że {target} cieszy się dobrym zdrowiem. To mnie zabolało.",
  failuremsg2 = "Śmierć {target}'s nie była twoją zasługą, więc nie pytaj o zapłatę.",
})

Translate:AddFlavour('Polski','Assassination', {
  adtext = "USUNIĘCIE: {target} nie jest już mile widziany w systemie {system}.",
  introtext = "Jestem {name}, i zapłacę {cash} za zlikwidowanie {target}.",
  successmsg = "Całość sumy została wypłacona za dotrzymanie umowy.",
  failuremsg = "Jest godne ubolewania że {target} wciąż żyje i ma się dobrze. Nie otrzymasz żadnej zapłaty za złamanie umowy.",
  failuremsg2 = "Ktoś inny wypełnił zlecenie. Następnym razem bądź szybszy!",
})

Translate:AddFlavour('Polski','Assassination', {
  adtext = "ELIMINACJA: Ktoś do wyeliminowania {target}.",
  introtext = "{target} powinien zamienić się w gwiezdny pył. {cash} nagrody za wykonanie zlecenia.",
  successmsg = "{target} nie żyje. Oto twoja nagroda.",
  failuremsg = "{target} żyje, zapłacisz za niewywiązanie się z umowy!",
  failuremsg2 = "Pytasz o pieniądze za prace wykonaną przez kogoś innego? Spadaj.",
})

Translate:AddFlavour('Polski','Assassination', {
  adtext = "PRZYMUSOWA EMERYTURA: Chcę wysłać {target} na przyśpieszoną emeryturę.",
  introtext = "{cash} za przekonanie {target} do odejścia z pracy.",
  successmsg = "Miło słyszeć o odejściu {target}. Oto twoje pieniądze.",
  failuremsg = "{target} wciąż oddycha i na pewno za to nie zapłacę.",
  failuremsg2 = "Nie ty przyczyniłeś się do usunięcia {target}.",
})

Translate:AddFlavour('Polski','Assassination', {
  adtext = "KONIEC BIOGRAFII: Pewni ludzie życzą {target} śmierci.",
  introtext = "Chcemy by {target} zakończył karierę w systemie {system}.Płacimy {cash}",
  successmsg = "Wspaniałe wieści o zakończeniu kariery {target}. Przelewamy obiecane {cash}.",
  failuremsg = "Słyszeliśmy że {target} kontynuuje swoją działalność. Nie ucieszyło nas to.",
  failuremsg2 = "{target} został unieszkodliwiony przez kogoś innego.",
})

Translate:Add({ Polski = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} będzie opuszczał {spaceport} w systemie {system} ({sectorX}, {sectorY}, {sectorZ}), oddalonym o {dist} lś, punktualnie o {date}. Na statku {shipname} o identyfikatorze {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Zrób to po tym jak {target} opuści {spaceport}. Nie przegap tej okazji.",
  ["Assassination"] = "Zabójstwo",
  ["Excellent."] = "Wspaniale.",
  ["Return here on the completion of the contract and you will be paid."] = "Wróć tu po wypełnieniu kontraktu, otrzymasz zapłatę.",
  ["Where can I find {target}?"] = "Gdzie mogę znaleźć {target}?",
  ["Could you repeat the original request?"] = "Czy możesz powtórzyć swoją ofertę?",
  ["How soon must it be done?"] = "Jak powinienem to zrobić?",
  ["How will I be paid?"] = "Jak będę miał zapłacone?",
  ["Ok, agreed."] = "Zgoda.",
  ["ly"] = "lś",

  -- Texts for the missions screen  
  ["Target name:"] = "Cel:",
  ["Spaceport:"] = "Port kosmiczny:",
  ["System:"] = "System:",
  ["Ship:"] = "Statek:",
  ["Ship ID:"] = "Identyfikator:",
  ["Target will be leaving spaceport at:"] = "Cel będzie opuszczał port kosmiczny o:",
  ["Distance:"] = "Dystans:",

  TITLE = {
		"admirał",
		"ambasador",
		"brygadier",
		"kadet",
		"kapitan",
		"kardynał",
		"pułkownik",
		"komendant",
		"komandor",
		"kapral",
		"chorąży",
		"generał",
		"sędzia",
		"prawnik",
		"porucznik",
		"marszałek",
		"kupiec",
		"oficer",
		"szeregowiec",
		"profesor",
		"prokurator",
		"rektor",
		"marynarz",
		"senator",
		"sierżant",
	 },
}, })

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "SE BUSCA: Eliminación de {target} en el sistema {system}.",
  introtext = "Hola, Soy {name}. Le pagaré {cash} para que se deshaga de {target}.",
  successmsg = "Las gratas noticias sobre las largas vacaciones de {target} han sido recibidas. Bien hecho, he iniciado su pago.",
  failuremsg = "Estoy muy disgustado al enterarme de que {target} aún sigue respirando. No es necesario decir que no recibirá pago alguno.",
  failuremsg2 = "No ha sido usted quien ha eliminado a {target}. Esta vez no hay recompensa.",
})

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "SE BUSCA: Alguien para matar a {target} en el sistema {system}.",
  introtext = "Necesito a {target} borrado del mapa. Se le pagará {cash} por el trabajo.",
  successmsg = "Me entristecen las noticias sobre el fallecimiento de {target}. Se le ha efectuado el pago completo.",
  failuremsg = "He escuchado que {target} goza de buena salud. Eso me apena.",
  failuremsg2 = "Usted no ha tenido nada que ver con el fallecimiento de {target}, por lo tanto no pregunte por su recompensa.",
})

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "ELIMINACION: {target} ya no es grato en el sistema {system}.",
  introtext = "Soy {name}, y le pagaré {cash} por acabar con {target}.",
  successmsg = "Se le ha pagado al completo por cumplir con éxito este importante contrato.",
  failuremsg = "Es muy lamentable que {target} aun está vivito y coleando. No recibirá pago alguno puesto que el contrato no ha sido cumplido.",
  failuremsg2 = "El contrato ha sido cumplido por otra persona. La próxima vez sea mas rápido!",
})

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "EXTERMINACION: Se necesita a alguien para eliminar a {target}.",
  introtext = "{target} debe ser reducido a polvo. Le recompensaré con {cash} por el trabajo.",
  successmsg = "{target} está muerto. He aquí su premio.",
  failuremsg = "Las pagará por no eliminar a {target}!",
  failuremsg2 = "Estas pidiendo dinero por un trabajo que ha hecho otro? Piérdete.",
})

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "RETIRO FORZOSO: Se busca a alguien que retire a {target}.",
  introtext = "Por {cash} nos gustaría forzar a {target} a que cese sus actividades de forma permanante.",
  successmsg = "Las noticias sobre el retiro forzoso de {target} se han recibido con placer. Aquí está su dinero.",
  failuremsg = "{target} aún respira y no habrá recompensa alguna.",
  failuremsg2 = "El retiro de {target} fue ejecutado por otra persona.",
})

Translate:AddFlavour('Spanish','Assassination', {
  adtext = "BIOGRAFICO: Ciertos admiradores quieren a {target} fiambre.",
  introtext = "Nos gustaría terminar con la carrera de {target} en el sistema {system} por {cash}.",
  successmsg = "El mensaje sobre el fin de la carrera de {target} se ha recibido felizmente. He aquí sus {cash}.",
  failuremsg = "Nos han llegado noticias de que {target} está totalmente operativo. Y esto nos entristece.",
  failuremsg2 = "{target} fue neutralizado por otro.",
})

Translate:Add({ Spanish = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} partirá de {spaceport} en el sistema {system} ({sectorX}, {sectorY}, {sectorZ}) el {date}. La nave es una {shipname} y tiene el número de registro {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Se debe actuar cuando {target} parta de {spaceport}. No pierda esta oportunidad.",
  ["Assassination"] = "Asesinato",
  ["Excellent."] = "Excelente.",
  ["Return here on the completion of the contract and you will be paid."] = "Regrese aquí cuando el contrato se cumpla y se le efectuará el pago.",
  ["Where can I find {target}?"] = "Dónde puedo encontrar a {target}?",
  ["Could you repeat the original request?"] = "Podría repetir la petición?",
  ["How soon must it be done?"] = "De cuanto tiempo dispongo?",
  ["How will I be paid?"] = "Cómo será efectuado el pago?",
  ["Ok, agreed."] = "De acuerdo.",
  TITLE = {
		"Almirante",
		"Embajador",
		"Brigadier",
		"Cadete",
		"Patrón",
		"Cardenal",
		"Coronel",
		"Capitán",
		"Comodoro",
		"Cabo",
		"Alferez",
		"General",
		"Juez",
		"Letrado",
		"Teniente",
		"Mariscal",
		"Mercader",
		"Oficial",
		"Soldado raso",
		"Profesor",
		"Fiscal",
		"Director",
		"Marinero",
		"Senador",
		"Sargento",
	 },
}, })

  ---- HUNGARIAN / MAGYAR ----

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "KERESEK: Olyan személyt, aki eltávolítaná {target} személyét {system} rendszerből.",
  introtext = "Helló, a nevem {name}. Fizetnék {cash} összeget, hogy megszabaduljak tőle: {target}.",
  successmsg = "Úgy hallom,{target} hosszú szabadságra ment. Nagyszerű, máris elküldöm a fizetséget.",
  failuremsg = "Nagyon elégedetlen vagyok azzal, hogy {target} még mindig életben van. Mondanom sem kell, hogy semmit nem kapsz.",
  failuremsg2 = "{target} hajóját más valaki lőtte szét. Így neked nem jár fizetség sem.",
})

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "KERESEK: Olyat, aki megölné {target} nem kívánatos személyt a(z) {system} rendszerben.",
  introtext = "Szeretném, ha {target} eltűnne örökre. Fizetek {cash} összeget, ha megteszed nekem.",
  successmsg = "Sajnálattal hallottam hírét {target} halálának. Máris megkapod a fizetséged.",
  failuremsg = "Úgy hallottam, hogy {target} jó egészségnek örvend. Ez sajnálattal tölt el.",
  failuremsg2 = "{target} halálát nem te okoztad, így ne is kérd a fizetséget érte.",
})

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "ELTÁVOLÍTÁS: {target} eltávolítása {system} rendszerből.",
  introtext = "A nevem {name}, és örömmel fizetek {cash} kreditet {target} megsemmisítéséért.",
  successmsg = "A fontos megbízás sikeres teljesítéséért megkapod teljes fizetséged.",
  failuremsg = "Nagyon sajnálatos, hogy {target} még mindig él, és jól van. Nem kapsz fizetséget, ha egyszer nem végezted el a feladatot.",
  failuremsg2 = "A megbízást más teljesítette. Legközelebb légy gyorsabb!",
})

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "MEGSEMMISÍTÉS: Keresek valakit, aki eltenné láb alól: {target}.",
  introtext = "A célpontnak: {target} űrhamuvá kell válnia. Ha megteszed, {cash} összeg üti a markod.",
  successmsg = "{target} meghalt. Itt a jutalmad.",
  failuremsg = "Még megfizetsz azért, hogy nem lőtted szét {target} hajóját!",
  failuremsg2 = "Azért kérsz pénzt, amit nem is te végeztél el? Na takarodj innen.",
})

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "NYUGDÍJAZÁS: Valakinek nyugdíjazni kellene {target} személyét.",
  introtext = "{cash} összegért cserébe szeretnénk, ha {target} végleg abbahagyná működését.",
  successmsg = "{target} visszavonulásának híre örömmel tölt el. Itt van a pénzed.",
  failuremsg = "{target} még mindig él, így nem adok semmi pénzt.",
  failuremsg2 = "{target} nyugdíjazását más intézte el.",
})

Translate:AddFlavour('Magyar','Assassination', {
  adtext = "BIOGRÁFIA: Néhány csodálója szeretné halottan látni {target} személyét.",
  introtext = "Szeretnénk, ha {target} karrierjének vége lenne {system} rendszerben. Fizetünk {cash} kreditet, ha megteszed.",
  successmsg = "{target} karrierjének vége. Itt a pénzed: {cash}.",
  failuremsg = "Megtudtuk, hogy {target} még mindig munkálkodik. Ez szomorúan halljuk.",
  failuremsg2 = "{target} karrierváltozását valaki más intézte.",
})

Translate:Add({ Magyar = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} {date} időpontban hagyja el a(z) {spaceport} dokkját a(z) {system} rendszerben ({sectorX}, {sectorY}, {sectorZ}). \n\nA hajó típusa: {shipname}, regisztrációs száma pedig {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Akkor kell elintézni, miután {target} elhagyta {spaceport} kikötőjét. Ne halaszd el a lehetőséget.",
  ["Assassination"] = "Orgyilkosság",
  ["Excellent."] = "Kitünő.",
  ["Return here on the completion of the contract and you will be paid."] = "Térj vissza ide a megbízás teljesítése után, hogy megkapd a fizetséged.",
  ["Where can I find {target}?"] = "Hol találhatom meg {target} hajóját?",
  ["Could you repeat the original request?"] = "Megismételnéd az eredeti feladatot?",
  ["How soon must it be done?"] = "Milyen hamar kell elvégezni?",
  ["How will I be paid?"] = "Hogy leszek megfizetve?",
  ["Ok, agreed."] = "Rendben, elfogadom.",
  TITLE = {
		"Admirális",
		"Nagykövet",
		"Dandártábornok",
		"Kadét",
		"Százados",
		"Bíboros",
		"Ezredes",
		"Parancsnok",
		"Tengernagy",
		"Tizedes",
		"Zászlós",
		"Tábornok",
		"Bíró",
		"Ügyvéd",
		"Hadnagy",
		"Marsall",
		"Kereskedő",
		"Tisztviselő",
		"Közlegény",
		"Professzor",
		"Ügyész",
		"Felügyelő",
		"Matróz",
		"Szenátor",
		"Őrmester",
	 },
}, })

  ---- RUSSIAN / РУССКИЙ ----

Translate:AddFlavour('Russian','Assassination', {
  adtext = "РАЗЫСКИВАЮ: человека для решения проблемы в системе {system}. Моя проблема - {target}.",
  introtext = "Привет! - меня зовут {name}. Я заплачу тебе {cash}, если {target} будет устранен.",
  successmsg = "Похоже {target} отправился на вечные каникулы. Отлично, я перечислю деньги на ваш счёт.",
  failuremsg = "Похоже {target} всё ещё дышит! Это не то, что могло бы меня успокоить.",
  failuremsg2 = "Я уже знаю, что {target} мёртв. Но не от вашей руки. Контракт разорван.",
})

Translate:AddFlavour('Russian','Assassination', {
  adtext = "ПРЕДЛОЖЕНИЕ: {target} должен навсегда исчезнуть из моей жизни в системе {system}.",
  introtext = "Мне нужно, чтоб {target} покинул этот мир. Я заплачу {cash} наличными.",
  successmsg = "Мне было очень грустно узнать, что {target} мёртв. Деньги уже перечислены на ваш счёт.",
  failuremsg = "Я слышал, что {target} находится в добром здравии. Это весьма неприятно.",
  failuremsg2 = "Я знаю, что {target} скончался без вашей помощи, так что не спрашивайте про деньги.",
})

Translate:AddFlavour('Russian','Assassination', {
  adtext = "УСТРАНЕНИЕ: {target} был недавно замечен в системе {system}.",
  introtext = "Я {name} заплачу вам {cash}, если {target} будет устранен.",
  successmsg = "Завершенный контракт оплачен в полном объеме. Удачи!.",
  failuremsg = "Весьма прискорбно, что {target} всё ещё жив и здоров. Вы не получите денег за незавершенный контракт!",
  failuremsg2 = "Контракт выполнили без вас. В следующий раз поторапливайся!",
})

Translate:AddFlavour('Russian','Assassination', {
  adtext = "ЗАДАНИЕ: {target} должен быть устранен.",
  introtext = "В звездную пыль должен быть превращен {target}. За это я награжу вас {cash}.",
  successmsg = "Всё прошло хорошо - {target} мёртв. Вот ваша награда.",
  failuremsg = "Вам придется ответить за то, что {target} жив!",
  failuremsg2 = "Просите деньги за чужую работу? Проваливайте!",
})

Translate:AddFlavour('Russian','Assassination', {
  adtext = "НА СВАЛКУ истории должен отправиться {target}",
  introtext = "Мы можем предложить {cash}, чтобы {target} прекратил свою деятельность.",
  successmsg = "Мы узнали, что {target} сдох - это восхитительно! Вот ваши деньги.",
  failuremsg = "Я не дам вам ни монетки - {target} ещё не в могиле!",
  failuremsg2 = "Похоже, что {target} уже в могиле без вашей помощи. Платить вам не за что.",
})

Translate:AddFlavour('Russian','Assassination', {
  adtext = "КОНТРАКТ: требуется человек для деликатной работы. Цель - {target}.",
  introtext = "Предлагаем вам сделку на {cash}.\n Цель контракта - {target}.\n Есть люди, которые считают, что пора поставить точку в биографии этой личности.\n Если наше предложение заинтересовало вас, подписывайте контракт и отправляйтесь в систему {system}.",
  successmsg = "Мы получили подтверждение ликвидации {target}. Призовая сумма в {cash} переведена на ваш счёт.",
  failuremsg = "Мы знаем, что {target} всё ещё в живых. Мы разочарованы.",
  failuremsg2 = "Сожалею, но {target} ликвидирован без вашего участия! Прощайте.",
})

Translate:Add({ Russian = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "Ваша цель - {target} - покинет станцию {spaceport} в системе {system} ({sectorX}, {sectorY}, {sectorZ}), расстояние {dist}, ровно в {date} года.\n Корабль {shipname} имеет регистрационный номер {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Вы должны сделать это когда {target} покинет {spaceport}. Не упустите момент.",
  ["Assassination"] = "Ликвидация",
  ["Excellent."] = "Отлично.",
  ["Return here on the completion of the contract and you will be paid."] = "Вы сможете получить деньги на этой станции после выполнения контракта.",
  ["Where can I find {target}?"] = "Где находится цель?",
  ["Could you repeat the original request?"] = "Повторите ваше задание, пожалуйста.",
  ["How soon must it be done?"] = "Когда я должен выполнить задание?",
  ["How will I be paid?"] = "Как я получу оплату за работу?",
  ["Ok, agreed."] = "Согласен, подписываю контракт.",
  ["ly"] = "св.лет",

  -- Texts for the missions screen   
  ["Target name:"] = "Цель:",
  ["Spaceport:"] = "Станция нахождения:",
  ["System:"] = "Система нахождения:",
  ["Ship:"] = "Тип корабля:",
  ["Ship ID:"] = "Номер корабля:",
  ["Target will be leaving spaceport at:"] = "Цель покинет станцию:",
  ["Distance:"] = "Расстояние:",

  TITLE = {
		"адмирал",
		"посол",
		"бригадир",
		"кадет",
		"капитан",
		"кардинал",
		"полковник",
		"командир",
		"командор",
		"капрал",
		"энсин",
		"генерал",
		"судья",
		"адвокат",
		"лейтенант",
		"маршал",
		"торговец",
		"офицер",
		"рядовой",
		"профессор",
		"прокурор",
		"ректор",
		"матрос",
		"сенатор",
		"сержант",
	 },
}, })

  ---- GERMAN / DEUTSCH ----

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "GESUCHT: Entfernung von {target} aus dem {system} System.",
  introtext = "Hi, ich bin {name}. Ich werde dich mit {cash} belohnen, wenn du {target} aus dem Weg schaffst.",
  successmsg = "Ich habe die Neuigkeiten über {target}s langen \"Urlaub\" empfangen. Gut gemacht, du wirst deinen Lohn erhalten.",
  failuremsg = "Ich bin sehr enttäuscht, dass {target} noch lebt. Den Lohn kannst du dir abschreiben.",
  failuremsg2 = "{target} wurde nicht von dir erledigt. Kein Lohn, vielleicht klappt es ja nächstes mal.",
})

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "GESUCHT: Jemand der {target} aus dem {system} System tötet.",
  introtext = "Ich will, dass {target} von der Bildfläche verschwindet. Wenn du es schaffst, erhälst du {cash}.",
  successmsg = "Mit Trauer hörte ich von {target}s Ableben. Du erhälst deine volle Bezahlung.",
  failuremsg = "Ich höre, dass {target} in bester Verfassung ist. This pains me.",
  failuremsg2 = "{target}s Ableben entstand nicht durch deine Hand, also frage nicht nach einer Bezahlung.",
})

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "ENTFERNUNG: {target} ist im {system} system nicht mehr erwünscht.",
  introtext = "Ich bin {name}, und ich werde dir {cash} zahlen, wenn du {target} eliminierst.",
  successmsg = "Du wurdest für den erfolgreichen Abschluss dieses wichtigen Auftrages belohnt.",
  failuremsg = "Es ist sehr bedauerlich, dass {target} noch gesund und munter ist. Du hast deinen Auftrag nicht erfüllt, du wirst keinen Lohn erhalten.",
  failuremsg2 = "Die Aufgabe wurde von jemand anderem erledigt. Sei nächstes mal schneller!",
})

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "ELIMINIERUNG: Jemanden, der {target} erledigt.",
  introtext = "{target} muss zu Sternenstaub werden. Ich wede dich mit {cash} belohnen, wenn du es tust.",
  successmsg = "{target} ist tot. Hier ist deine Belohnung",
  failuremsg = "Du wirst es noch bedauern, {target} nicht eleminiert zu haben!",
  failuremsg2 = "Du willst Geld für den Job, den jemand anderes gemacht hat? Zieh Leine.",
})

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "RUHESTAND: Jemand soll {target} in den Ruhestand schicken.",
  introtext = "Für {cash} soll {target} ermutigt werden, seine Arbeit einzustellen und in den Ruhestand zu gehen.",
  successmsg = "Neuigkeiten von {target}s Rückzug. Hier ist dein Geld.",
  failuremsg = "{target} atmet noch und ich werde dir kein Geld geben.",
  failuremsg2 = "Jemand anderes hat {target} in den Ruhestand geschickt.",
})

Translate:AddFlavour('Deutsch','Assassination', {
  adtext = "BIOGRAFISCH: Einige Verehrer wollen {target} tot sehen.",
  introtext = "Wir wollen einende von {target}s Karriere im {system} System. Wir sind bereit, {cash} zu zahlen.",
  successmsg = "Die Nachricht von {target}s Karriereende hat uns erreicht. Hier sind deine {cash}.",
  failuremsg = "Wir haben herausgefunden, dass {target} immer noch tätig ist. Wir bedauern das.",
  failuremsg2 = "{target} wurde von jemand Anderem neutralisiert.",
})

Translate:Add({ Deutsch = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} wird {spaceport} im {system} system ({sectorX}, {sectorY}, {sectorZ}) am Datum {date} verlassen. Das Schiff heißt {shipname} und hat die Registrations-ID {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Es muss passieren, nachdem {target}  {spaceport} verlässt. Lass dir diese Chance nicht entgehen.",
  ["Assassination"] = "Attentat",
  ["Excellent."] = "Wunderbar.",
  ["Return here on the completion of the contract and you will be paid."] = "Komme hierhin zurück, um den Vertrag abzuschließen und deine Belohnung zu bekommen.",
  ["Where can I find {target}?"] = "Wo kann ich {target} finden?",
  ["Could you repeat the original request?"] = "Könntest du die Frage noch einmal wiederholen?",
  ["How soon must it be done?"] = "Bis wann muss es passieren?",
  ["How will I be paid?"] = "Wie werde ich Bezahlt?",
  ["Ok, agreed."] = "Okay, ich mache das..",

  -- Texts for the missions screen   
  ["Target name:"] = "Zielname:",
  ["Spaceport:"] = "Raumhafen:",
  ["System:"] = "System:",
  ["Ship:"] = "Schiff:",
  ["Ship ID:"] = "Schiff-ID:",
  ["Target will be leaving spaceport at:"] = "Ziel wird Raumhafen verlassen um:",
  ["Distance:"] = "Distanz:",

  TITLE = {
		"Admiral",
		"Botschafter",
		"Brigadier",
		"Kadett",
		"Kapitän",
		"Kardinal",
		"Oberst",
		"Kommandant",
		"Kommodore",
		"Unteroffizier",
		"Leutnant",
		"General",
		"Richter",
		"Anwalt",
		"Leutnant",
		"Marschall",
		"Händler",
		"Offizier",
		"Gefreiter",
		"Professor",
		"Staatsanwalt",
		"Kanzler",
		"Seemann",
		"Senator",
		"Feldwebel",
	 },
}, })

  ---- CZECH / ČESKY ----

Translate:AddFlavour('Czech','Assassination', {
  adtext = "HLEDANÝ: Odstranit {target} ze systému {system}.",
  introtext = "Zdravím, jsem {name}. Zaplatím ti {cash} když mě zbavíš {target}.",
  successmsg = "Zprávy o {target}'s dlouhé \"dovolené\" už dorazily. Výborně, dostal jsi zaplaceno v plné výši.",
  failuremsg = "Jsem velmi nespokojen(a), že {target} je stále naživu. Je zbytečné dodávat, že žádné peníze nedostaneš.",
  failuremsg2 = "{target} nebyl odstraněn tebou. Tentokrát žádná platba nebude.",
})

Translate:AddFlavour('Czech','Assassination', {
  adtext = "HLEDANÝ: Zabít {target} ze systému {system}.",
  introtext = "Potřebuji aby {target} zmizel ze scény. Zaplatím ti {cash} když to zařídíš.",
  successmsg = "Donesla se mi žalostná zpráva o úmrtí {target}. Dostal jsi zaplaceno v plné výši.",
  failuremsg = "Slyšel(a) jsem, že {target} se těší dobrému zdraví. To mě opravdu bolí.",
  failuremsg2 = "{target} zemřel rukou někoho jiného, tak se neptej na peníze!",
})

Translate:AddFlavour('Czech','Assassination', {
  adtext = "ODSTRANĚNÍ: {target} již není v systému {system} vítán.",
  introtext = "Jsem {name}, zaplatím ti {cash} když eliminuješ {target}.",
  successmsg = "Za vyřízení této důležité dohody jsi dostal zaplaceno v plné výši.",
  failuremsg = "Je velmi politováníhodné, že {target} je pořád naživu. Žádné peníze nedostaneš, protože jsi nesplnil naši dohodu.",
  failuremsg2 = "Úkol byl vyřízen někým jiným. Přístě buď rychlejší!",
})

Translate:AddFlavour('Czech','Assassination', {
  adtext = "ELIMINACE: Kdo odstraní {target}?",
  introtext = "{target} musí být rozprášen(a) na kousíčky. Odměna bude {cash} když to uděláš.",
  successmsg = "{target} je po smrti. Zde je tvá odměna.",
  failuremsg = "Toho budeš litovat, {target} je naživu a ty chceš peníze!?!",
  failuremsg2 = "Ty chceš peníze za práci, kterou vyřídil někdo jiný? Ať už tě nevidím!",
})

Translate:AddFlavour('Czech','Assassination', {
  adtext = "VÝSLUŽBA: Kdo pošle na odpočinek {target}?",
  introtext = "{cash} zaplatíme tomu, kdo povzbudí {target}, aby okamžitě zanechal(a) práce a šel/šla do penze.",
  successmsg = "Obdrželi jsme báječné zprávy o {target}'s odchodu do důchodu. Zde jsou tvé peníze.",
  failuremsg = "{target} pořád dýchá, žádné peníze nedostaneš!",
  failuremsg2 = "{target} poslal na odpočinek někdo jiný!",
})

Translate:AddFlavour('Czech','Assassination', {
  adtext = "ŽIVOTOPISNÝ: Někteří ctitelé si přejí smrt {target}.",
  introtext = "Přejeme si ukončit kariéru {target} v systému {system}, jsme ochotni zaplatit {cash}.",
  successmsg = "Zpráva o konci {target} šťastně dorazila. Zde je tvých {cash}.",
  failuremsg = "Zjistili jsme, že je {target} víceméně stále aktivní. To nás rmoutí.",
  failuremsg2 = "{target} byl neutralizován někým jiným!",
})

Translate:Add({ Czech = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}), distance {dist} ly, at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} bude odlétat z {spaceport} v systému {system} ({sectorX}, {sectorY}, {sectorZ}), vzdálenost {dist} ly, k datu {date}. Loď se jmenuje {shipname} a ma registrační id {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Úkol musí být splněn až po té, co {target} opustí {spaceport}. Nezmeškej tuto příležitost!",
  ["Assassination"] = "Atentát",
  ["Excellent."] = "Výborně.",
  ["Return here on the completion of the contract and you will be paid."] = "Po splnění úkolu se sem vrať a dostaneš zaplaceno.",
  ["Where can I find {target}?"] = "Kde můžu {target} najít?",
  ["Could you repeat the original request?"] = "Můžeš mi zopakovat původní dotaz?",
  ["How soon must it be done?"] = "Do kdy musí být úkol splněn?",
  ["How will I be paid?"] = "Jak dostanu zaplaceno?",
  ["Ok, agreed."] = "OK, souhlasím.",
  ["ly"] = "ly",

  -- Texts for the missions screen   
  ["Target name:"] = "Jméno cíle:",
  ["Spaceport:"] = "Stanice:",
  ["System:"] = "Systém:",
  ["Ship:"] = "Loď:",
  ["Ship ID:"] = "ID lodě:",
  ["Target will be leaving spaceport at:"] = "Cíl opustí stanici v:",
  ["Distance:"] = "Vzdálenost:",

  TITLE = {
	"admirál",
	"velvyslanec",
	"velitel brigády",
	"kadet",
	"kapitán",
	"kardinál",
	"plukovník",
	"velitel",
	"komodor",
	"desátník",
	"podporučík",
	"generál",
	"soudce",
	"právník",
	"poručík",
	"maršál",
	"obchodník",
	"důstojník",
	"vojín",
	"profesor",
	"prokurátor",
	"kancléř",
	"námořník",
	"senátor",
	"seržant",
  },
}, })
