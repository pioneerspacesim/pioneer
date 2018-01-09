-- Copyright © 2008-2018 Pioneer Developers. See AUTHORS.txt for details
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

	local rand = Rand.New(station.seed .. '-police')
	local face = InfoFace.New(Character.New({
		title  = l.CONSTABLE,
		armour = true,
	}, rand))

	local crimes, fine = Game.player:GetCrimeOutstanding()
	local tmp_table = {}
	for k,v in pairs(crimes) do
		table.insert(tmp_table,k)
	end

	local infoBox = ui:VBox(10)
	if #utils.build_array(pairs(crimes)) > 0 then
		infoBox:PackEnd({
			ui:Label(l.OUTSTANDING_FINES):SetFont("HEADING_LARGE"),
			ui:VBox():PackEnd(
				utils.build_table(utils.map(function (k,v)
												return k, ui:Label(crimes[v].count.."\t"..Legal.CrimeType[v].name)
											end, pairs(tmp_table)))),
		})
	end

	local actionBox = ui:VBox()
	infoBox:PackEnd(actionBox)

	local past_crimes, stump = Game.player:GetCrimeRecord()
	local tmp_table_old = {}
	for k,v in pairs(past_crimes) do
		table.insert(tmp_table_old, k)
		print(v.count.."\t"..Legal.CrimeType[k].name) -- todo xxx
	end

	local grey = { r = 0.3, g = 0.3, b = 0.3}
	local past_crimes_infoBox = ui:VBox(10)
	if #utils.build_array(pairs(past_crimes)) > 0 then
		past_crimes_infoBox:PackEnd({
			ui:Label(l.CRIMINAL_RECORD):SetFont("HEADING_LARGE"):SetColor(grey),
			ui:VBox():PackEnd(
				utils.build_table(utils.map(function (k,v)
												return k, ui:Label(past_crimes[v].count.."\t"..Legal.CrimeType[v].name):SetColor(grey)
											end, pairs(tmp_table_old)))),
		})
	end


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
				ui:VBox(50):PackEnd({infoBox, past_crimes_infoBox})
			})
			:SetColumn(2, {
				face.widget
			})
end

return police
