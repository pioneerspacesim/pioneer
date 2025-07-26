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

local AU = 149598000000

---@class Economy
local Economy = package.core['Economy']

---@class Economy.StationMarket
---@field commodities table<string, table>
---@field lastStockUpdate number

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

local Economies = {}
local Commodities = Economy.GetCommodities()

local CommodityList = utils.build_array(utils.keys(Commodities))
table.sort(CommodityList)

-- Create a stable iteration order for economies to avoid non-deterministic results
for _, econ in pairs(Economy.GetEconomies()) do
	table.insert(Economies, econ)
end

table.sort(Economies, function(a, b) return a.id < b.id end)

-- Percentage modifier applied to buying/selling commodities
-- Prevents buying a commodity at a station and immediately reselling it
Economy.TradeFeeSplit = 2

-- Total trade fee percentage applied to a buy->sell transaction
Economy.TotalTradeFees = 2 * Economy.TradeFeeSplit

-- Scalar multiplier applied when reselling "used" equipment items back onto the market
Economy.BaseResellPriceModifier = 0.8

-- stationMarket is a persistent table of stock information for every station
-- the player has visited in their journey
local stationMarket = {}

---@class Economy.EconomyDef
---@field industries string[]
---@field tags table<string, boolean>
---@field supply table<string, integer>
---@field demand table<string, integer>

---@class Economy.SupplyGroup
---@field primary SystemPath
---@field supply table<string, integer>
---@field starports SystemPath[]

---@class Economy.StationMarket2
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

-- persistent cache of local modifiers to station trade
---@type table<SystemPath, Economy.StationMarket2>
local persistentMarket = {}

local sEmptyMarket = {
	supply = {},
	demand = {},
	stock = {},
	history = {},
	updated = 0,
}

local populationDef = Industry.NewIndustry("population", Json.LoadJson('economy/population.json'))

local affinityCache = {}

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
-- New Economy
--=============================================================================

-- TODO
-- Commodity prices are based on current distribution between demand/supply, which can be altered by selling to the market.
-- Consistently buying from the market can lower the effective supply value for a time (beyond transient restocks)
-- Don't keep a timestamp, instead have the "history" value decay at a rate 1/e of commodity restock
-- Remove demand limit at stations; selling goods increases transient supply > demand, which affects commodity prices.


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

-- Function: Economy.GetStationEconomy2
--
-- Return or create the deterministic EconomyDef for the given station.
--
-- Status: Experimental
---@param sbody SystemBody
---@return Economy.EconomyDef
function Economy.GetStationEconomy2(sbody)
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
function Economy.GetCommodityFlowParams(sbody, commodityId)
	local econ = Economy.GetStationEconomy2(sbody)
	return econ.supply[commodityId] or 0, econ.demand[commodityId] or 0
end

-- Function: GetLocalSupply
--
-- Return the flow value of the given commodity present in the local "supply
-- neighborhood" for the given station.
--
---@param sbody SystemBody
---@param commodityId string
function Economy.GetLocalSupply(sbody, commodityId)
	if not neighborhoodCache[sbody.path] then
		Economy.GenerateSupplyNeighborhood(sbody)
	end

	return neighborhoodCache[sbody.path].supply[commodityId] or 0
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

				local econ = Economy.GetStationEconomy2(child)

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
	local econ = Economy.GetStationEconomy2(sbody)

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
---@param sbody SystemBody
---@param commodityId string
---@param market Economy.StationMarket2?
function Economy.GetCommodityPriceMod(sbody, commodityId, market)
	local local_supply = Economy.GetLocalSupply(sbody, commodityId)
	local persist = market or persistentMarket[sbody.path] or sEmptyMarket

	-- TODO: allow affecting demand values with transient events

	-- NOTE: this is not the current commodity stock value, but rather the persistent
	-- supply adjusted for the effects of repeated purchases and local events.
	-- The effective supply value can go negative if the player has purchased enough
	-- commodities from the local market in a short enough period of time.
	local supply = (persist.supply[commodityId] or 0) + (persist.history[commodityId] or 0)
	local demand = (persist.demand[commodityId] or 0)

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
-- Initialize the commodity market for the given {station, commodity} pair.
-- This function generates deterministic values for the random market state at
-- the t0 point of the station's history.
---@param sbody SystemBody
---@param commodityId string
---@param market Economy.StationMarket2
function Economy.CreateCommodityMarket(sbody, commodityId, market)
	local fS, fD = Economy.GetCommodityFlowParams(sbody, commodityId)
	local equilibrium = Economy.GetCommodityStockEquilibrium(sbody, commodityId)
	local rand_init = Rand.New("{}-state-{}" % { sbody.seed, commodityId })

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

	return extrapolate_bounded_walk(rand, x0, dT, stddev, mean, bound)
end

-- Function: UpdateStationMarket2
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
---@param market Economy.StationMarket2?
function Economy.UpdateStationMarket2(sbody, market)
	market = market or persistentMarket[sbody.path]
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
		market.stock[id] = extrapolate_bounded_walk(randRestock, market.stock[id], dR, sStock, eiStock, siStock)

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

-- Function: CreateStationMarket2
--
-- Initialize a station market for the given space station.
function Economy.CreateStationMarket2(sbody)
	---@type Economy.StationMarket2
	local market = {
		supply = {},
		demand = {},
		stock = {},
		history = {},
		updated = Game.time
	}

	persistentMarket[sbody.path] = market

	for key, comm in pairs(Commodities) do
		Economy.CreateCommodityMarket(sbody, key, market)
	end
end

function Economy.GetStationMarket2(path)
	return persistentMarket[path] or sEmptyMarket
end

--=============================================================================
-- Old Economy
--=============================================================================

-- Function: GetStationEconomy
--
-- This function calculates the economy-type affinities of the given station
-- based on several body parameters. It's intended as a stub to generate
-- interesting and sometimes-useful results until a more complete and
-- parameter-rich implementation can be added to system generation directly.
--
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

-- Function: GetMarketPrice
--
-- Return a modified price according to the given price modifier factor
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
