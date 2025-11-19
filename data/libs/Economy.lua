-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Rand       = require 'Rand'
local Game       = require 'Game'
local Json       = require 'Json'
local Serializer = require 'Serializer'
local Engine     = require 'Engine'
local utils      = require 'utils'

local Conditions = require 'Economy.Conditions'
local Industry   = require 'Economy.Industry'

--
-- Interface: Economy
--
-- This interface is reponsible for maintaining the game's economic simulation.
-- It models economies at the levels of individual starports, representing them
-- as a collection of <Industries> which inform the supply and demand values
-- of the local economy.
--
-- Any commodities which are not produced at a station are first sourced from
-- the "local neighborhood"; commodities not produced locally are assumed to
-- be imported from distant planets or via interstellar trade.
--
-- It is important that the operations to create an initial equilibrium remain
-- something that can be easily implemented in C++ code. The overall goal of
-- this economy system is to be computable at system generation time to inform
-- system-wide economic data. As such, both the Conditions and Industries
-- abstractions must remain possible to move to C++ without dependencies on Lua-
-- specific operations or concepts.
--

local AU = 149598000000

---@class Economy
local Economy = package.core['Economy']

-- Determines how far a commodity's price can be perturbed from the system-wide
-- average price at an individual station
local kMaxCommodityVariance = 15

-- Amount pricemod is increased for a local surplus or deficit
-- pricemod is computed as pricemod + variance * (ln(supply) - supply_flow)
-- local kSurplusDeficitVariance = 20

-- Stddev of the random walk for commodity supply/demand values, expressed as a fraction of the
-- equilibrium stock/demand.
-- This controls how quickly commodity supply/demand will wander about over a unit of time
-- equal to kSupplyDemandScale * kMarketUpdateTick.
local kSupplyDemandDeviation = 0.16

-- Stddev of the random walk bounds for commodity supply/demand values, expressed as a fraction
-- of the equilibrium supply/demand.
-- Supply/demand values will lie within +-2*kSupplyDemandBound proportions of the mean under 95% of conditions.
-- (For an equilibrium supply of 100, this means commodity supply will almost never dip below 60 nor go above 140.)
local kSupplyDemandBound = 0.3

-- Time period for the deviation value, in multiples of kMarketUpdateTick
-- It is expected for the walk to wander with a std.dev. of kSupplyDemandDeviation
-- over a time period of kSupplyDemandScale * kMarketUpdateTick
local kSupplyDemandScale = 30

-- Stddev of the random walk for commodity stocking values at stations
local kStockDeviation = 0.15

-- Time period for the station commodity stock value, in multiples of kMarketUpdateTick
-- Commodity stock values at a station are expected to wander with a std.dev. of kStockDeviation
-- (relative to the current equilibrium) over this time period.
local kStockUpdateScale = 10

-- Duration of a single "market update" step, primarily used when in-system for regular market updates.
-- When returning to a system, commodity markets will be extrapolated based on the various duration
-- scalars listed above.
local kMarketUpdateTick = 60 * 60 -- 1hr for debug testing

local Economies = utils.to_array(Economy.GetEconomies())
local Commodities = Economy.GetCommodities()

local CommodityList = utils.build_array(utils.keys(Commodities))
table.sort(CommodityList)

-- Create a stable iteration order for economies to avoid non-deterministic results
table.sort(Economies, function(a, b) return a.id < b.id end)

-- Percentage modifier applied to buying/selling commodities
-- Prevents buying a commodity at a station and immediately reselling it
Economy.TradeFeeSplit = 2

-- Total trade fee percentage applied to a buy->sell transaction
Economy.TotalTradeFees = 2 * Economy.TradeFeeSplit

-- Scalar multiplier applied when reselling "used" equipment items back onto the market
Economy.BaseResellPriceModifier = 0.8

---@class Economy.EconomyDef
---@field industries string[]
---@field tags table<string, boolean>
---@field supply table<string, integer>
---@field demand table<string, integer>

---@class Economy.SupplyGroup
---@field primary SystemPath
---@field supply table<string, integer>
---@field starports SystemPath[]

