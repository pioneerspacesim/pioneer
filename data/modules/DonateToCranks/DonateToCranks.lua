-- Get the translator function
local t = Translate:GetTranslator()

local ads = {}

local onChat = function (form, ref, option)
	local crank_flavours = Translate:GetFlavours('DonateToCranks')
	local ad = ads[ref]

	if option == 0 then
		form:Clear()

		form:SetTitle(ad.title)
		form:SetFace({ seed = ad.faceseed })
		form:SetMessage(ad.message)

		form:AddOption("$1", 1)
		form:AddOption("$10", 10)
		form:AddOption("$100", 100)
		form:AddOption("$1000", 1000)
		form:AddOption("$10000", 10000)
		form:AddOption("$100000", 100000)
		form:AddOption(t('HANG_UP'), -1)

		return
	end

	if option == -1 then
		form:Close()
		return
	end

	if Game.player:GetMoney() < option then
		UI.Message(t("You do not have enough money."))
	else
		if option >= 10000 then
			UI.Message(t("Wow! That was very generous."))
		else
			UI.Message(t("Thank you. All donations are welcome."))
		end
		Game.player:AddMoney(-option)
	end
end

local onDelete = function (ref)
	ads[ref] = nil
end

local onCreateBB = function (station)
  local crank_flavours = Translate:GetFlavours('DonateToCranks')
	local n = Engine.rand:Integer(1, #crank_flavours)

	local ad = {
		title    = crank_flavours[n].title,
		message  = crank_flavours[n].message,
		station  = station,
		faceseed = Engine.rand:Integer()
	}

	local ref = station:AddAdvert(ad.title, onChat, onDelete)
	ads[ref] = ad
end

local loaded_data

local onGameStart = function ()
	ads = {}

	if not loaded_data then return end

	for k,ad in pairs(loaded_data.ads) do
		local ref = ad.station:AddAdvert(ad.title, onChat, onDelete)
		ads[ref] = ad
	end

	loaded_data = nil
end

local serialize = function ()
	return { ads = ads }
end

local unserialize = function (data)
	loaded_data = data
end

EventQueue.onCreateBB:Connect(onCreateBB)
EventQueue.onGameStart:Connect(onGameStart)

Serializer:Register("DonateToCranks", serialize, unserialize)
