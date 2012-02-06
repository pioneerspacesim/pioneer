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

---- ENGLISH / ENGLISH ----

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

Translate:Add({ English = {
	["I don't have enough money"] = "I don't have enough money",
	["Your drive was last serviced on {date} by {company}"] = "Your drive was last serviced on {date} by {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Your drive has not been serviced since it was installed on {date}",
	["You do not have a drive to service!"] = "You do not have a drive to service!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "The ship's hyperdrive has been destroyed by a malfunction",
}, })

---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Konserwacja Silników {name}",
	intro = [[Uniknij kłopotów związanych z uszkodzeniem napędu nadprzestrzennego.  Zamów przegląd, w certyfikowanej firmie Konserwacja Silników {name}.

Silnik: {drive}
Przegląd: {price}
Gwarancja: 18 miesięcy
{lasttime}]],
	yesplease = "Rozpocznij przegląd",
	response = "Silnik przeszedł przegląd.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Fachowiec od konserwacji napędów nadprzestrzennych {proprietor}",
	intro = [[Jestem {proprietor}.  Mogę obejrzeć twój {drive}, gwarantuje nie mniej niż rok bezproblemowego działania.  Koszt tej usługi wynosi {price}
{lasttime}]],
	yesplease = "Dokonaj przeglądu",
	response = "Zakończyłem przegląd twojego napędu nadprzestrzennego.",
	strength = 1.2,
	baseprice = 4,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "{proprietor} & Co HyperMechanics",
	intro = [[Cześć.  Jestem z {proprietor} & Co, nasza doskonała reputacja świadczy o jakości naszych usług.

{lasttime}
Możemy dostroić twój {drive}, za {price}, zapewniamy 12 miesięcy bezproblemowej pracy.  Zajmę się tym osobiście, więc możesz być pewien usługi na najwyższym poziomie.]],
	yesplease = "Wyreguluj napęd za podaną cenę",
	response = "Zrobione.  Dziękujemy za zaufanie",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "{name} przedstawiciel SuperFix Maintenance.",
	intro = [[Witamy w SuperFix Maintenance.

{lasttime}
Czas na co półroczny przegląd? Pozwól że obejrze twój napęd!
Mogę wyregulować twój {drive} za jedyne {price}.  Nie znajdziesz nikogo tańszego!]],
	yesplease = "Zleć usługę SuperFix!",
	response = "Napęd wyregulowany, z gwarancją SuperFix!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Time and Space Engines, Inc.",
	intro = [[Witamy w Time and Space.
	
Specjalizujemy się w napędach nadprzestrzennych. Dajemy dwuletnią gwarancje na wszystkie prace konserwacyjne.
{lasttime}
Za {drive} cena wynosi {price}.  Kontynuować?]],
	yesplease = "Tak, proszę",
	response = "Zakończyliśmy prace nad napędem.",
	strength = 2.1,
	baseprice = 10,
})

Translate:AddFlavour('Polski','BreakdownServicing', {
	title = "Warsztat Pan Józek i {proprietor}",
	intro = [[Uniknij kłopotów związanych z uszkodzeniem napędu nadprzestrzennego.  Zamów przegląd jeszcze dziś.

Silnik: {drive}
Usługa: {price}
{lasttime}]],
	yesplease = "Rozpocznij przegląd napędu",
	response = "Napęd przeszedł przegląd.",
	strength = 0.0,
	baseprice = 1.8,
})

Translate:Add({ Polski = {
	["I don't have enough money"] = "Nie mam tyle pieniędzy",
	["Your drive was last serviced on {date} by {company}"] = "Ostatni przegląd wykonany {date} przez {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Twój napęd nie przechodził przeglądu od czasu instalacji - dnia {date}",
	["You do not have a drive to service!"] = "Nie posiadasz hipernapędu!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "Awaria, napęd nadprzestrzenny zniszczony.",
}, })