---@class Economy.StationMarket
--- Station supply values are the instantaneous production rate of the given commodity at this specific station.
---@field supply table<string, number>
--- Station demand values are the instantaneous consumption rate of the given commodity at this specific station.
---@field demand table<string, number>
--- Stock values are the transient, instantaneous available quantity of the commodity for purchase by the player.
--- Commodity stock does not currently inform pricing, as it is just a "window" into the export or import status
--- of the commodity.
---@field stock table<string, number>
---@field history table<string, number>
---@field updated number

-- cache of deterministic station economies
---@type table<SystemPath, Economy.EconomyDef>
local economyCache = {}

-- cache of locally-available supply amounts by starport path
---@type table<SystemPath, Economy.SupplyGroup>
local neighborhoodCache = {}

-- cache of station groups which participate in local supply trading
-- top-level index is path to system, second-level index is body path
---@type table<SystemPath, table<SystemPath, Economy.SupplyGroup>>
local supplyGroupCache = utils.automagic()

-- persistent cache of station economies
---@type table<SystemPath, Economy.StationMarket>
local persistentMarket = {}

local sEmptyMarket = {
	supply = {},
	demand = {},
	stock = {},
	history = {},
	updated = 0,
}

local populationDef = Industry.NewIndustry("population", Json.LoadJson('economy/population.json'))

--=============================================================================

-- Convert a commodity flow magnitude to a quantity in cargo units
local function flowToAmount(mag)
	return math.ceil(math.exp(2 + mag))
end

-- Convert the inverse (flow magnitude) of a commodity amount in cargo units
local function invFlow(amount)
	return amount > 0 and math.log(amount) - 2 or 0
end

Economy.FlowToAmount = flowToAmount

--=============================================================================
-- Station Economies
--=============================================================================

-- Function: PrecacheSystem
--
-- Generate economies and cached values for the spaceports in the given star system.
-- Use <ReleaseCachedSystem> to retire cached values.
--
---@param system StarSystem
function Economy.PrecacheSystem(system)
	local _end = utils.profile("Economy.PrecacheSystem(\"{name}\")" % system)

	local starports = utils.map_array(system:GetStationPaths(), function(path) return path:GetSystemBody() end)

	for _, sbody in ipairs(starports) do
		Economy.GenerateStationEconomy(sbody)
	end

	for _, sbody in ipairs(starports) do
		if not neighborhoodCache[sbody.path] then
			Economy.GenerateSupplyNeighborhood(sbody)
		end
	end

	_end()
end

-- Function: ReleaseCachedSystem
--
-- Remove the starports of the given system from the economy's caches.
-- Should be used when leaving a system or closing a view of a remote system.
--
---@param system StarSystem
function Economy.ReleaseCachedSystem(system)

	local paths = system:GetStationPaths()

	for _, path in ipairs(paths) do
		economyCache[path] = nil
		neighborhoodCache[path] = nil
	end

	supplyGroupCache[system.path] = nil

end

-- Function: Economy.GetStationEconomy
--
-- Return or create the deterministic EconomyDef for the given station.
--
-- Status: Experimental
---@param sbody SystemBody
---@return Economy.EconomyDef
function Economy.GetStationEconomy(sbody)
	if economyCache[sbody.path] then
		return economyCache[sbody.path]
	end

	return Economy.GenerateStationEconomy(sbody)
end

-- Function: GenerateStationEconomy
--
-- Compute the list of station industries for the given station SystemBody,
-- creating and caching the EconomyDef for that station.
---@param sbody SystemBody
---@return Economy.EconomyDef
function Economy.GenerateStationEconomy(sbody)
	-- local _p = utils.profile("Economy.GenerateStationEconomy(\"{name}\")" % sbody)
	assert(sbody.superType == "STARPORT")

	local conditions = Economy.ComputeStationConditions(sbody)
	local size = Economy.GetStationSizeClass(sbody)
	local rand = Rand.New(sbody.seed)

	-- Generate the supply/demand values for the station's population
	-- Each supply/demand number is scaled by the size class of the station.
	local supply, demand = Industry.ComputeModifiers(populationDef, conditions)
	local pop_scale = size - 1

	for comm, amt in pairs(supply) do
		supply[comm] = amt + pop_scale
	end

	for comm, amt in pairs(demand) do
		demand[comm] = amt + pop_scale
	end

	local economy = {}

	economy.tags = conditions
	economy.industries, economy.supply, economy.demand = Industry.GenerateIndustries(sbody, conditions, size, rand, supply, demand)

	economyCache[sbody.path] = economy

	-- _p()
	return economy
