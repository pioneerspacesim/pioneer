-- Copyright © 2008-2014 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Format = import("Format")
local Comms = import("Comms")
local utils = import("utils")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")

local ui = Engine.ui
local l = Lang.GetResource("ui-core")

local crimeStrings = {
	TRADING_ILLEGAL_GOODS = l.TRADING_ILLEGAL_GOODS,
	WEAPON_DISCHARGE      = l.UNLAWFUL_WEAPONS_DISCHARGE,
	PIRACY                = l.PIRACY,
	MURDER                = l.MURDER,
}

local police = function (tab)
	local station = Game.player:GetDockedWith()

	local rand = Rand.New(util.hash_random(station.seed .. '-police', 2^31-1) - 1)
	local face = InfoFace.New(Character.New({
		title  = l.CONSTABLE,
		armour = true,
	}, rand))

	local crimes, fine = Game.player:GetCrime()

	local infoBox = ui:VBox(10)
	if #crimes > 0 then
		infoBox:PackEnd({
			ui:Label(l.CRIMINAL_RECORD):SetFont("HEADING_LARGE"),
			ui:VBox():PackEnd(
				utils.build_table(utils.map(function (k,v) return k,crimeStrings[v] end, pairs(crimes)))
			),
		})
	end

	local actionBox = ui:VBox()
	infoBox:PackEnd(actionBox)

	local function noBusiness ()
		actionBox:Clear()
		actionBox:PackEnd(l.WE_HAVE_NO_BUSINESS_WITH_YOU)
	end

	if fine == 0 then
		noBusiness()
	else
		local b = SmallLabeledButton.New(string.interp(l.PAY_FINE_OF_N, { amount = Format.Money(fine) }))
		actionBox:PackEnd(b)
		b.button.onClick:Connect(function ()
			if Game.player:GetMoney() < fine then
				Comms.Message(l.YOU_NOT_ENOUGH_MONEY)
				return
			end

			Game.player:AddMoney(-fine)
			Game.player:ClearCrimeFine()

			noBusiness()
		end)
	end

	return
		ui:Grid({48,4,48},1)
			:SetColumn(0, {
				infoBox
			})
			:SetColumn(2, {
				face.widget
			})
end

return police
