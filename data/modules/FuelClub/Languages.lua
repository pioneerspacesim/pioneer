-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- There's only one flavour as I write, but more could be added
Translate:AddFlavour('English','FuelClub',{
	-- Translators: Feel free to change this in your language!
	-- It's a proper name, so exact translation is not vital.
	clubname = "Interstellar Pilots' Fuel Co-operative",
	welcome = 'Welcome to {clubname}',
	nonmember_intro = [[{clubname} is an independent organisation dedicated to providing discounted starship fuel to its members. Branches can be found throughout the galaxy. Benefits of membership include:

	* Our own stocks of fuel, independent of the general market
	* {hydrogen} at discount prices
	* {military_fuel} at discount prices
	* {water} at discount prices
	* {radioactives}, free disposal (conditions apply)
	* Fuel tank refilling, where necessary

Join now! Annual membership costs only {membership_fee}]],
	member_intro = [[You may purhase fuel and dispose of {radioactives} here.]],
	annual_fee = 400,
})

Translate:Add({English = {
	["What conditions apply to {radioactives} disposal?"] = "What conditions apply to {radioactives} disposal?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us.",
	["Apply for membership"] = "Apply for membership",
	["Your membership application has been declined."] = "Your membership application has been declined.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "You are now a member. Your membership will expire on {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "You must buy our {military_fuel} before we will take your {radioactives}",
	["Begin trade"] = "Begin trade",
}})

  ---- POLISH / POLSKI ----

Translate:AddFlavour('Polski','FuelClub',{

	clubname = "Spółdzielnia Paliwowa Pilotów Międzygwiezdnych",
	welcome = 'Witamy w Spółdzielni Paliwowej Pilotów Międzygwiezdnych',
	nonmember_intro = [[{clubname} jest niezależną organizacją, udostępniającą swoim członkom własne zapasy paliwa, posiadamy filie w całej galaktyce. Korzyści płynące z członkostwa:

	* Posiadamy własne zapasy paliwa, niezależne od zapasów na wolnym rynku.
	* {hydrogen} w atrakcyjnej cenie.
	* {military_fuel} w atrakcyjnej cenie.
	* {water} w atrakcyjnej cenie.
	* {radioactives} - darmowa utylizacja (pod pewnymi warunkami).
	* Uzupełnienie zbiornika z paliwem, kiedy to konieczne.

Przyłącz się! Członkostwo kosztuje tylko {membership_fee} rocznie]],
	member_intro = [[Zakup paliwa i utylizacja odpadów radioaktywnych.]],
	annual_fee = 400,
})

Translate:Add({Polski = {
	["What conditions apply to {radioactives} disposal?"] = "Jakie ograniczenia obowiązują przy utylizacji odpadów radioaktywnych?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Zutylizujemy tylko tyle odpadów radioaktywnych, ile zakupisz od nas paliwa wojskowego.",
	["Apply for membership"] = "Wystąp o członkostwo",
	["Your membership application has been declined."] = "Twój wniosek o członkostwo został odrzucony.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Wniosek został przyjęty. Twoje członkostwo wygasa {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Musisz zakupić u nas {military_fuel} zanim zutylizujemy twoje {radioactives}",
	["Begin trade"] = "Pohandlujmy",
}})

   ---- SPANISH / ESPAÑOL ----

Translate:AddFlavour('Spanish','FuelClub',{
	-- Translators: Feel free to change this in your language!
	-- It's a proper name, so exact translation is not vital.
	clubname = "Cooperativa Interestelar de Combustibles",
	welcome = 'Bienvenido a {clubname}',
	nonmember_intro = [[{clubname} es una organización independiente dedicada a proveer descuentos en combustible a sus miembros. Sus sedes se encuentran por toda la galaxia. Los beneficios de afiliación incluyen:

	* Existencias de combustible propias, independientes del mercado general
	* Descuento de precios en {hydrogen}
	* Descuento de precios en {military_fuel}
	* {radioactives}, disponibilidad libre (bajo condiciones)

Apúntese! Afiliación anual por solo {membership_fee}]],
	member_intro = [[Puede adquirir combustible y disponer de {radioactives} aquí.]],
	annual_fee = 400,
})

Translate:Add({Spanish = {
	["What conditions apply to {radioactives} disposal?"] = "Cuáles son las condiciones aplicables en la disponibilidad de {radioactives}?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Solo dispensaremos una cantidad de {radioactives} equivalente a las toneladas de {military_fuel} adquiridas.",
	["Apply for membership"] = "Solicitar el ingreso",
	["Your membership application has been declined."] = "Su ingreso ha sido rechazado.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Ahora es miembro. Su afiliación expira en {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Debe adquirir nuestro {military_fuel} para dispensarle {radioactives}",
	["Begin trade"] = "Comenzar transacción",
}})


  ---- HUNGARIAN / MAGYAR ----

Translate:AddFlavour('Magyar','FuelClub',{

	clubname = "Csillagközi pilóták üzemanyagegyüttműködése",
	welcome = 'Üdvözlünk a(z) {clubname} klubban.',
	nonmember_intro = [[{clubname} egy független szervezet, amely olcsón kínál űrhajóüzemanyagot tagjainak. A fiókok szerte megtalálhatóak a galaxisban. A tagság előnyei a következők:

	* Saját üzemanyagraktárunk árai függetlenek a piactól
	* {hydrogen} diszkontáron kapható
	* {military_fuel} diszkontáron kapható
	* {radioactives} megsemmisítése ingyen (feltételekkel)

Csatlakozz most! Az éves tagsági díj csak {membership_fee}]],
	member_intro = [[Továbbá vehetsz üzemanyagot és {radioactives} megsemmisítést is vállalunk.]],

	annual_fee = 400,
})

Translate:Add({Magyar = {
	["What conditions apply to {radioactives} disposal?"] = "Mik a feltételei a {radioactives} kezelésének?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Annyi tonna {radioactives} semlegesítését vállaljuk, amennyi {military_fuel} vásárolsz tőlünk.",
	["Apply for membership"] = "Tagságigénylés",
	["Your membership application has been declined."] = "A tagságigénylésedet visszautasítottuk.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Immáron tag vagy. A tagságod {expiry_date} napján jár le.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Venned kell: {military_fuel} mielőtt átvennénk tőled: {radioactives}",
	["Begin trade"] = "Üzlet megkezdése",
}})

  ---- RUSSIAN / РУССКИЙ ----

Translate:AddFlavour('Russian','FuelClub',{

	clubname = "Космическое Топливное Содружество Пилотов",
	welcome = 'Космическое Топливное Содружество Пилотов приветствует вас, капитан!',
	nonmember_intro = [[{clubname} - независимая организация, предоставляющая своим членам собственные топливные ресурсы. Наша организация работает по всей Галактике. Членство даёт следующие преимущества:

	* Собственные запасы топлива,  независимые от предложения на открытом рынке.
	* {hydrogen} по привлекательной цене.
	* {military_fuel} продается без акцизов.
	* {water} по сниженным ценам.
	* {radioactives} - принимаются для утилизации бесплатно (при определенных условиях).
	* Заправка топливного бака, по необходимости.

Присоединяйтесь! Членство в Содружестве стоит всего лишь {membership_fee} в год.]],
	member_intro = [[Здесь вы можете приобрести топливо и сдать радиоактивные отходы на утилизацию.]],
	annual_fee = 400,
})

Translate:Add({Russian = {
	["What conditions apply to {radioactives} disposal?"] = "Какие ограничения имеются для реализации бесплатной утилизации радиоактивных отходов?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Вы можете сдать на утилизацию столько же тонн радиоактивных отходов, сколько тонн военного топлива вы у нас приобрели.",
	["Apply for membership"] = "Мне заинтересовало ваше предложения. Вот моя заявка на вступление в Содружество.",
	["Your membership application has been declined."] = "Ваша заявка на вступление отклонена, сожалеем.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Ваша заявка одобрена Содружеством. Ваш членский билет действует до {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Перед тем как сдать на утилизацию {radioactives}, вы должны приобрести у нас {military_fuel}.",
	["Begin trade"] = "Провести сделку.",
}})

  ---- GERMAN / DEUTSCH ----

Translate:AddFlavour('Deutsch','FuelClub',{

	clubname = "Treibstoff-Kooperative Interstellarer Piloten",
	welcome = 'Wilkommen bei der {clubname}',
	nonmember_intro = [[{clubname} ist eine unabhängige Organisation, deren Ziel es ist, günstigen Raumschifftreibstoff an seine Mitglieder zu Verkaufen. Ableger können überall in der Galaxie gefunden werden. Die Vorteile einer Mitgliedschaft umfassen:

	* Unsere eigenen, vom Markt unabhängigen, Treibstoffvorräte
	* {hydrogen} zu Discount-Preisen
	* {military_fuel} zu Discount-Preisen
	* {water} zu Discount-Preisen
	* {radioactives}, kostenlose Entsorgung (siehe Konditionen)
	* Kraftstoff-Nachfüllung wo es nötig ist

Trete uns noch heute bei! Jährliche Mitgliedschaft nur {membership_fee}]],
	member_intro = [[Du kannst hier Kraftstoffe kaufen und {radioactives} entsorgen.]],
	annual_fee = 400,
})

Translate:Add({Deutsch = {
	["What conditions apply to {radioactives} disposal?"] = "Welche Konditionen gelten bei der Entsorgung von {radioactives}?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Wir werden die gleiche Menge {radioactives} entsorgen, wie du {military_fuel} von uns gekauft hast.",
	["Apply for membership"] = "Für Mitgliedschaft bewerben",
	["Your membership application has been declined."] = "Deine Bewerbung wurde abgelehnt.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Du bist nun Mitglied. Die Mitgliedschaft verfällt am {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Du musst unseren {military_fuel} kaufen, bevor wir deine {radioactives} annehmen",
	["Begin trade"] = "Beginne Handel",
}})

  ---- CZECH / ČESKY ----

Translate:AddFlavour('Czech','FuelClub',{
	-- Translators: Feel free to change this in your language!
	-- It's a proper name, so exact translation is not vital.
	clubname = "Palivová-Kooperativa mezihvězdných pilotů",
	welcome = 'Vítej v {clubname}',
	nonmember_intro = [[{clubname} je nezávislá organizace, která si klade za cíl poskytovat svým členům cenově zvýhodněné palivo pro jejich lodě. Její pobočky lze nalézt po celé galaxii. Členské výhody zahrnují:

	* Naše vlastní, na trhu nezávislé, zásoby paliva
	* {hydrogen} za slevněné ceny
	* {military_fuel} za slevněné ceny
	* {water} za slevněné ceny
	* {radioactives}, likvidace zdarma (viz podmínky)
	* Fuel tank refilling, where necessary
	* Doplnění paliva, kde je potřeba

Přidej se k nám! Roční členství stojí pouhých {membership_fee}]],
	member_intro = [[Zde si můžeš zakoupit palivo a zlikvidovat {radioactives}.]],
	annual_fee = 400,
})

Translate:Add({Czech = {
	["What conditions apply to {radioactives} disposal?"] = "Za jakých podmínek lze zlikvidovat {radioactives}?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "Pro {radioactives} platí následující pravidlo: zlikvidujeme ho tolik tun, kolik {military_fuel} jsi u nás koupil.",
	["Apply for membership"] = "Pouze pro členy",
	["Your membership application has been declined."] = "Vaše žádost o členství byla zamítnuta.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "Nyní jste členem. Vaše členství vyprší {expiry_date}.",
	["You must buy our {military_fuel} before we will take your {radioactives}"] = "Nejdřív musíš koupit naše {military_fuel} než převezmeme tvůj {radioactives}",
	["Begin trade"] = "Provést výměnu",
}})
