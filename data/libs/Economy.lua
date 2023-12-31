-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Rand       = require 'Rand'
local Game       = require 'Game'
local Serializer = require 'Serializer'
local utils      = require 'utils'
local Engine     = require 'Engine'

---@class Economy
local Economy = package.core['Economy']

---@class Economy.StationMarket
---@field commodities table<string, table>
---@field lastStockUpdate number

-- Determines how far a commodity's price can be perturbed from the system-wide
-- average price at an individual station
local kMaxCommodityVariance = 15

local Economies = {}
local Commodities = Economy.GetCommodities()

-- Create a stable iteration order for economies to avoid non-deterministic results
for _, econ in pairs(Economy.GetEconomies()) do
	table.insert(Economies, econ)
end

table.sort(Economies, function(a, b) return a.id < b.id end)

-- Percentage modifier applied to buying/selling commodities
-- Prevents buying a commodity at a station and immediately reselling it
Economy.TradeFeeSplit = 2
Economy.TotalTradeFees = 2 * Economy.TradeFeeSplit

-- stationMarket is a persistent table of stock information for every station
-- the player has visited in their journey
local stationMarket = {}

local affinityCache = {}

-- Function: GetStationEconomy
--
-- This function calculates the economy-type affinities of the given station
-- based on several body parameters. It's intended as a stub to generate
-- interesting and sometimes-useful results until a more complete and
-- parameter-rich implementation can be added to system generation directly.
--
-- Status: experimental
---@param stationBody SystemBody
---@return table<integer, number> affinity Station's affinity to a specific economic type
function Economy.GetStationEconomy(stationBody)
	local sBody = stationBody.parent

	if not sBody then return {} end

	if affinityCache[stationBody.path] then
		return affinityCache[stationBody.path]
	end

	logVerbose("Computing station economy type for Station {name}" % stationBody)

	local econAffinity = {}
	local max = 0.0
	local sum = 0.0

	-- Two-pass design attempting to maintain a variance between the primary
	-- producing economy on the station and the secondary economies

	local rand = Rand.New('station-econ-{}' % { stationBody.seed })

	for _, econ in ipairs(Economies) do
		local score = 0.0
		score = score + econ.generation.agricultural * math.max(sBody.agricultural, sBody.life * 0.75)
		score = score + econ.generation.metallicity * (sBody.metallicity + sBody.volatileIces) * 0.5
		score = score + econ.generation.industrial * (sBody.volcanicity * sBody.metallicity + sBody.atmosOxidizing) * 0.5
		score = score + rand:Number(0, 1)

		econAffinity[econ.id] = score
		sum = sum + score
	end

	for _, econ in ipairs(Economies) do
		local score = econAffinity[econ.id]
		local offset = rand:Number(0, math.min(sum, score))

		score = score + offset
		sum = sum - offset

		logVerbose("\teconomy {} affinity {}" % { econ.name, score })
		econAffinity[econ.id] = score
		max = math.max(max, score)
	end

	for k, v in pairs(econAffinity) do
		econAffinity[k] = v / max
	end

	affinityCache[stationBody.path] = econAffinity

	return econAffinity
end

-- Modify the given stock or demand level for a commodity based on the
-- system-wide import/export status of the commodity
--
-- Pricemod is technically a percentage modification of the commodity price
-- but functions more like a factor controlling relative supply and demand of
-- the commodity - positive values indicate higher demand, while negative
-- values indicate higher supply
--
-- Pricemod is scaled according to a log curve to increase the offset applied
-- at higher percentages.
function Economy.ApplyPriceMod(max, val, pricemod)
	local priceScale = 4.0 - math.clamp(math.log(math.abs(pricemod)), 1, 3)
	return val - (max * pricemod * 0.01 * priceScale)
end

-- Function: GetMaxStockForPrice
--
-- Returns the maximum commodity stocking at any station based on price
-- adjusted for some rarity curve by an exponent.
--
-- The higher the nominal price of the commodity, the exponentially less
-- maximum stock this station will have of it - this models commodity rarity
-- as a function of commodity price.
function Economy.GetMaxStockForPrice(price)
	return 290000 / price^1.217
end

-- weight a scalar in the range -1..1 away from 0 to generate more interesting results
local function weight_affinity(a)
	local v = math.abs(a) - 1
	return math.sign(a) * ( 1 + v * v * v ) -- out cubic easing
end

-- Function: GetStationTargetStock
--
-- Calculate the target stock level for a commodity based on the given station
-- seed number. This is used to determine the persistent equilibrium stock for
-- "non-market" commodities at a station. (e.g. rubbish and fuels)
--
-- Parameters:
--   key  - string, name of the commodity
--   seed - number, the target space station's unique seed value
--
-- Returns:
--  targetStock - the persistent equilibrium stock amount of the commodity for
--                the given station steed.
function Economy.GetStationTargetStock(key, seed)
	-- use a deterministic random function to determine target stock numbers
	local rand = Rand.New(seed .. '-stock-' .. key)
	local comm = Commodities[key]
	local rn = Economy.GetMaxStockForPrice(math.abs(comm.price))

	return rn * (rand:Number() + rand:Number()) * 0.5 -- normal 0-100% "permanent" stock
