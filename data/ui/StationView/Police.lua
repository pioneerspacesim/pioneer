-- Copyright © 2008-2015 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local Rand = import("Rand")
local Character = import("Character")
local Format = import("Format")
local utils = import("utils")
local Legal = import("Legal")

local InfoFace = import("ui/InfoFace")
local SmallLabeledButton = import("ui/SmallLabeledButton")
local MessageBox = import("ui/MessageBox")

local ui = Engine.ui
local l = Lang.GetResource("ui-core")


local police = function (tab)

	local station = Game.player:GetDockedWith()

	local rand = Rand.New(util.hash_random(station.seed .. '-police', 2^31-1) - 1)
	local face = InfoFace.New(Character.New({
		title  = l.CONSTABLE,
		armour = true,
	}, rand))

	local crimes, fine = Game.player:GetCrime()

	local tmp_table = {}
	for k,v in pairs(crimes) do
		table.insert(tmp_table,k)
	end

	local crimeStat = function (k,v)
		local s = crimes[v].count.."\t"..Legal.CrimeType[v].name
		return k, s
	end

	local infoBox = ui:VBox(10)
	if #utils.build_array(pairs(crimes)) > 0 then
		infoBox:PackEnd({
			ui:Label(l.CRIMINAL_RECORD):SetFont("HEADING_LARGE"),
			ui:VBox():PackEnd(
				utils.build_table(utils.map(crimeStat, pairs(tmp_table)))
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
				MessageBox.Message(l.YOU_NOT_ENOUGH_MONEY)
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
