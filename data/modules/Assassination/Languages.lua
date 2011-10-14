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
  failuremsg = "We found out that {target} is nonetheless operative. This sadness us.",
  failuremsg2 = "{target} was neutralized by someone else.",
})

Translate:Add({ English = {
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}) at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}) at {date}. The ship is {shipname} and has registration id {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "It must be done after {target} leaves {spaceport}. Do not miss this opportunity.",
  ["Assassination"] = "Assassination",
  ["Excellent."] = "Excellent.",
  ["Return here on the completion of the contract and you will be paid."] = "Return here on the completion of the contract and you will be paid.",
  ["Where can I find {target}?"] = "Where can I find {target}?",
  ["Could you repeat the original request?"] = "Could you repeat the original request?",
  ["How soon must it be done?"] = "How soon must it be done?",
  ["How will I be paid?"] = "How will I be paid?",
  ["Ok, agreed."] = "Ok, agreed.",
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
  successmsg = "Cieszą mnie wieści o wysłaniu {target}'s na długie wakacje. Dobra robota, rozpocząłem przelew pieniędzy.",
  failuremsg = "Jestem bardzo rozczarowany że {target} wciąż żyje. Chyba nie muszę mówić, że nie zobzczysz żadnych pieniędzy.",
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
  ["{target} will be leaving {spaceport} in the {system} system ({sectorX}, {sectorY}, {sectorZ}) at {date}. The ship is {shipname} and has registration id {shipregid}."] = "{target} będzie opuszczał {spaceport} w systemie {system} ({sectorX}, {sectorY}, {sectorZ}) o {date}, na statku {shipname} o identyfikatorze {shipregid}.",
  ["It must be done after {target} leaves {spaceport}. Do not miss this opportunity."] = "Zrób to po tym jak {target} opuści {spaceport}. Nie przegap tej okazji.",
  ["Assassination"] = "Zabójstwo",
  ["Excellent."] = "Wspaniale.",
  ["Return here on the completion of the contract and you will be paid."] = "Wróć tu po wypełnieniu kontraktu, otrzymasz zapłatę.",
  ["Where can I find {target}?"] = "Gdzie mogę znaleźć {target}?",
  ["Could you repeat the original request?"] = "Czy możesz powtórzyć swoją ofertę?",
  ["How soon must it be done?"] = "Jak powinienem to zrobić?",
  ["How will I be paid?"] = "Jak będę miał zapłacone?",
  ["Ok, agreed."] = "Zgoda.",
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