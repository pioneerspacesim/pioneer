-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require "Game"
local Rand = require "Rand"
local ui = require 'pigui'
local StationView = require 'pigui.views.station-view'
local Lang = require 'Lang'
local Legal = require "Legal"
local utils = require "utils"
local PiGuiFace = require 'pigui.libs.face'
local Format = require "Format"
local Character = require "Character"
local l = Lang.GetResource("ui-core")

local ModalWindow = require 'pigui.libs.modal-win'

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local face = nil
local stationSeed = false

local popup = ModalWindow.New('policePopup', function(self)
	ui.text(l.YOU_NOT_ENOUGH_MONEY)
	ui.dummy(Vector2((ui.getContentRegion().x - 100*rescaleVector.x) / 2, 0))
	ui.sameLine()
	if ui.button(l.OK, Vector2(100*rescaleVector.x, 0)) then
		self:close()
	end
end)

local gray = Color(100, 100, 100)

local widgetSizes = ui.rescaleUI({
	itemSpacing = Vector2(4, 9),
	faceSize = Vector2(586,565),
	crimeRecordColumnWidth = 55,
	buttonSize = Vector2(100,0),
	dummySpaceSmall = Vector2(0, 10),
	dummySpaceMedium = Vector2(0, 50),
}, Vector2(1600, 900))


local function payfine(fine)
	if Game.player:GetMoney() < fine then
		popup:open()
		return
	end
	Game.player:AddMoney(-fine)
	Game.player:ClearCrimeFine()
end


local function crime_record()
	local past_crimes, stump = Game.player:GetCrimeRecord()
	if #utils.build_array(pairs(past_crimes)) > 0 then
		ui.withFont(orbiteer.heading, function()
			ui.textColored(gray, l.CRIMINAL_RECORD)
		end)
		ui.withFont(pionillium.body, function ()
			for k,v in pairs(past_crimes) do
				ui.textColored(gray, v.count)
				-- start second column at this position:
				ui.sameLine(widgetSizes.crimeRecordColumnWidth)
				ui.textColored(gray, Legal.CrimeType[k].name)
			end
		end)
	end
end


local function outstanding_fines()
	local crimes, fine = Game.player:GetCrimeOutstanding()
	if #utils.build_array(pairs(crimes)) > 0 then

		-- headline
		ui.dummy(widgetSizes.dummySpaceSmall)
		ui.withFont(orbiteer.heading, function()
			ui.text(l.OUTSTANDING_FINES)
		end)

		-- wrap list in medlarge font
		ui.withFont(pionillium.body, function()
			for k,v in pairs(crimes) do
				ui.text(v.count)
				-- start second column at this position:
				ui.sameLine(widgetSizes.crimeRecordColumnWidth)
				ui.text(Legal.CrimeType[k].name)
			end
			local pay_fine_text = string.interp(l.PAY_FINE_OF_N,
				{ amount = Format.Money(fine) })
			ui.text(pay_fine_text)

			if ui.button(l.PAY, widgetSizes.buttonSize) then
				payfine(fine)
			end
		end)
	else
		ui.withFont(pionillium.body, function()
			ui.text(l.WE_HAVE_NO_BUSINESS_WITH_YOU)
		end)
	end
end


local function drawPolice()
	local intro_txt = string.interp(l.THIS_IS_FACTION_POLICE,
		{ faction_police = Game.system.faction.policeName, faction = Game.system.faction.name})

	ui.withStyleVars({ItemSpacing = widgetSizes.itemSpacing}, function()
		local infoColumnWidth = ui.getContentRegion().x
			- widgetSizes.faceSize.x - widgetSizes.itemSpacing.x

		ui.child("CrimeStats", Vector2(infoColumnWidth, 0), {}, function()
			ui.withFont(pionillium.heading, function ()
				ui.text(intro_txt)
			end)

			ui.newLine()

			-- 1. If outstanding fines, show list & offer to pay
			outstanding_fines()

			ui.dummy(widgetSizes.dummySpaceMedium)

			-- 2 If old payed fines, show grayd out list
			crime_record()
		end)

		ui.sameLine()
		if(face ~= nil) then face:render() end
	end)
end


StationView:registerView({
	id = "police",
	name = l.POLICE,
	icon = ui.theme.icons.shield_other,
	showView = true,
	draw = drawPolice,
	refresh = function()
		local station = Game.player:GetDockedWith()
		if (station) then
			if (stationSeed ~= station.seed) then
				stationSeed = station.seed
				local rand = Rand.New(station.seed .. "-police")
				face = PiGuiFace.New(Character.New({ title = l.CONSTABLE, armour=true }, rand),
					{itemSpacing = widgetSizes.itemSpacing})
			end
		end
	end,
	debugReload = function()
		package.reimport()
	end
})
