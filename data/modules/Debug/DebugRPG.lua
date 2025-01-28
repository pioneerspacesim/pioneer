-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Format = require "Format"
local ui = require 'pigui'
local debugView = require 'pigui.views.debug'
local Commodities = require 'Commodities'
local Equipment   = require 'Equipment'
local amount = 1000

local Legal = require "Legal"
local utils = require "utils"
local Lang = require 'Lang'
local l = Lang.GetResource("ui-core")

-- build list of all crime types:
local crime_types = {}
for k, _ in pairs(Legal.CrimeType) do
	table.insert(crime_types, k)
end

local Character = require "Character"

local commodities = {}
local commodities_name = {}
local selected_commodity = 0
local cargo_amount = 0

local get_commodities = function()
	for _, commodity in pairs(Commodities) do
		table.insert(commodities, commodity)
	end

	table.sort(commodities, function(a, b) return a:GetName() < b:GetName() end)

	for _, commodity in ipairs(commodities) do
		table.insert(commodities_name, commodity:GetName())
	end
end

local equipment = {}
local selected_equip = 0

local get_equipment = function()
	for id, equip in pairs(Equipment.new) do
		table.insert(equipment, { "{} (S{})" % { equip:GetName(), equip.slot and equip.slot.size or "-" }, equip })
	end

	table.sort(equipment, function(a, b) return a[1] < b[1] end)
end

debugView.registerTab("debug-player", {
	icon = ui.theme.icons.personal,
	label = "Player Debug",
	show = function() return Game.player ~= nil end,
	draw = function()
		ui.text("State: " .. Game.player:GetFlightState())

		-- Reputation
		Character.persistent.player.reputation = ui.sliderInt("Reputation", Character.persistent.player.reputation, 0, 512)
		ui.sameLine()
		ui.text(Character.persistent.player:GetReputationRating())

		-- Kills
		Character.persistent.player.killcount = ui.sliderInt("Kills", Character.persistent.player.killcount, 0, 6000)
		ui.sameLine()
		ui.text(Character.persistent.player:GetCombatRating())

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

		if ui.collapsingHeader("Crime", {}) then

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

		-- Add equipment
		if ui.collapsingHeader("Equipment") then

			if #equipment == 0 then
				get_equipment()
			end

			local active_equip = equipment[selected_equip]

			ui.comboBox("##Selected", active_equip and active_equip[1] or "<None>", function()
				for i, pair in ipairs(equipment) do
					if ui.selectable(pair[1], selected_equip == active_equip) then
						selected_equip = i
						active_equip = pair
					end
				end
			end)

			if active_equip then

				ui.sameLine()

				local equipSet = Game.player:GetComponent('EquipSet')

				local slot = equipSet:GetFreeSlotForEquip(active_equip[2])

				if ui.button("Install") and slot then

					---@type EquipType
					local inst = active_equip[2]:Instance()

					if inst.SpecializeForShip then
						inst:SpecializeForShip(equipSet.config)
					end

					if slot.count then
						inst:SetCount(slot.count)
					end

					if equipSet:CanInstallInSlot(slot, inst) then
						equipSet:Install(inst, slot)
					end

				end

				ui.text(slot and "Slot Status: Available" or "Slot Status: Unavailable or Occupied")

			end

		end

		-- Load up on commodities
		if ui.collapsingHeader("Cargo", {}) then  -- {"OpenOnDoubleClick"}

			---@type CargoManager
			local cargoMgr = Game.player:GetComponent('CargoManager')
			local free_space = cargoMgr:GetFreeSpace()

			ui.nextItemWidth(-1.0)
			cargo_amount = ui.sliderInt("##CommodityAmount", math.min(cargo_amount, free_space), 0, free_space)

			if #commodities == 0 then
				get_commodities()
			end

			if ui.button("Buy commodity") then
				cargoMgr:AddCommodity(commodities[selected_commodity + 1], cargo_amount)
			end

			ui.sameLine()
			ui.nextItemWidth(-1.0)

			local _
			_, selected_commodity = ui.combo("##Commodities", selected_commodity, commodities_name)

		end

	end
})
