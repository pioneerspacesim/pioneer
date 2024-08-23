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
local Commodities = require 'Commodities'
local l = Lang.GetResource("ui-core")

local ModalWindow = require 'pigui.libs.modal-win'

local rescaleVector = ui.rescaleUI(Vector2(1, 1), Vector2(1600, 900), true)

local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local face = nil
local stationSeed = false

local crimeTableID = "##CrimeTable"

local popup = ModalWindow.New('policePopup', function(self)
	ui.text(l.YOU_NOT_ENOUGH_MONEY .. ".")
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
	tablePadding = Vector2(12, 4)
}, Vector2(1600, 900))


local function payfine(fine)
	if Game.player:GetMoney() < fine then
		popup:open()
		return
	end
	Game.player:AddMoney(-fine)
	Game.player:ClearCrimeFine()
end

local function make_crime_list(record)
	local crimes = utils.build_array(utils.map(
		function(crime, v) return true, { count = v.count, name = Legal.CrimeType[crime].name } end,
		pairs(record)
	))

	table.sort(crimes, function(a, b) return a.count > b.count end)

	return crimes
end

-- Render a list of outstanding crimes
local function crime_table(crimes)
	ui.withStyleVars({ CellPadding = widgetSizes.tablePadding }, function()
		ui.beginTable(crimeTableID, 2, { "BordersInnerV" })

		ui.tableSetupColumn("count", "WidthFixed")
		ui.tableSetupColumn("name", "WidthStretch")

		for _, v in pairs(crimes) do
			ui.tableNextRow()

			ui.tableNextColumn()
			ui.text(v.count)

			ui.tableNextColumn()
			ui.text(v.name)
		end

		ui.endTable()
	end)
end

local function crime_record()
	local past_crimes = make_crime_list(Game.player:GetCrimeRecord())

	if #past_crimes > 0 then
		ui.withFont(orbiteer.heading, function()
			ui.text(l.CRIMINAL_RECORD .. ":")
		end)

		ui.withStyleColors({ Text = ui.theme.colors.fontDim }, function()
			crime_table(past_crimes)
		end)
	end
end


local function outstanding_fines()
	local crimes, fine = Game.player:GetCrimeOutstanding()

	local crime_list = make_crime_list(crimes)
	if #crime_list > 0 then

		-- headline
		ui.withFont(orbiteer.heading, function()
			ui.text(l.OUTSTANDING_FINES .. ":")
		end)

		-- wrap list in medlarge font
		ui.withFont(pionillium.body, function()
			crime_table(crime_list)
			ui.spacing()

			local canPay = Game.player:GetMoney() >= fine
			local pay_fine_text = string.interp(l.PAY_FINE_OF_N,
				{ amount = Format.Money(fine) })

			if ui.button(pay_fine_text, nil, not canPay and ui.theme.buttonColors.disabled) then
				payfine(fine)
			end

			if not canPay and ui.isItemHovered() then
				ui.setTooltip(l.YOU_NOT_ENOUGH_MONEY)
			end
		end)
	else
		ui.withFont(pionillium.body, function()
			ui.text(l.WE_HAVE_NO_BUSINESS_WITH_YOU)
		end)
	end
end

local function drawRulesAndRegulations()
	local station = Game.player:GetDockedWith()

	local header = string.interp(l.LEGAL_RULES, {
		distance = Format.Distance(station.lawEnforcedRange),
	})
	ui.text(header)
	ui.newLine()

	local lawlessness = Game.system.lawlessness

	local crimeList = utils.build_array(utils.map(
		function(k, v) return true, { name = v.name, fine = Legal:fine(k, lawlessness)} end,
		pairs(Legal.CrimeType)
	))

	table.sort(crimeList, function(a, b) return a.name < b.name end)

	ui.withStyleVars({ CellPadding = widgetSizes.tablePadding }, function()
		ui.beginTable("Crimes", 2, { "BordersInnerH" })
		ui.tableSetupColumn(l.CRIME .. ":")
		ui.tableSetupColumn(l.FINE .. ":")

		ui.withFont(pionillium.heading, function()
			ui.tableHeadersRow()
		end)

		for _, v in ipairs(crimeList) do
			ui.tableNextRow()

			ui.tableNextColumn()
			ui.text(v.name .. ":")

			ui.tableNextColumn()
			ui.textAligned(Format.Money(v.fine), 1.0)
		end

		ui.endTable()
	end)
end

local policeTabs = {
	{
		name = l.CRIMINAL_RECORD,
		draw = function()
			-- 1. If outstanding fines, show list & offer to pay
			outstanding_fines()

			ui.dummy(widgetSizes.dummySpaceSmall)

			-- 2 If old payed fines, show grayd out list
			crime_record()
		end
	},
	{
		name = l.LEGAL_RULES_AND_REGULATIONS,
		draw = drawRulesAndRegulations
	},
	{
		name = l.ILLEGAL_COMMODITIES,
		draw = function()
			ui.textWrapped(l.POSESSION_OF_COMMODITIES_ILLEGAL)
			ui.spacing()
			ui.separator()
			ui.spacing()
			ui.textWrapped(l.THESE_ITEMS_ARE_PUNISHABLE_BY_FINE .. ":")

			ui.spacing()

			-- Build a list of illegal goods in this system
			local illegal = utils.map_array(utils.build_array(pairs(Commodities)), function(comm)
				return not Game.system:IsCommodityLegal(comm.name) and comm:GetName() or nil
			end)

			-- Sort the list lexicographically
			table.sort(illegal)

			for _, name in ipairs(illegal) do
				ui.text(name)
			end
		end
	}
}

local function drawPolice()
	local intro_txt = string.interp(l.THIS_IS_FACTION_POLICE,
		{ faction_police = Game.system.faction.policeName, faction = Game.system.faction.name})

	ui.withStyleVars({ItemSpacing = widgetSizes.itemSpacing}, function()
		local padding = ui.getWindowPadding()
		local infoColumnWidth = ui.getContentRegion().x
			- widgetSizes.faceSize.x - padding.x * 2

		ui.child("CrimeStats", Vector2(infoColumnWidth, 0), {}, function()
			ui.withFont(pionillium.heading, function ()
				ui.text(intro_txt)
			end)

			ui.tabBarFont("tabs", policeTabs, pionillium.heading)
		end)

		ui.sameLine(0, padding.x * 2)
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
