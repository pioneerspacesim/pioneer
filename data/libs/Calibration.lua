-- Copyright © 2008-2021 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--Profit as ratio of investment from Major export to minor import (estimated from the current economy)
local p = 0.25

--Average as ratio of nominal stock from Major export to minor import. See updateEquipmentStock function in SpaceStation.lua
local v = 1.30

--Maximum shift of stock from Major export to minor import. See updateEquipmentStock function in SpaceStation.lua
local v2 = 1.80

-- Definition of exponential
-- of current 48th line of SpaceStation.lua by the free cargo of 2 ships and the willing ratio of their profits.
-- Minimum free cargo for amphiesma is 12t. We have 16 ships and prefer them to increase their free cargo by constant ratio.
-- Our largest ship is dsminer with 3021t free cargo. The ratio RVab of free cargo volumes of ship 'a' and his next ship 'b' should be such that RVab^16=3021/12
local Nships = 16 --Number of ships we want to callibrate
local Vmax = 3021 -- maximum free available cargo of largest ship
local Vmin = 12 -- maximum free available cargo of smallest ship
local RVab = (Vmax / Vmin)^(1/Nships)

-- Ratio Rab of profits of 'a' and 'b' ships should be eye detectible by player. So, we want it above 1.20.
-- As i wanted between 1.20 and 1.25, i chosed, in a try not to get too away from current price of precious metals, to define it as 1.225
local Rab = 1.225

-- The exponential of former line 48 of SpaceStation.lua calculated by two ratios of volume and profit
local Exponential = function (RVolume, Rprofit)
	return math.log(RVolume)/(math.log(RVolume)-math.log(Rprofit))
end

local minMissiles = 2 --number of missiles that corresponds to minmum trading profit. In other words number of missiles smalest ship can acquire.
local maxMissiles = 24 --maximum number of missiles any ship can acquire. It should be get average reward about equal to the profit of the second in hierarchy trading ship.

local Calibration
Calibration = {
	--StockPriceOne can variate to fine tune stock and prices. I prefer it constant so as to define prices from stock. See function updateEquipmentStock in SpaceStation.lua.
	--On low prices rn are large integers that can not exceed much the 2*10^9 anyway.
	StockPriceOne = 10^9,
	PriceExponential = Exponential(RVab, Rab),
	avgStock = v
}

-- THE MOST IMPORTANT FUNCTION OF CALIBRATION. Average maximum profit of a ship that can trade cargo tones of his specialization commoditiy.
-- Once the initial and permanent settings for StockPriceOne and PriceExponential is taken, all future ships' profits and prices are defined according to them.
Calibration.profit = function (cargo)
	local a = Calibration.PriceExponential
	local constant = Calibration.StockPriceOne
	local coeff = (v2 * constant / cargo)^((1-a) / a)
	local profitTo = p * v * coeff * constant
	local profitFrom = p * v2 * coeff * constant / 2
	return (profitTo + profitFrom) / 2
end

--Define the ratio of fighting rewards to minimum trading profit in linear way. Used in Assassination and Combat modules.
Calibration.Fighting = function (missiles)
	local ratio = Rab^(Nships - 1)
	return math.max(1, (ratio - 1) / (maxMissiles - minMissiles) * missiles + 1 - 2 * (ratio - 1) / (maxMissiles - minMissiles))
end

-- The maximum profit of the base (smallest) ship.
Calibration.minProfit = Calibration.profit(Vmin)

--Nominal cash needed to trade all stock of a commodity depended on its nominal price.
Calibration.InvestmentOfPrice = function (price)
	return Calibration.StockPriceOne / price^(Calibration.PriceExponential - 1)
end

-- Adding prerequisite feature to ships in order to exclude them from function addRandomShipAdvert of SpaceStation.lua, if player is not their prequisite.
-- Prerequisite is table for example prerequisites["Sinonatrix"] = {"Amphiesma", "Lunar Shuttle"}

Calibration.prerequisite = function (pship,xship)
	local prerequisites ={}
	prerequisites["Lunar Shuttle"] = {"Amphiesma"}
	prerequisites["Sinonatrix"] = {"Lunar Shuttle"}
	prerequisites["Mola Mola"] = {"Sinonatrix"}
	prerequisites["Natrix"] = {"Mola Mola"}
	prerequisites["Skipjack"] = {"Natrix"}
	prerequisites["Deneb"] = {"Skipjack"}
	prerequisites["Bluenose"] = {"Deneb"}
	prerequisites["AC-33 Dropstar"] = {"Bluenose"}
	prerequisites["Mola Ramsayi"] = {"AC-33 Dropstar"}
	prerequisites["Venturestar"] = {"Mola Ramsayi"}
	prerequisites["Storeria"] = {"Venturestar"}
	prerequisites["Malabar"] = {"Storeria"}
	prerequisites["Nerodia"] = {"Malabar"}
	prerequisites["Vatakara"] = {"Nerodia"}
	prerequisites["Lodos"] = {"Vatakara"}

	if not prerequisites[xship] then
		return true
	else
		for i, p in pairs(prerequisites[xship]) do
		if p == pship then
				return true
			end
		end
		return false
	end
end

--------------------- Not used. Just helpfull -------------------------

--Nominal cash needed to trade all stock of a commodity depended on its nominal stock.
Calibration.InvestmentOfStock = function (stock)
	return (Calibration.StockPriceOne / stock)^(1 / Calibration.PriceExponential) * stock
end

--Nominal price of best commodity for a ship with cargo tones free cargo to trade
Calibration.BestCommodityPrice = function (cargo)
	return (v2 * Calibration.StockPriceOne / cargo)^(1 / Calibration.PriceExponential)
end

--Price of 'next' ship depended on current ship's free cargo and value along with 'turns' (or standard units or station visits) needed to acquire it.
Calibration.ShipPrice = function(currentShipPrice, currentCargo, turns)
	return Calibration.profit(currentCargo) * turns + currentShipPrice / 2
end

return Calibration
