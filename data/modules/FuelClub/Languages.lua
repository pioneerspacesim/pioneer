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
	* {radioactives}, free disposal (conditions apply)

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
	
	clubname = "Spółdzielnia Paliwowa",
	welcome = 'Witamy w Spółdzielni Paliwowej',
	nonmember_intro = [[{clubname} jest niezależną organizacją, udostępniającą swoim członkom własne zapasy paliwa, posiadamy filie w całej galaktyce. Korzyści płynące z członkostwa:

	* Posiadamy własne zapasy paliwa, niezależne od zapasów na wolnym rynku.
	* {hydrogen} w atrakcyjnej cenie.
	* {military_fuel} bez akcyzy.
	* {radioactives} - darmowa utylizacja. (pod pewnymi warunkami)

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