---- GERMAN / DEUTSCH ----

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "{name} Laufwerk Wartungs",
	intro = [[Vermeiden Sie die Unannehmlichkeiten eines kaputten Hyperraumantrieb. Erhalten Sie Ihr gewartet heute durch amtlich beglaubigte {name} Laufwerk Wartungs.

Antrieb: {drive}
Dienst: {price}
Garantie: 18 Monate
{lasttime}]],
	yesplease = "Den Hyperraumantrieb pflegen",
	response = "Ihr Hyperraumantrieb gewartet worden ist.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "{proprietor}: Technikerspezialist des Hyperraumantriebs",
	intro = [[Ich bin {proprietor}.  Ich kann Ihr {drive} pflegen, garantiert für mindestens ein Jahr von störungsfreien Betriebs.  Die Kosten des Dienstes wird {price} sein
{lasttime}]],
	yesplease = "Meinen Hyperraumantrieb pflegen",
	response = "Ich habe Ihren Hyperraumantrieb gepflegt.",
	strength = 1.2, -- At least a year... hidden bonus!
	baseprice = 4,
})

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "{proprietor} & Co Hyper-Mechaniker",
	intro = [[Guten Tag.  Wir bei {proprietor} & Co verbürgen für unsere Arbeit mit unser Ruf.

{lasttime}
Wir können Ihr {drive}, so dass er mindestens noch 12 Monate Betrieb hat, für {price} einstellen.  Ich werde die Überwachung der Arbeit selbst, so können Sie sicher sein, dass eine gute Arbeit geleistet wird.]],
	yesplease = "Bitte stellen sie mein Hyperraumantrieb ein",
	response = "Einstellen fertig.  Danke schön für Ihre Kundschaft.",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "SuperFix Maintenance ({name} Filiale)",
	intro = [[Wilkommen bei SuperFix Maintenance.

{lasttime}
Zeit für Ihre halbjährliche Wartung?  Lassen Sie uns Ihr Hyperraumantrieb SuperFixen!
Wir können Ihr {drive} pflegen für nur {price}.  Es gibt niemanden billiger!]],
	yesplease = "SuperFix me!",
	response = "Ihr SuperFix Pflege ist fertig, mit SuperFix Garantie!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "Zeit und Raum Antriebe, Inc.",
	intro = [[Wilkommen nach Zeit und Raum.
		
Wir spezialisieren uns auf interstellaren Antriebe.  Alle Wartungsarbeiten für zwei Jahre garantiert.

{lasttime}
Wartung Ihr {drive} wird {price} kosten.  Möchten Sie fortfahren?]],
	yesplease = "Ja, bitte fortfahren",
	response = "Wir haben die Arbeit an Ihrem hyperdrive abgeschlossen.",
	strength = 2.1, -- these guys are good.
	baseprice = 10,
})

