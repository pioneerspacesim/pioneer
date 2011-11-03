---------------
-- Fuel Club --
---------------

-- The fuel club is an organisation that provides subsidized hydrogen fuel,
-- military fuel and radioactives processing for its members. Membership is
-- normally annual. A Goods Trader interface is provided. Facilities do not
-- exist on every station in the galaxy.

-- Get the translator function
local t = Translate:GetTranslator()

-- Default numeric values --
----------------------------
local oneyear = 31557600 -- One standard Julian year
-- 10, guaranteed random by D16 dice roll.
-- This is to make the BBS name different from the station welcome character.

-- 27, guaranteed random by D100 dice roll.
-- This is to make the BBS name different from the station welcome character.
-- This will be unnecessary once ported to Character
local seedbump = 27

local ads = {}
local memberships = {
-- some_club = {
--	joined = 0,
--	expiry = oneyear,
-- }
}

-- 1 / probability that you'll see one in a BBS
local chance_of_availability = 1

local loaded_data -- empty unless the game is loaded

local onGameStart = function ()
	local ref
	if loaded_data then
		-- rebuild saved adverts
		for k,ad in pairs(loaded_data.ads) do
			ads[ad.station:AddAdvert(ad.flavour.clubname, onChat, onDelete)] = ad
		end
		-- load membership info
		memberships = loaded_data.memberships
		loaded_data = nil
	end
end

local onDelete = function (ref)
	-- ad has been destroyed; forget its details
	ads[ref] = nil
end

local onChat = function (form, ref, option)
	local ad = ads[ref]

	local setMessage = function (message)
		form:SetMessage(message:interp({
			hydrogen = t('HYDROGEN'),
			military_fuel = t('MILITARY_FUEL'),
			radioactives = t('RADIOACTIVES'),
			clubname = ad.flavour.clubname,
		}))
	end

	form:Clear()
	form:SetFace(ad.face)
	form:SetTitle(ad.flavour.welcome:interp({clubname = ad.flavour.clubname}))
	local membership = memberships[ad.flavour]
	if membership and (membership.joined + membership.expiry > Game.time) then
		-- members get the trader interface
		setMessage(ad.flavour.member_intro)
	elseif option == -1 then
		-- hang up
		form:Close()
	elseif option == 1 then
		-- Player asked the question about radioactives
		setMessage(t('We will only dispose of as many tonnes of {radioactives} as you have bought tonnes of {military_fuel} from us.'))
		form:AddOption(t('Apply for membership'),2)
		form:AddOption(t('GO_BACK'),0)
		form:AddOption(t('HANG_UP'),-1)
	elseif option == 2 then
		-- Player applied for membership
		setMessage(t('Your membership application has been declined.'))
		form:AddOption(t('HANG_UP'),-1)
	else
		-- non-members get offered membership
		setMessage(ad.flavour.nonmember_intro:interp({
			membership_fee = Format.Money(500) -- Placeholder
		}))
		form:AddOption(t('What conditions apply to {radioactives} disposal?'):interp({radioactives = t('RADIOACTIVES')}),1)
		form:AddOption(t('Apply for membership'),2)
		form:AddOption(t('HANG_UP'),-1)
	end
end

local onCreateBB = function (station)
	-- deterministically generate our instance
	local rand = Rand:New(station.seed + seedbump)
	if rand:Integer(1,chance_of_availability) == 1 then
	print('***** FUELCLUB: ',station.label)
		-- Create our bulletin board ad
		local ad = {}
		local flavours = Translate:GetFlavours('FuelClub')
		print(flavours,' ',#flavours)
		ad.flavour = flavours[rand:Integer(1,#flavours)]
		print(ad.flavour,' ',#ad.flavour)
		-- I'd like to replace this with a Character sheet for sheer
		-- convenience of face generation!
		ad.face = {
			female = (rand:Integer(1) == 1),
			seed = rand:Integer(),
			name = NameGen.FullName(female,rand),
			title = ad.flavour.clubname,
		}
		ads[station:AddAdvert(ad.flavour.clubname,onChat,onDelete)] = ad
	end
	print('***** CONSIDERED: ',station.label)
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)

-- Serializer:Register("FuelClub", serialize, unserialize)