end

-- Function: ComputeStationConditions
--
-- Compute the list of condition tags affecting this particular station and
-- return it.
--
-- This function is not particularly fast and the result should be cached if
-- it is intended to be accessed multiple times.
---@param sbody SystemBody
---@return table<string, boolean> tags
function Economy.ComputeStationConditions(sbody)
	local conditions = Conditions.Evaluate(sbody.system)

	Conditions.Evaluate(sbody.parent, conditions)
	Conditions.Evaluate(sbody, conditions)

	return conditions
end

-- Function: GetStationSizeClass
--
-- Compute a station's "size class", AKA the logarithm of its population size.
---@param sbody SystemBody
function Economy.GetStationSizeClass(sbody)
	local popThousands = sbody.population * 1000000
	return math.max(1, math.ceil(math.log(popThousands, 10)))
end

-- Function: GetCommodityFlowParams
--
-- Returns the raw supply and demand flow values for a station's economy,
-- with no effects from transient events or supply neighborhood being applied.
--
---@param sbody SystemBody
---@param commodityId string
---@return number supplyFlow
---@return number demandFlow
function Economy.GetCommodityFlowParams(sbody, commodityId)
	local econ = Economy.GetStationEconomy(sbody)
	return econ.supply[commodityId] or 0, econ.demand[commodityId] or 0
end

-- Function: GetLocalSupply
--
-- Return the flow value of the given commodity present in the local "supply
-- neighborhood" for the given station.
--
---@param path SystemPath
---@param commodityId string
function Economy.GetLocalSupply(path, commodityId)
	if not neighborhoodCache[path] then
		return 0
	end

	return neighborhoodCache[path].supply[commodityId] or 0
end

-- Function: GenerateSupplyNeighborhood
--
-- Traverse the local environs of this system body to determine the "supply
-- neighborhood"; the local network of stations exporting goods to each other.
--
---@param sbody SystemBody
function Economy.GenerateSupplyNeighborhood(sbody)
	local supply = {}
	local starports = {}

	local group = {
		supply = supply,
		starports = starports
	}

	---@param sbody SystemBody
	local function traverse_econ(sbody)

		for _, child in ipairs(sbody.children) do

			if child.superType == "STARPORT" then

				table.insert(starports, child.path)

				local econ = Economy.GetStationEconomy(child)

				for id, produced in pairs(econ.supply) do

					if (econ.demand[id] or 0) <= produced then
						supply[id] = math.max(supply[id] or 0, produced)
					end

				end

				neighborhoodCache[child.path] = group

			elseif (child.apoapsis + child.periapsis) * 0.5 < AU * 0.01 then

				traverse_econ(child)

			end

		end

	end

	-- Start with this station's parent, and go up the tree until the distance to parent is > 0.01 AU
	local primary = assert(sbody.parent)

	while primary.parent and (primary.apoapsis + primary.periapsis) * 0.5 < AU * 0.01 do
		primary = assert(primary.parent)
	end

	group.primary = primary
	traverse_econ(primary)

	supplyGroupCache[sbody.system.path][primary.path] = group
end

-- Function: GetSupplyGroupForStation
--
-- Return the local supply neighborhood for the passed station.
-- Will return nil if the station hasn't been cached with <PrecacheSystem>.
function Economy.GetSupplyGroupForStation(sbody)
	return neighborhoodCache[sbody.path]
end