Translate:AddFlavour('Deutsch','BreakdownServicing', {
	title = "{proprietor} Laufwerk Wartungs",
	intro = [[[Vermeiden Sie die Unannehmlichkeiten eines kaputten Hyperraumantrieb. Erhalten Sie Ihr gewartet heute.

Antrieb: {drive}
Dienst: {price}
{lasttime}]],
	yesplease = "Den Hyperraumantrieb pflegen",
	response = "Ihr Hyperraumantrieb gewartet worden ist.",
	strength = 0.0, -- These guys just reset the jump count.  Shoddy.
	baseprice = 1.8,
})

Translate:Add({ Deutsch = {
	["I don't have enough money"] = "Ich habe zu wenig Geld",
	["Your drive was last serviced on {date} by {company}"] = "Ihr Antrieb wurde zuletzt gewartet am {date} von {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Ihr Antrieb hat nicht gewartet sein, da er am {date} installiert wurde",
	["You do not have a drive to service!"] = "Sie haben keine Hyperraumantrieb zu pflegen!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "Der Hyperraumantrieb des Schiffs hat durch eine Fehlfunktion zerstört worden",
}, })

---- DUTCH / NEDERLANDS ----

Translate:AddFlavour('Nederlands','BreakdownServicing', {
   	title = "{name} Motor-service B.V.",
		intro = [[Vermijdt het ongemak van een kapotte hyperaandrijving!  Laat de uwe vandaag nog onderhouden door de officieël erkende {name} Motor-service B.V.

Hyperaandrijving: {drive}
Servicebeurt: {price}
Garantie: 18 maanden
{lasttime}]],
		yesplease = "Ik wil een servicebeurt",
		response = "Uw hyperaandrijving heeft een servicebeurt gekregen.",
		strength = 1.5,
		baseprice = 6,
})
  
Translate:AddFlavour('Nederlands','BreakdownServicing',{
		title = "{proprietor}: Hyperaandrijving onderhouds-specialist",
		intro = [[Ik ben {proprietor}.  Ik kan uw {drive} onderhouden, zodat deze er minstens een jaar tegenaan kan! De kosten bedragen {price}
{lasttime}]],
		yesplease = "Geef mijn hyperaandrijving een servicebeurt",
		response = "Ik heb uw hyperaandrijving een beurt gegeven.",
		strength = 1.2, -- Minstens een jaar... verborgen bonus!
		baseprice = 4,
})
  
Translate:AddFlavour('Nederlands','BreakdownServicing',{
		title = "{proprietor} & Co. HyperMonteurs",
		intro = [[Hallo.  Wij van {proprietor} & Co. zetten onze reputatie in op de kwaliteit van ons werk!

{lasttime}
We kunnen uw {drive} een onderhoudsbeurt geven, waarna we 12 maanden dienst garanderen.  De kosten bedragen {price}. Ik zal het werk zelf overzien om de kwaliteit te bewaken!]],
		yesplease = "Geeft u mijn hyperaandrijving alstublieft een beurt tegen de genoemde prijs",
		response = "Service voltooid.  Bedankt voor uw klanditie.",
		strength = 1.0,
		baseprice = 3,
})
  
Translate:AddFlavour('Nederlands','BreakdownServicing',{
		title = "SuperFix Onderhoud (afdeling {name})",
		intro = [[Welkom bij SuperFix Onderhoud.

{lasttime}
Tijd voor uw half-jaarlijkse onderhoudsbeurt?  Laat ons uw hyperaandrijving SuperFixen!
We kunnen uw {drive} onderhouden voor slechts {price}.  Niemand is goedkoper!]],
		yesplease = "Geef me een SuperFix!",
		response = "Uw SuperFix-service is voltooid, met een SuperFix-garantie!",
		strength = 0.5,
		baseprice = 2,
})
  
Translate:AddFlavour('Nederlands','BreakdownServicing',{
		title = "Tijd en Ruimte Motoren, B.V.",
		intro = [[Welkom bij Tijd en Ruimte.
		
Wij specialiseren ons in interstellaire aandrijf-systemen. Op al ons onderhoudswerk zit een garantie van twee jaar.

{lasttime}
Het onderhoud aan uw {drive} kost {price}.  Wilt u doorgaan?]],
		yesplease = "Ja, gaat u alstublieft aan de slag",
		response = "We hebben het werk aan uw hyperaandrijving voltooid.",
		strength = 2.1, -- zeer goede service van deze gasten.
		baseprice = 10,
})

Translate:AddFlavour('Nederlands','BreakdownServicing',{
		title = "{proprietor} Motor-service B.V.",
		intro = [[Vermijdt het ongemak van een kapotte hyperaandrijving!  Laat de uwe vandaag nog nalopen!

Hyperaandrijving: {drive}
Servicebeurt: {price}
{lasttime}]],
		yesplease = "Ik wil een servicebeurt",
		response = "Uw hyperaandrijving heeft een servicebeurt gekregen.",
		strength = 0.0, -- Deze heren geven zeer slechte service
		baseprice = 1.8,
})
	
Translate:Add({ Nederlands = {
  ["I don't have enough money"] = "Ik heb niet genoeg geld",
  ["Manufacturer's warranty"] = "Fabrieksgarantie",
  ["Your drive was last serviced on {date} by {company}"] = "Uw aandrijving is het laatst onderhouden op {date} door {company}",
  ["Your drive has not been serviced since it was installed on {date}"] = "Uw aandrijving heeft geen onderhoud gekregen sinds deze werd geïnstalleerd op {date}",
  ["You do not have a drive to service!"] = "Uw heeft geen aandrijving om onderhoud op te plegen!",
  ["The ship's hyperdrive has been destroyed by a malfunction"] = "De hyperaandrijving van het schip is vernietigd door een defect",
}, })

---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "Compañía de Revisión de Motores {name}",
	intro = [[Elimine el inconveniente de un motor hiperespacial averiado. Tenga hoy mismo el suyo revisado por la compañía homologada Revisión de Motores {name}.

Engine: {drive}
Service: {price}
Guarantee: 18 meses
{lasttime}]],
	yesplease = "Revise el motor",
	response = "Su motor ha sido revisado.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "{proprietor}: Especialistas en mantenimiento de motores de Hiperimpulso",
	intro = [[Soy {proprietor}.  Puedo revisar su {drive}, con garantía por un año.  El costo de la revisión será {price}
{lasttime}]],
	yesplease = "Revise mi motor",
	response = "Acabo de revisar su motor de hiperimpulso.",
	strength = 1.2, -- Mínimo un año... bonus ocultos!
	baseprice = 4,
})

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "{proprietor} & Co HyperMechanics",
	intro = [[Saludos.  En {proprietor} & Co ponemos nuestra reputación en el trabajo.

{lasttime}
Podemos afinar el {drive} de su nave, garantía por 12 meses, por la suma de {price}.  Estaré supervisando el trabajo yo mismo, por lo tanto puedo asegurarle un trabajo bien hecho.]],
	yesplease = "Por favor afine mi motor al precio acordado",
	response = "Revisión completa.  Gracias.",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "Mantenimiento SuperFix (Sucursal {name})",
	intro = [[Bienvenido Mantenimiento SuperFix.

{lasttime}
Ha llegado el momento de su mantenimiento bianual? Permítanos SuperReparar su motor de hiperimpulso!
Podemos tunear su {drive} por solo {price}.  No hay nada mas barato!]],
	yesplease = "SuperRepárame!",
	response = "La SuperReparación está completa, con garantía SuperFix!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "Motores Espacio Tiempo, Inc.",
	intro = [[Bienvenido a Motores Espacio Tiempo.
		
Nos especializamos en motores interestelares. Garantía por dos años.
{lasttime}
La revisión de su {drive} le costará {price}.  Quiere continuar?]],
	yesplease = "Si, por favor proceda",
	response = "Hemos completado el trabajo en su motor.",
	strength = 2.1, -- estos tipos son buenos.
	baseprice = 10,
})