end

---@param stationBody SystemBody
---@param comm table the commodity data involved
function Economy.GetStationFlowParams(stationBody, comm)
	local affinities = Economy.GetStationEconomy(stationBody)

	-- use a deterministic random function to determine target stock numbers
	local rand = Rand.New('station-{}-stock-{}' % { stationBody.seed, comm.name })

	-- Calculate the total flow for a commodity at this station based on a
	-- normal distribution.

	-- "Flow" models both supply and demand of a commodity, and is used to
	-- determine equilibrium state
	-- NOTE: a better model for calculating commodity flow would be nice -
	-- this produces varied but irrational results
	local flow = (rand:Number() + rand:Number()) * 0.5

	-- Calculate this station's proportion of export to import based on its
	-- affinity to the producing economy.
	-- A station with positive affinity primarily exports the commodity while a
	-- station with negative affinity primarily imports the commodity.
	local affinity = (affinities[comm.producer] or 1) * 2 - 1

	-- Randomly scale or invert commodity affinity at this station to provide more variance
	-- TODO: this should be removed (or its intensity reduced) once more economy types exist
	-- This term is only here to avoid all stations in the galaxy being primarily agrarian
	affinity = affinity * rand:Normal(0, 1)

	-- Weight and clamp the affinity away from 0 to ensure stations have at least minimal trade
	affinity = weight_affinity(affinity)
	affinity = math.clamp(affinity, -0.99, 0.99)

	-- Adjust the flow at this station to reduce stocking for minimal trade commodities
	-- The smaller the absolute value of affinity is, the lower commodity trade at this station
	-- Apply the weighting function to affinity again to reduce the chance of a commodity having
	-- no trade at all
	flow = flow * math.abs(weight_affinity(affinity) * 1.2)

	-- logVerbose("{}: flow {}, affinity: {}" % { comm.name, flow, affinity })

	return flow, affinity
end

function Economy.GetCommodityStockFromFlow(comm, flow, affinity)
	affinity = affinity * 0.5 + 0.5

	local flowQuant = flow * Economy.GetMaxStockForPrice(math.abs(comm.price))

	-- Compute real stock and demand numbers in commodity units
	local stock = math.ceil(affinity * flowQuant)
	local demand = math.ceil((1 - affinity) * flowQuant)

	return stock, demand
end

-- Function: CreateStationCommodityMarket
--
-- Calculate the target stock, demand, and nominal pricing for the given
-- commodity in the 'equilibrium' state at the passed station.
--
-- This is the "baseline" amount that the stock level will naturally return to
-- over time in the absense of any external factors.
--
-- Parameters:
--   stationBody - SystemBody, the target space station body
--   key         - string, name of the commodity
--
-- Returns:
--  market - table containing information about the equilibrium stock, demand
--           and price modifier
---@param stationBody SystemBody
---@param key string the string name of the commodity
function Economy.CreateStationCommodityMarket(stationBody, key)
	local comm = Commodities[key]
	-- return a new "empty" market table here so it can be modified downstream
	if not comm then return { 0, 0, 0 } end

	local flow, affinity = Economy.GetStationFlowParams(stationBody, comm)

	-- Calculate the typical amount of stock and demand in commodity units at
	-- this station to seed the market with, as well as the real price of the
	-- item at equilibrium
	local stock, demand = Economy.GetCommodityStockFromFlow(comm, flow, affinity)

	return { stock, demand, 0 }
end

-- Function: UpdateCommodityPriceMod
--
-- Recompute the price modifier of the given commodity according to the current
-- market parameters
--
-- Parameters:
--   stationBody - SystemBody, the target space station body
--   key         - string, name of the commodity
--   commMarket  - table, information about the commodity market
--
---@param sBody SystemBody the station's SystemBody
---@param key string the string name of the commodity
---@param commMarket table the current state of the commmodity market
function Economy.UpdateCommodityPriceMod(sBody, key, commMarket)
	local comm = Commodities[key]
	if not comm then return 0 end

	local maxStock = Economy.GetMaxStockForPrice(math.abs(comm.price))
	local systemMod = sBody.system:GetCommodityBasePriceAlterations(key)

	-- [stock, demand]
	local stockPrice  = commMarket[1] / maxStock * -kMaxCommodityVariance
	local demandPrice = commMarket[2] / maxStock *  kMaxCommodityVariance

	commMarket[3] = utils.round(stockPrice + demandPrice, 0.01) + systemMod
end

-- Function: GetStationCommodityPrice
--
-- Return a modified price according to the given commodity market conditions
function Economy.GetMarketPrice(price, pricemod)
	return price * (1 + pricemod * 0.01)
end

local kAvgTicksToRestock = 12

