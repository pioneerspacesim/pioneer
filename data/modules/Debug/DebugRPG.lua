-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Format = require "Format"
local ui = require 'pigui'
local debugView = require 'pigui.views.debug'
local amount = 1000
local selected = 0

local Legal = require "Legal"
local utils = require "utils"
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

-- build list of all crime types:
local crime_types = {}
for k, v in pairs(Legal.CrimeType) do
	table.insert(crime_types, k)
end

local Character = require "Character"

local Equipment = require 'Equipment'
local ci = 0

local commodities = {}
local commodities_name = {}
local selected_commodity = 0
local cargo_amount = 0
local changed_commodity = false
local get_commodities = function()
	for key, equip in pairs(Equipment.cargo) do
		table.insert(commodities, equip)
		table.insert(commodities_name, equip:GetName())
	end
end

debugView.registerTab("RPG-debug-view", function()
	if Game.player == nil then return end
    if not ui.beginTabItem("RPG") then return end
		ui.text("State: " .. Game.player:GetFlightState())

		-- Reputation
		Character.persistent.player.reputation = ui.sliderInt("Reputation", Character.persistent.player.reputation, 0, 512)
		ui.sameLine()
		ui.text(Character.persistent.player:GetReputationRating())

		-- Kills
		Character.persistent.player.killcount = ui.sliderInt("Kills", Character.persistent.player.killcount, 0, 6000)
		ui.sameLine()
		ui.text(Character.persistent.player:GetCombatRating())
		ui.endTabItem()

		-- Ship hull condition
		local hull = ui.sliderInt("Hull", Game.player.hullPercent, 0, 100)
		Game.player:SetHullPercent(hull)

		if ui.collapsingHeader("Money", {"DefaultOpen"}) then
			-- args: label, default, min, max, (optional: str format)
			amount = ui.sliderInt("Amount", amount, 0, 100000)
			if ui.button("Give money", Vector2(100, 0)) then
				Game.player:AddMoney(amount)
			end
			ui.sameLine()
			ui.text("Current money: " .. Format.Money(Game.player:GetMoney()))
		end
		ui.separator()

		local rows = 10
		if ui.collapsingHeader("Crime", {}) then
			local changed, ret = 0

			ui.text("ADD CRIMINAL CHARGES:")
			local selected_crime = 0
			for i, v in pairs(crime_types) do
				if ui.selectable(v:lower(), selected_crime == i, {}) then
					Legal:notifyOfCrime(Game.player, crime_types[i])
				end
			end

			ui.dummy(Vector2(0,10))
			-- CRIME
			local crimes, fine = Game.player:GetCrimeOutstanding()
			if #utils.build_array(pairs(crimes)) > 0 then
				ui.text(l.OUTSTANDING_FINES)
				for k,v in pairs(crimes) do
					ui.text(v.count.."\t"..Legal.CrimeType[k].name)
				end
				ui.text("Fine:\t".. tostring(fine))
			end

			ui.dummy(Vector2(0,10))
			local past_crimes, _ = Game.player:GetCrimeRecord()
			if #utils.build_array(pairs(past_crimes)) > 0 then
				ui.text(l.CRIMINAL_RECORD)
				for k,v in pairs(past_crimes) do
					local s = v.count.."\t"..Legal.CrimeType[k].name
					ui.text(s, Vector2(100, 0))
				end
			end
		end


		-- Load up on commodities
		if ui.collapsingHeader("Cargo", {}) then  -- {"OpenOnDoubleClick"}
			-- to do: max cargo space
			cargo_amount = ui.sliderInt("##CommodityAmount", cargo_amount, 0, 100)

			if #commodities == 0 then
				get_commodities()
			end

			changed_commodity, commodity_idx = ui.combo("Commodities", selected_commodity, commodities_name)

			if changed_commodity then
				selected_commodity = commodity_idx
				ci = selected_commodity + 1
			end
			ui.text("Selected comodity index " ..selected_commodity)

			if ui.button("Buy commodity", Vector2(100, 0)) then
				print(commodities, ci)            -- when ci=0
				print(commodities[ci])			  -- and this is nil, next line crashes. If nothing has been selected?
				print(commodities[ci]:GetName())
				Game.player:AddEquip(commodities[ci], cargo_amount, "cargo")
			end

		end

end)