Translate:AddFlavour('Spanish','BreakdownServicing', {
	title = "{proprietor} Compañía de Revisión de Motores",
	intro = [[Elimine el inconveniente de un motor de hiperimpulso averiado.  Optimice hoy mismo el suyo.

Engine: {drive}
Service: {price}
{lasttime}]],
	yesplease = "Revise el motor de hiperimpulso",
	response = "El motor ha sido revisado.",
	strength = 0.0, -- Estos tipos resetean la cuenta de saltos.  Chapuceros.
	baseprice = 1.8,
})

Translate:Add({ Spanish = {
	["I don't have enough money"] = "No dispongo de dinero suficiente",
	["Your drive was last serviced on {date} by {company}"] = "La última revisión de su motor fue el {date} por {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "Su motor no ha tenido ninguna revisión desde que fué instalado el {date}",
	["You do not have a drive to service!"] = "No dispone de motor para revisar!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "El motor de hiperimpulso de la nave se ha destruído por un mal funcionamiento",
}, })

---- HUNGARIAN / MAGYAR ----

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "{name} hajtóműszervízelő társaság",
	intro = [[Kerüld el a lerobbant hipermotor okozta kellemetlenségeket. Intézd el a szervízt még ma, a hivatalosan jóváhagyott {name} hajtóműszervízelő társaságnál.

Hajtómű: {drive}
Szervíz ára: {price}
Garancia: 18 hónap
{lasttime}]],
	yesplease = "Hipermotor szervízelés",
	response = "A hipermotor szervizelése megtörtént.",
	strength = 1.5,
	baseprice = 6,
})

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "{proprietor}: Hipermotor karbantartó specialista",
	intro = [[{proprietor} vagyok. Meg tudom javítani a(z) {drive} hipermotort, garantálva, hogy legalább 1 évig problémamentesen fog működni. Ennek ára pedig {price}
{lasttime}]],
	yesplease = "Hipermotorszervízt kérnék",
	response = "Megjavítottam a hipermotort.",
	strength = 1.2, -- legalább egy évig... rejtett bónusz!
	baseprice = 4,
})

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "{proprietor} & társa hiperszervízes",
	intro = [[Üdv, és helló. Mi a(z) {proprietor} & társánál a munkánk minősége a hírnevünk.

{lasttime}
Fel tudjuk javítani a hajód {drive} hajtóművét, biztosítva, hogy 12 hónapon át problémamentesen működjön, mindössze {price} összegért. Én magam felügyelem a munkát, szóval biztos lehetsz benne, hogy jó munkát végzünk.]],
	yesplease = "Kérlek javítsd fel a hipermotoromat a megadott árért",
	response = "Szervíz befejezve. Köszönjük a vásárlást.",
	strength = 1.0,
	baseprice = 3,
})

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "SuperFix karbantartás ({name} fiók)",
	intro = [[Üdvözöllek a Superfix karbantartásnál.

{lasttime}
Itt az idő a féléves ellenőrzésre? Engedd meg, hogy szuperfixáljuk a hipermotorod!
Meg tudjuk bütykölni a(z) {drive} hajtóművedet csupán {price} összegért. Senki sem olcsóbb nálunk!]],
	yesplease = "SuperFix módú javítást kérek!",
	response = "A SuperFix javítás kész van, SuperFix garanciával!",
	strength = 0.5,
	baseprice = 2,
})

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "Idő és Tér hajtóművek Rt.",
	intro = [[Üdv az Idő és Tér Rt.-nél.
		
A mi specialitásunk a csillagközi hajtómű. Minden szervízelésünkre 2 év garanciát vállalunk.
{lasttime}
A hajó {drive} meghajtójának szervize {price} összegbe kerül. Belekezdjünk?]],
	yesplease = "Igen, mehet a szervíz",
	response = "Befejeztük a munkát a hipermotoron.",
	strength = 2.1, -- these guys are good.
	baseprice = 10,
})

Translate:AddFlavour('Magyar','BreakdownServicing', {
	title = "{proprietor} hajtóműszervízelő társaság",
	intro = [[Kerüld el a lerobbant hipermotor okozta kellemetlenségeket. Szervizeld még ma.

Hajtómű: {drive}
Szervíz ára: {price}
{lasttime}]],
	yesplease = "Hipermotor szervizelése",
	response = "A hajtómű szervize kész.",
	strength = 0.0, -- These guys just reset the jump count.  Shoddy.
	baseprice = 1.8,
})

Translate:Add({ Magyar = {
	["I don't have enough money"] = "Nincs annyi pénzem",
	["Your drive was last serviced on {date} by {company}"] = "A hajtóművet {date} időpontban szervizelte a(z) {company}",
	["Your drive has not been serviced since it was installed on {date}"] = "A hajtómű még egyszer sem volt szervizelve a beszerelése, {date} óta.",
	["You do not have a drive to service!"] = "Nincs is szervizelni való hajtóműved!",
	["The ship's hyperdrive has been destroyed by a malfunction"] = "A hajó hipermotorja megsemmisült egy meghibásodás miatt",
}, })
