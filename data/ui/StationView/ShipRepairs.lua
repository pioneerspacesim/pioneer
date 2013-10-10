-- Copyright © 2008-2013 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Engine = import("Engine")
local Game = import("Game")
local ShipDef = import("ShipDef")
local Lang = import("Lang")
local Format = import("Format")

local l = Lang.GetResource("ui-core")
local ui = Engine.ui

local getRepairCost = function (percent)
	-- repairing 1% hull damage costs 0.1% of ship price
	local shipDef = ShipDef[Game.player.shipId]
	return math.ceil(shipDef.basePrice * (percent * 0.1)) * 0.01
end

local repairItemTemplate = function (damage, price, handler)
	local btn = ui:Button(ui:Expand('HORIZONTAL', ui:Label(string.interp(
            l.REPAIR_X_HULL_DAMAGE_FOR_X, {
				damage = string.format('%.1f', damage),
				price = Format.Money(price)
			}
		))))
	btn.onClick:Connect(function () handler(damage, price); end)
	return btn
end

local function createRepairOptions (root)
	local hullPercent = Game.player.hullPercent

	if root ~= nil then
		root:Clear()
	end

	-- XXX this display should probably include the player's current cash somewhere

	if hullPercent >= 100 then
		root:PackEnd(ui:Label(l.SHIP_IS_ALREADY_FULLY_REPAIRED))
	else
		local hullDamage = 100 - hullPercent
		local smallDamage = math.min(hullDamage, 1)
		local costRepair1 = getRepairCost(smallDamage)
		local costRepairAll = getRepairCost(hullDamage)
		local feedback = ui:Label('')

		local repairDamage = function (damage, price)
			-- repair the hull for the money!
			if Game.player:GetMoney() >= price then
				Game.player:AddMoney(-price)
				Game.player:SetHullPercent(hullPercent + damage)
				createRepairOptions(root)
			else
				feedback:SetText(t("You don't have enough money for that option."))
			end
		end

		local rows = {}
		rows[1] = ui:Label(string.interp(l.YOUR_HULL_IS_AT_X_INTEGRITY,
			{value = string.format('%.1f', hullPercent)}))
		rows[2] = repairItemTemplate(smallDamage, costRepair1, repairDamage)
		if hullDamage > 1 then
			rows[3] = repairItemTemplate(hullDamage, costRepairAll, repairDamage)
		end
		rows[4] = feedback

		root:PackEnd(rows)
	end

	return root
end

local shipRepairs = function (args)
	local root = ui:VBox(5)
	createRepairOptions(root)
	return root
end

return shipRepairs