-- Function: GetSupplyGroups
--
-- Get the set of local "supply neighborhoods" for the passed system.
-- Each supply neighborhood is a group of starports under a primary SystemBody
-- which are within economic export range of each other.
--
-- Interplanetary or interstellar trade is likely to take place between
-- different supply groups, and this information can be used to determine
-- where start and end points are for trade networks.
--
-- Note that no values will be returned unless <PrecacheSystem> has been called
-- prior for the passed system.
function Economy.GetSupplyGroups(system)
	return supplyGroupCache[system.path]
end

--=============================================================================
-- Station Markets
--=============================================================================

-- Function: GetCommodityStockEquilibrium
--
-- Returns the average stock quantity of a commodity at this station under
-- equilibrium conditions. This should be used to compute the initial state
-- of a station, or otherwise determine the nominal condition of the station
-- in absence of any other factors.
--
-- For the current equilibrium stock quantity affected by perturbed supply
-- and demand values, see [GetCommodityStockTargetEquilibrium].
--
-- Parameters:
--
--    sbody - SystemBody, the station for which to compute the equilibrium
--    commodityId - string, the id of the commodity in question
--
-- Returns:
--
--    equilibrium - number, total commodity stock expected to be present at this station
--
---@param sbody SystemBody
---@param commodityId string
function Economy.GetCommodityStockEquilibrium(sbody, commodityId)
	local econ = Economy.GetStationEconomy(sbody)

	-- Supply/demand flow values
	local fS = econ.supply[commodityId] or 0
	local fD = econ.demand[commodityId] or 0

	return Economy.GetCommodityStockTargetEquilibrium(sbody, commodityId, flowToAmount(fS), flowToAmount(fD))
end

-- Function: GetCommodityStockTargetEquilibrium
--
-- Returns the equilibrium stocking of a commodity at this station, given the
-- current supply and demand for the commodity.
--
-- This computes the expected amount of commodity stock available from station
-- arbitrage brokers under the given conditions, assuming the market is at
-- equilibrium (i.e. has not been perturbed by player activity).
--
-- The result should be modified by a small random factor to determine actual
-- commodity stock numbers to avoid visible "sameness".
--
-- Parameters:
--
--    sbody - SystemBody, the station for which to compute the equilibrium
--    commodityId - string, the id of the commodity in question
--
-- Returns:
--
--    equilibrium - number, total commodity stock expected to be present at this station
--
function Economy.GetCommodityStockTargetEquilibrium(sbody, commodityId, supply, demand)
	local import = flowToAmount(Economy.GetLocalSupply(sbody, commodityId))
	-- The amount of imported goods is related to the demand at this station.
	-- With no demand, minimal quantities will be in stock.
	local import_avail = math.min(demand * math.exp(-1), import)

	-- The 'median' value is the amount of commodities available on the market when the station production
	-- is equal to its consumption rate.
	-- The station arbitrage market opportunistically imports goods to cover temporary production shortfalls,
	-- and thus a minimal amount of goods are available.
	local median = supply * 0.33 + import_avail

	if supply >= demand then
		-- If we produce much more than we consume, the full production is available on the market with no imports.
		-- If we produce in equilibrium with the demand, a portion of our production plus imports is available on the market.
		return math.lerp(supply - demand + import_avail, median, demand / supply)
	else
		-- If we consume more than we locally produce, the station arbitrage market buys in advance. The amount of commodities
		-- available from the arbitrage market reduces as the ratio of production to consumption decreases.
		local avail = (supply + demand + import_avail) * math.exp(-1)

		return math.lerp(avail, median, supply / demand)
	end
end

