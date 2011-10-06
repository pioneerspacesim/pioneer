-- Flavours:

-- title: Name of company, can contain a {name} for the station's name,
--		or a {proprietor} for the company's owner
-- intro: The initial blurb on the ad.  In additoin to {name} and
--		{proprietor}, {drive} is the player's current hyperdrive,
--		{price} is the price of a 12 month service and {lasttime} inserts
--		the time and company name of the last service.
-- yesplease: The prompt for the player to click on to get a service.
-- response: What the company says when the player clicks yesplease.
-- strength: A numeric value, being a multiplier of the 1 year guarantee
-- cost: A numeric value, being a multiplier of base cost (about 2-10)

Translate:AddFlavour('English','BreakdownServicing', {
	title = "{name} Engine Servicing Company",
	intro = [[Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today, by the officially endorsed {name} Engine Servicing Company.

Engine: {drive}
Service: {price}
Guarantee: 18 months
{lasttime}]],
	yesplease = "Service hyperspace engine",
	response = "Your engine has been serviced.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('English','BreakdownServicing', {
	title = "{proprietor}: Hyperdrive maintenance specialist",
	intro = [[I'm {proprietor}.  I can service your {drive}, guaranteeing at least a year of trouble-free performance.  The cost for this service will be {price}
{lasttime}]],
	yesplease = "Service my drive",
	response = "I have serviced your hyperdrive.",
	strength = 1.2, -- At least a year... hidden bonus!
	baseprice = 4,
})

Translate:AddFlavour('English','BreakdownServicing', {
	title = "{proprietor} & Co HyperMechanics",
	intro = [[Hi there.  We at {proprietor} & Co stake our reputation on our work.

{lasttime}
We can tune your ship's {drive}, ensuring 12 months of trouble-free operation, for the sum of {price}.  I'll be supervising the work myself, so you can be sure that a good job will be done.]],
	yesplease = "Please tune my drive at the quoted price",
	response = "Service complete.  Thanks for your custom.",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('English','BreakdownServicing', {
	title = "SuperFix Maintenance ({name} branch)",
	intro = [[Welcome SuperFix Maintenance.

{lasttime}
Time for your biannual maintenance? Let us SuperFix your hyperdrive!
We can tune your {drive} for just {price}.  There's nobody cheaper!]],
	yesplease = "SuperFix me!",
	response = "Your SuperFix service is complete, with SuperFix guarantee!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('English','BreakdownServicing', {
	title = "Time and Space Engines, Inc.",
	intro = [[Welcome to Time and Space.
		
We specialise in interstellar engines. All maintenance work guaranteed for two years.
{lasttime}
Servicing your {drive} will cost {price}.  Would you like to proceed?]],
	yesplease = "Yes, please proceed",
	response = "We have completed the work on your hyperdrive.",
	strength = 2.1, -- these guys are good.
	baseprice = 10,
})

Translate:AddFlavour('English','BreakdownServicing', {
	title = "{proprietor} Engine Servicing Company",
	intro = [[Avoid the inconvenience of a broken-down hyperspace engine.  Get yours serviced today.

Engine: {drive}
Service: {price}
{lasttime}]],
	yesplease = "Service hyperspace engine",
	response = "Your engine has been serviced.",
	strength = 0.0, -- These guys just reset the jump count.  Shoddy.
	baseprice = 1.8,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Konserwacja Silników w {name}",
	intro = [[Uniknij kłopotów związanych z uszkodzeniem napędu nadprzestrzennego.  Zamów przegląd, w certyfikowanej firmie Konserwacja Silników w {name}.

Napęd: {drive}
Przegląd: {price}
Gwarancja: 18 miesięcy
{lasttime}]],
	yesplease = "Rozpocznij przegląd",
	response = "Silnik przeszedł przegląd.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "{proprietor}: Fachowiec od konserwacji napędów nadprzestrzennych",
	intro = [[Jestem {proprietor}.  Mogę obejrzeć twój {drive}, gwarantuje nie mniej niż rok bezproblemowego działania.  Koszt tej usługi wynosi {price}
{lasttime}]],
	yesplease = "Dokonaj przeglądu",
	response = "Zakończyłem przegląd twojego napędu nadprzestrzennego.",
	strength = 1.2,
	baseprice = 4,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "{proprietor} & Co HyperMechanics",
	intro = [[Cześć.  Jestem z {proprietor} & Co, stawiamy naszą reputacje na to, że będziesz zadowolony z usługi.

{lasttime}
Możemy dostroić twój {drive}, za {price}, zapewniamy 12 miesięcy bezproblemowej pracy.  Zajmę się tym osobiście, więc możesz być pewien usługi na najwyższym poziomie.]],
	yesplease = "Wyreguluj napęd za podaną cenę",
	response = "Zrobione.  Dziękujemy za zaufanie",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = " SuperFix Maintenance (firmowana przez {name})",
	intro = [[Witamy w SuperFix Maintenance.

{lasttime}
Czas na co półroczny przegląd? Pozwól nam obejrzeć twój napęd!
Możemy wyregulować twój {drive} za jedyne {price}.  Nie znajdziesz nikogo tańszego!]],
	yesplease = "Zleć usługę SuperFix!",
	response = "Usługa SuperFix zakończona, z gwarancją SuperFix!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Time and Space Engines, Inc.",
	intro = [[Witamy w Time and Space.
	
Specjalizujemy się w napędach nadprzestrzennych. Dajemy dwuletnią gwarancje na wszystkie prace konserwacyjne.
{lasttime}
Za przegląd {drive} cena wynosi {price}.  Kontynuować?]],
	yesplease = "Tak, proszę",
	response = "Zakończyliśmy prace nad napędem.",
	strength = 2.1,
	baseprice = 10,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Warsztat Pan Józek i {proprietor}",
	intro = [[Uniknij kłopotów związanych z uszkodzeniem napędu nadprzestrzennego.  Zamów przegląd jeszcze dziś.

Napęd: {drive}
Usługa: {price}
{lasttime}]],
	yesplease = "Rozpocznij przegląd napędu",
	response = "Napęd przeszedł przegląd.",
	strength = 0.0,
	baseprice = 1.8,
})

Translate:Add({

English = {
	["I don't have enough money"] = "I don't have enough money",
	["Manufacturer's warranty"] = "Manufacturer's warranty",
	["Your drive was last serviced on {date} by {company}"] = "Your drive was last serviced on {date} by {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Your drive has not been serviced since it was installed on {date}",
	["You do not have a drive to service!"] = "You do not have a drive to service!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "The ship's hyperdrive has been destroyed by a malfunction",
-- /English
},

Polski = {

	["I don't have enough money"] = "Nie mam tyle pieniędzy",
	["Manufacturer's warranty"] = "producenta",
	["Your drive was last serviced on {date} by {company}"] = "Ostatni przegląd wykonany {date} przez {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Twój napęd nie przechodził przeglądu od czasu instalacji - dnia {date}",
	["You do not have a drive to service!"] = "Nie posiadasz hipernapędu!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "Awaria, napęd nadprzestrzenny zniszczony.",

-- /Polski
},

-- /Translate:add
})
