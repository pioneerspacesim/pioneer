-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Lang = import("Lang")
local Game = import("Game")
local ShipDef = import("ShipDef")
local Format = import("Format")
local Rand = import("Rand")
local InfoGauge = import("ui/InfoGauge")
local InfoFace = import("ui/InfoFace")
local ModelSpinner = import("UI.Game.ModelSpinner")
local Character = import("Character")

local l = Lang.GetResource("ui-core")
local ui = Engine.ui

local getRepairCost = function (percent, shipDef)
	-- repairing 1% hull damage costs 0.1% of ship price
	shipDef = shipDef or ShipDef[Game.player.shipId]
	return math.ceil(shipDef.basePrice * (percent * 0.1)) * 0.01
end

local getRepairMessage = function (damage, price)
	return string.interp(
		l.REPAIR_X_HULL_DAMAGE_FOR_X, {
			damage = string.format('%.1f', damage),
			price = Format.Money(price)
		})
end

local shipRepairs = function (args)
	local messageLabel = ui:Label('<message>')
	local feedbackLabel = ui:Label('<feedback>')
	local repairButtons = ui:VBox(5)
	local repair1Label = ui:Label('Repair 1% hull damage for $.')
	local repairAllLabel = ui:Label('Repair all hull damage $$$.')
	local repair1Btn = ui:Button(ui:Expand('HORIZONTAL', repair1Label))
	local repairAllBtn = ui:Button(ui:Expand('HORIZONTAL', repairAllLabel))

	local cashLabel = ui:Label('<cash>')
	local integrityGauge = InfoGauge.New({
		formatter = function (v) return string.format('%.0f%%', v * 100); end,
		warningLevel = 0.5,
		criticalLevel = 0.2,
		levelAscending = false,
	})

	local damageAll, damage1, costRepairAll, costRepair1

	local update = function (feedbackText)
		local shipDef = ShipDef[Game.player.shipId]
		local hullPercent = Game.player.hullPercent

		cashLabel:SetText(string.interp(l.CASH_N,
			{money = Format.Money(Game.player:GetMoney())}))
		integrityGauge:SetValue(hullPercent / 100)

		if hullPercent >= 100 then
			messageLabel:SetText(l.SHIP_IS_ALREADY_FULLY_REPAIRED)
			feedbackLabel:SetText(feedbackText)
			repairButtons:Clear()
		else
			damageAll = 100 - hullPercent
			damage1 = math.min(damageAll, 1)
			costRepair1 = getRepairCost(damage1, shipDef)
			costRepairAll = getRepairCost(damageAll, shipDef)

			messageLabel:SetText(string.interp(l.YOUR_HULL_IS_AT_X_INTEGRITY,
				{value = string.format('%.1f', hullPercent)}))
			feedbackLabel:SetText(feedbackText)

			repair1Label:SetText(getRepairMessage(damage1, costRepair1))
			repairAllLabel:SetText(getRepairMessage(damageAll, costRepairAll))

			repairButtons:Clear()
			repairButtons:PackEnd(repair1Btn)
			if damageAll > damage1 then
				repairButtons:PackEnd(repairAllBtn)
			end
		end
	end

	local tryRepair = function (damage, price)
		if Game.player:GetMoney() >= price then
			Game.player:AddMoney(-price)
			Game.player:SetHullPercent(Game.player.hullPercent + damage)
			update('')
		else
			update(l.YOU_DONT_HAVE_ENOUGH_MONEY_FOR_THAT_OPTION)
		end
	end

	repair1Btn.onClick:Connect(function () tryRepair(damage1, costRepair1); end)
	repairAllBtn.onClick:Connect(function () tryRepair(damageAll, costRepairAll); end)

	-- initialise UI state
	update('')

	-- XXX need a better way of seeding this
	local station = Game.player:GetDockedWith()
	local rand = Rand.New(util.hash_random(station.seed .. '-repair-guy', 4294967296) - 1)
	local face = InfoFace.New(Character.New({ title = "Repair Guy" }, rand))

	local repairShop = ui:VBox(5):PackEnd({
		ui:Grid({1,1,1},1):SetCell(1, 0, face),
		messageLabel,
		repairButtons,
		feedbackLabel,
		ui:Expand('VERTICAL'),
	})

	local shipDef = ShipDef[Game.player.shipId]
	local shipView = ui:VBox(5):PackEnd({
		ui:Grid({1,2,1},1):SetCell(1, 0, ModelSpinner.New(ui, shipDef.modelName, Game.player:GetSkin())),
		ui:Expand('VERTICAL'),
		cashLabel,
		ui:Label(l.HULL_INTEGRITY),
		integrityGauge,
	})

	return
		ui:Grid(2,1)
			:SetCell(0, 0, ui:Margin(20, 'RIGHT', repairShop))
			:SetCell(1, 0, ui:Margin(20, 'LEFT', shipView))
end

return shipRepairs