-- Function: GetCommodityPriceMod
--
-- Returns the instantaneous price modifier for the given commodity at this station.
-- This price modifier is affected by the player's trade history, as well as local
-- events near the station which affect the supply / demand / import transient modifiers.
--
-- Parameters:
--
--    sbody - SystemBody, the station to compute the price modifier for.
--    commodityId - string, the id of the commodity to compute the price modifier for.
--
-- Returns:
--
--    pricemod - number, percentage modification of the commodity price.
--
---@param path SystemPath
---@param commodityId string
---@param market Economy.StationMarket?
function Economy.GetCommodityPriceMod(path, commodityId, market)
	local local_supply = Economy.GetLocalSupply(path, commodityId)
	market = market or persistentMarket[path] or sEmptyMarket

	-- TODO: allow affecting demand values with transient events

	-- NOTE: this is not the current commodity stock value, but rather the persistent
	-- supply adjusted for the effects of repeated purchases and local events.
	-- The effective supply value can go negative if the player has purchased enough
	-- commodities from the local market in a short enough period of time.
	local supply = (market.supply[commodityId] or 0) + (market.history[commodityId] or 0)
	local demand = (market.demand[commodityId] or 0)

	-- TODO: approximation of instantaneous import satisfaction based on import capacity
	local imported = math.max(demand - supply, 0)
	local nearby_import = math.min(imported, flowToAmount(local_supply))

	local est_max_flow = Economy.GetMaxFlowForPrice(Commodities[commodityId].price)

	-- TODO: handle surplus (greater supply than interstellar export capacity)
	-- TODO: handle deficit (greater demand than interstellar import capacity)

	if supply >= demand then
		return math.clamp(invFlow(supply - demand) / est_max_flow, 0, 2) * -kMaxCommodityVariance
	else
		local pricemod = math.clamp(invFlow(imported) / est_max_flow, 0, 2) * kMaxCommodityVariance
		-- scale price based on proportion of imported goods available nearby; 1x at fully imported,
		-- 0.5x when all demand is sourced from nearby starports
		return pricemod * (1.0 - nearby_import / (imported * 2))
	end
end

-- Function: GetMaxFlowForPrice
--
-- Returns the maximum "estimated flow" for a commodity based on its price.
-- This function is used to relate commodity stock/demand values to price modifiers.
--
-- Parameters:
--
--    price - number, price of the commodity to estimate.
--
-- Returns:
--
--    flow - number, estimated maximim flow for the commodity
--
---@param price number
function Economy.GetMaxFlowForPrice(price)
	-- This is a set of magic numbers which relates (most) of our commodity prices
	-- to expected flow values.
	return 30 / math.log(math.abs(price) + 7.38)
end

-- Function: CreateCommodityMarket
--
-- Initialize the starting state of the commodity market for the given
-- {station, commodity} pair. This function returns a deterministic sample
-- of the stochastic equilibrium of the market, and is suitable for seeding
-- market state in distant systems.
--
-- The current time value should be passed when seeding markets for the current
-- system, so as to produce some deterministic variantion on subsequent visits.
--
---@param sbody SystemBody
---@param commodityId string
---@param market Economy.StationMarket for
---@param time number?
function Economy.CreateCommodityMarket(sbody, commodityId, market, time)
	local fS, fD = Economy.GetCommodityFlowParams(sbody, commodityId)
	local equilibrium = Economy.GetCommodityStockEquilibrium(sbody, commodityId)
	local time_id = time % (kSupplyDemandScale * kMarketUpdateTick)
	local rand_init = Rand.New("{}-state-{}-{}" % { sbody.seed, commodityId, time_id })

	market.supply[commodityId] = math.ceil(flowToAmount(fS) * rand_init:Normal(1.0, kSupplyDemandDeviation))
	market.demand[commodityId] = math.ceil(flowToAmount(fD) * rand_init:Normal(1.0, kSupplyDemandDeviation))

	market.stock[commodityId] = math.ceil(equilibrium * rand_init:Normal(1.0, kStockDeviation))
end

-- https://www.gameaipro.com/GameAIPro3/GameAIPro3_Chapter02_Creating_the_Past_Present_and_Future_with_Random_Walks.pdf
---@param rand Rand
---@param x0 number Initial value to extrapolate from
---@param dT number Elapsed time to extrapolate over
---@param stddev number Standard deviation of the random walk, controlling how far it will deviate from x0 over unit time
---@param mean number Mean value to bound the result around
---@param bound_dev number Standard deviation of the result relative to the mean
local function extrapolate_bounded_walk(rand, x0, dT, stddev, mean, bound_dev)
	-- variance of the random walk
	local v = stddev^2
	-- variance of the bounding normal distribution
	local vBound = bound_dev^2

	-- The result will be x0 +-1.96*stddev over dT=1, with a probability of ~95%
	local term_a = (x0 / (dT * v) + mean / vBound)
	local term_b = (1 / (dT * v) + 1 / vBound)

	return rand:Normal(term_a / term_b, math.sqrt(1 / term_b))
