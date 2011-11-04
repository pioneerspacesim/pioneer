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
	annual_fee = 500,
})

Translate:Add({English = {
	["What conditions apply to {radioactives} disposal?"] = "What conditions apply to {radioactives} disposal?",
	["We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us."] = "We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us.",
	["Apply for membership"] = "Apply for membership",
	["Your membership application has been declined."] = "Your membership application has been declined.",
	["You are now a member. Your membership will expire on {expiry_date}."] = "You are now a member. Your membership will expire on {expiry_date}.",
}})