---@param sBody SystemBody
function Economy.UpdateStationCommodityMarket(sBody, rand, market, key, numTicks)
	local comm = Commodities[key]

	local flow, affinity = Economy.GetStationFlowParams(sBody, comm)
	local targetStock, targetDemand = Economy.GetCommodityStockFromFlow(comm, flow, affinity)

	local stock, demand, pricemod = market[1], market[2], market[3]

	for i = 1, numTicks do
		stock = stock + rand:Normal(1, 1) / kAvgTicksToRestock * targetStock
		demand = demand + rand:Normal(1, 1) / kAvgTicksToRestock * targetDemand
	end

	stock = math.clamp(math.ceil(stock), 0, targetStock)
	demand = math.clamp(math.ceil(demand), 0, targetDemand)

	market[1] = stock
	market[2] = demand
end

-- create a persistent entry for the given station's commodity market if it
-- does not already exist, and populate persistent and transient stock info
---@param sBody SystemBody
---@return Economy.StationMarket
function Economy.CreateStationMarket(sBody)
	if stationMarket[sBody.path] then
		return stationMarket[sBody.path]
	end

	local storedStation = {
		commodities = {},
		lastStockUpdate = Game.time
	}

	stationMarket[sBody.path] = storedStation

	local h2 = Commodities.hydrogen
	for key, comm in pairs (Commodities) do
		local market

		if comm.price > 0.0 and comm ~= h2 then
			-- Don't create a persistent equilibrium for "haulaway" commodities or fuel

			-- Initialize the station's stock levels to the equilibrium
			market = Economy.CreateStationCommodityMarket(sBody, key)

		elseif comm.price < 0.0 then
			-- Create a random stock value which can be different for every save

			-- "haulaway" commodities
			-- approximately 1t rubbish for every 10 people
			local rn = math.floor(math.max(sBody.population * 1e8, 100) / math.abs(comm.price))
			local stock = Engine.rand:Integer(rn / 10, rn)
			local demand = Engine.rand:Integer(0, stock / 50) -- extremely low buyback demand if any

			market = { stock, demand, 0 }

		elseif comm == h2 then
			-- Create a random stock value which can be different for every save

			-- make sure we always have enough hydrogen here at the station
			local rn = Economy.GetStationTargetStock(key, sBody.seed)
			local stock = Engine.rand:Integer((rn + 1)/4, rn + 1)
			local demand = Engine.rand:Integer(stock / 10, stock / 4)

			market = { stock, demand, 0 }

		end

		Economy.UpdateCommodityPriceMod(sBody, key, market)
		storedStation.commodities[key] = market

		logVerbose("\t{}\n\tstock: {}, demand: {}, priceMod: {}, system priceMod: {}" % {
			key, market[1], market[2], market[3],
			sBody.system:GetCommodityBasePriceAlterations(key)
		})
	end

	return storedStation
end

function Economy.GetStationMarket(path)
	return stationMarket[path]
end

local kTickDuration = 7 * 24 * 60 * 60 -- 1 week

-- Handle gradually restocking commodities at a station over time.
--
-- By default, a station restores to its maximum stock from complete depletion
-- after 12 weeks.
--
-- This function is safe to call at any time, though it should be rate-limited
function Economy.UpdateStationMarket(sBody)
	local storedStation = stationMarket[sBody.path]
	if not storedStation then return end

	local lastStockUpdate = storedStation.lastStockUpdate
	local timeSinceUpdate = Game.time - lastStockUpdate
	if timeSinceUpdate <= kTickDuration then return end

	-- make sure the next tick happens at the correct time
	local numTicks = math.floor(timeSinceUpdate / kTickDuration)
	storedStation.lastStockUpdate = lastStockUpdate + numTicks * kTickDuration

	-- use a unique random function to tally up restocks
	-- this ensures that each restock uses a different random number based on game start time
	local randRestock = Rand.New(sBody.seed .. '-stockMarketUpdate-' .. math.floor(lastStockUpdate))

	local h2 = Commodities.hydrogen
	for key, data in pairs (storedStation.commodities) do
		local comm = Commodities[key]
		logVerbose("\tcommodity {} was {}/{} (%{})" % { key, data[1], data[2], data[3] })

		if comm.price > 0 and comm ~= h2 then
			-- persistent commodities get a full simulation returning towards equilibrium
			Economy.UpdateStationCommodityMarket(sBody, randRestock, data, key, numTicks)
		else
			-- simple random walk for h2/haulaway commodities
			data[1] = math.ceil(math.abs(Engine.rand:Normal(data[1], data[1] / kAvgTicksToRestock)))
			data[2] = math.ceil(math.abs(Engine.rand:Normal(data[2], data[2] / kAvgTicksToRestock)))
		end

		Economy.UpdateCommodityPriceMod(sBody, key, data)

		logVerbose("\tcommodity {} now {}/{} (%{})" % { key, data[1], data[2], data[3] })
	end
end

function Economy.OnGameEnd()
	stationMarket = {}
	affinityCache = {}
end

Serializer:Register("Economy",
	function()
		return {
			market = stationMarket
		}
	end,
	function(data)
		stationMarket = data.market or {}
	end
)

return Economy