end

-- Extrapolate a bounded walk for commodity supply/demand values based on the commodity flow
local function extrapolate_bounded_sd_walk(rand, x0, dT, flow)
	local mean = flowToAmount(flow)
	local stddev = mean * kSupplyDemandDeviation
	local bound = mean * kSupplyDemandBound

	return math.ceil(extrapolate_bounded_walk(rand, x0, dT, stddev, mean, bound))
end

-- Function: UpdateStationMarket
--
-- Perform a market update, extrapolating the supply, demand, and stock values
-- based on the time since the last market update.
--
-- This function also handles decay of the history register which records the
-- player's effects on the market.
--
-- TODO: this function does not yet take into account any sort of "local event"
-- system which would affect the supply/demand/stock values.
---@param sbody SystemBody
---@param market Economy.StationMarket
function Economy.UpdateStationMarket(sbody, market)

	if not market then return end

	local lastStockUpdate = market.updated
	local timeSinceUpdate = Game.time - lastStockUpdate
	if timeSinceUpdate <= kMarketUpdateTick then return end

	-- make sure the next tick happens at the correct time
	local numTicks = math.floor(timeSinceUpdate / kMarketUpdateTick)
	local delta = kMarketUpdateTick * numTicks
	market.updated = lastStockUpdate + delta

	-- use a unique random function to tally up restocks
	-- this ensures that each restock uses a different random number based on game start time
	local randRestock = Rand.New(sbody.seed .. '-stockMarketUpdate-' .. math.floor(lastStockUpdate))

	for _, id in ipairs(CommodityList) do

		-- Delta for supply/demand random walk
		local dT = delta / (kSupplyDemandScale * kMarketUpdateTick)
		-- Delta for stock quantity random walk
		local dR = delta / (kStockUpdateScale * kMarketUpdateTick)

		local fS, fD = Economy.GetCommodityFlowParams(sbody, id)
		local eStock = Economy.GetCommodityStockEquilibrium(sbody, id)

		-- Go on a random walk to determine what the local supply/demand values are at the station
		-- TODO: allow events to influence the outcomes of these walks
		market.supply[id] = extrapolate_bounded_sd_walk(randRestock, market.supply[id], dT, fS)
		market.demand[id] = extrapolate_bounded_sd_walk(randRestock, market.demand[id], dT, fD)

		-- Instantaneous stock quantity equilibrium based on current supply/demand value
		local eiStock = Economy.GetCommodityStockTargetEquilibrium(sbody, id, market.supply[id], market.demand[id])
		-- sigma stock, std deviation of stock value wander step size
		local sStock = eStock * kStockDeviation
		-- sigma stock bounds, std deviation of instantaneous stock quantity
		local siStock = eiStock * kStockDeviation

		-- Update the current commodity stock value, tending to stay within +- 2*siStock of the current stock equilibrium
		-- based on the instantaneous supply/demand of the commodity at this station.
		-- This random walk uses the current commodity stock value and handles "decay" of commodities sold/purchased by the player.
		-- TODO: should the history value have a persistent effect on the equilibrium available stock?
		market.stock[id] = math.ceil(extrapolate_bounded_walk(randRestock, market.stock[id], dR, sStock, eiStock, siStock))

		if market.history[id] then

			local history = math.abs(market.history[id])

			-- This step causes the player's effect on the persistent market to decay over time.
			-- We do this by sampling a normal distribution, with a std. deviation proportional to the std. deviation of the "restock" step.
			-- The last scalar causes large history magnitudes to decay more quickly than small history magnitudes.
			local variance = sStock^2 * math.exp(-1) * math.max(history / (eStock * 4), 1)^2

			history = history - math.abs(randRestock:Normal(0, math.sqrt(dR * variance)))

			-- Clamp the walk to prevent it crossing below 0
			market.history[id] = history > 0 and math.sign(market.history[id]) * history or nil

		end

	end

end

-- Function: CreateStationMarket
--
-- Initialize a station market for the given space station.
-- By default, the returned market will not be cached anywhere and will not
-- persist. Use InitPersistentMarket instead.
---@param sbody SystemBody
---@param time number?
---@return Economy.StationMarket
function Economy.CreateStationMarket(sbody, time)
	time = time or 0

	---@type Economy.StationMarket
	local market = {
		supply = {},
		demand = {},
		stock = {},
		history = {},
		updated = Game.time
	}

	for id, _ in pairs(Commodities) do
		Economy.CreateCommodityMarket(sbody, id, market, time)
	end

	return market
end

---@param path SystemPath
function Economy.GetStationMarket(path)
	return persistentMarket[path] or sEmptyMarket
end

-- Function: InitPersistentMarket
--
-- Initialize and return a persistent market entry for the given system body.
-- If a persistent market is already cached, return the cached market instead.
---@param sbody SystemBody
---@return Economy.StationMarket
function Economy.InitPersistentMarket(sbody)
	if not persistentMarket[sbody.path] then
		persistentMarket[sbody.path] = Economy.CreateStationMarket(sbody, Game.time)
	end

	return persistentMarket[sbody.path]
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

-- Function: GetMarketPrice
--
-- Return a modified price according to the given price modifier factor
function Economy.GetMarketPrice(price, pricemod)
	return price * (1 + pricemod * 0.01)
end

function Economy.OnGameEnd()
	economyCache = {}
	neighborhoodCache = {}
	supplyGroupCache = utils.automagic()

	persistentMarket = {}
end

Serializer:Register("Economy",
	function()
		---@type table<SystemPath, Economy.StationMarket>
		local market = utils.filter_table(persistentMarket, function(path, mkt)
			-- Any market which would be observable after a load must be persisted in the savefile.
			-- This is all markets in the current system, and any other persistent markets which
			-- the player has acted upon sufficiently recently that the market state is still perturbed.
			return next(mkt.history) ~= nil or Game.system ~= nil and Game.system.path == path:SystemOnly()
		end)

		-- To save space in the savefile, we convert market data from key-value tables to arrays of values
		-- using a shared commodity index across all markets in the save file.
		-- This eliminates N copies of commodity names, and allows the code to take advantage of a potential
		-- future optimization to compactly store array-tables.
		local commodity_indices = utils.build_array(utils.keys(Commodities))
		table.sort(commodity_indices)

		local map_commodities = function(t)
			local out = {}

			for i = 1, #commodity_indices do
				out[i] = t[commodity_indices[i]]
			end

			return out
		end

		-- Convert market table to arrays of values rather than string tables.
		market = utils.map_table(market, function(k, mkt)
			local new_mkt = {}

			new_mkt.history = map_commodities(mkt.history)
			new_mkt.supply = map_commodities(mkt.supply)
			new_mkt.demand = map_commodities(mkt.demand)
			new_mkt.stock = map_commodities(mkt.stock)
			new_mkt.updated = mkt.updated

			return k, new_mkt
		end)

		return {
			commodities = commodity_indices,
			market = market
		}
	end,
	function(data)
		local commodity_cache = data.commodities

		local map_commodities = function(t)
			local out = {}

			for i, v in pairs(t) do
				out[commodity_cache[i]] = v
			end

			return out
		end

		-- Convert condensed array tables to maps of values
		persistentMarket = utils.map_table(data.market or {}, function(k, mkt)
			local new_mkt = {}

			new_mkt.history = map_commodities(mkt.history)
			new_mkt.supply = map_commodities(mkt.supply)
			new_mkt.demand = map_commodities(mkt.demand)
			new_mkt.stock = map_commodities(mkt.stock)
			new_mkt.updated = mkt.updated

			return k, new_mkt
		end)
	end
)

return Economy
