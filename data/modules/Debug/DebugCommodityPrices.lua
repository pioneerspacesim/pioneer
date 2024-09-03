-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
-- impaktor's script for monitoring commodity prices

local Game = require "Game"
local ui = require 'pigui'
local Economy = require 'Economy'
local debugView = require 'pigui.views.debug'
local Format = require 'Format'
local Commodities = require 'Commodities'

-- (local) Global options
local radius = 20
local N = 0
local commodities = nil
local include_illegal = false
local include_only_purchasable = true

-- Create class to document our commodity
local Commodity = {}
Commodity.__index = Commodity

function Commodity.new (o, name, id_name, baseprice)
	-- o = o or {}
	local self = setmetatable({}, Commodity)
	-- self.__index = self
	self.name = name			-- name of commodity
	self.id_name = id_name		-- key of commodity
	self.max = 0				-- max price for this commodity
	self.min = 1e9				-- min price for this commodity
	self.prices = {}			-- table of every price for this commodity
	self.maxsys = nil			-- system path for system with max price
	self.minsys = nil			-- system path for system with min price
	self.baseprice = baseprice or nil  -- baseprice for this commodity
	self.price_bins = nil
	return self
end

function Commodity:add_price(price, sys)
	table.insert(self.prices, price)
	self.price_bins = nil

	-- Some prices are negative, and things get wonky
	local mod = price < 0 and -1 or 1

	-- update min price and minsys if new "record" low price seen
	if mod * price < self.min then
		self.min = price
		self.minsys = sys
	end

	-- update max price and maxsys if new "record" high price seen
	if mod * price > self.max then
		self.max = price
		self.maxsys = sys
	end
end

-- compute average price seen
function Commodity:mean()
	local sum = 0
	for _, v in pairs(self.prices) do
		sum = sum + v
	end

	return sum / #self.prices
end

function Commodity:std()
	local std = 0
	local mean = self:mean()
	for _, v in pairs(self.prices) do
		std = std + (v - mean)^2
	end
	return math.sqrt(std / #self.prices)
end

-- compute media price seen
function Commodity:median()
	table.sort(self.prices)
	local middle = math.floor(#self.prices / 2)

	-- if lenght == 1, lua not zero indexed, happens e.g. if filtered
	-- out illegal goods, so commodity only legal once.
	middle = math.max(1, middle)

	return self.prices[middle]
end

-- make data suitable for histogram
function Commodity:get_bins(n_bins)
	if self.price_bins then
		return self.price_bins
	end
	-- sort our prices (done in place)
	table.sort(self.prices)

	local n = n_bins or math.sqrt(#self.prices)

	local width = (self.max - self.min) / n
	local xaxis = {}
	for i = 1, n+1 do
		table.insert(xaxis, self.min+width*i)
	end
	-- where we sum up each column:
	self.price_bins = {}
	for i=1,n do
		table.insert(self.price_bins, 0)
	end

	for _, val in pairs(self.prices) do
		for i=1,#self.price_bins do
			if xaxis[i] <= val and val < xaxis[i+1] then
				self.price_bins[i] = self.price_bins[i] + 1
			end
		end
	end

	-- https://rosettacode.org/wiki/Statistics/Normal_distribution
	return self.price_bins
end


local build_nearby_systems = function (dist, display)
	local max_dist = dist or 30
	local display = display or false

	local nearbysystems = Game.system:GetNearbySystems(max_dist, function (s) return #s:GetStationPaths() > 0 end)

	if display then
		for _, sys in pairs(nearbysystems) do
			ui.text(sys.name)
			ui.sameLine()
			ui.text(sys:DistanceTo(Game.system))
		end
	end
	return nearbysystems
end


local getprice = function (equip, system)
	-- Hack: taken from libs/SpaceStation.lua, to check without needing a "station" object
	return equip.price * ((100 + system:GetCommodityBasePriceAlterations(equip.name)) / 100.0)
end



-- scan all systems within radius dist from current position
local scan_systems = function(dist)

	local nearby_systems = build_nearby_systems(dist, false)

	-- table of Commodity objects
	local commodities = {}

	-- for each system
	for idx, sys in pairs(nearby_systems) do
		-- for each commodity
		for key, comm in pairs(Commodities) do
			-- table.sort(comm, function (a,b) return a:GetName() > b:GetName() end)

			-- include goods if legal, or if we've chosen to include it
			local include = include_illegal or sys:IsCommodityLegal(comm.name)

			if include_only_purchasable then
				include = include and comm.purchasable
			end

			local name = comm:GetName()
			local price = getprice(comm, sys)

			if include and not commodities[name] then
				commodities[name] = Commodity:new(name, comm.name, comm.price)
			end

			-- if interesting, get name, and check price
			if include then
				commodities[name]:add_price(price, sys)
			end
		end
	end
	return commodities, #nearby_systems
end


local clicked = nil

-- Generates text of type: <name> <min price> <max price> <average price>
local function show_price_table(commodities)
	if not commodities then
		return
	end
	local alpha_w = 1.2 * ui.calcTextSize("Industrial machines").x
	local num_w = 1.2 * ui.calcTextSize("$100.00").x

	-- HEADLINE
	ui.text("NAME")
	ui.sameLine(alpha_w)

	ui.text("MIN")
	ui.sameLine(alpha_w + num_w)

	ui.text("MEAN")
	ui.sameLine(alpha_w + num_w*2)

	ui.text("MAX")
	ui.sameLine(alpha_w + num_w*3)

	ui.text("DIFF")
	ui.sameLine(alpha_w + num_w*4)

	ui.text("REL")

	local count = 0
	for key, val in pairs(commodities) do
		if ui.selectable(key, key == count) then
			print("Clicked:", key)
			clicked = key
		end
		ui.sameLine(alpha_w)
		ui.text(Format.Money(val.min))

		ui.sameLine(alpha_w + num_w)
		ui.text(Format.Money(val:mean()))

		ui.sameLine(alpha_w + num_w*2)
		ui.text(Format.Money(val.max))

		ui.sameLine(alpha_w + num_w*3)
		ui.text(Format.Money(val.max - val.min))

		ui.sameLine(alpha_w + num_w*4)
		ui.text(string.format("%.2f", 100*(val.max - val.min)/val.min) .. "%")
		count = count + 1
	end
end

local function show_system_info(min, sys, str)
	if not sys then
		return
	end
	ui.text(str .. " price:\t ".. Format.Money(min) .. " in system: " .. sys.name)
	local dist = string.format("%.2f", sys:DistanceTo(Game.system))
	ui.text("Distance:\t " .. dist .. " ly")
	ui.text("Faction:\t " .. sys.faction.name)
	ui.text("Government:\t " .. string.lower(sys.govtype):gsub("^%l", string.upper)) -- PolitGovType
	ui.spacing()

	-- Add from C++ side, E.g:
	-- System type: Mining colony
	-- Government type: Earth Federation Democrac
	-- Economy type: capitalist
end

local function show_details_on_commodity(commodities, selected)
	if not selected then
		return
	end
	ui.text(string.upper(selected))
	local c = commodities[selected]

	-- If previous click was non-purchasable item followed by
	-- filtering them out, 'clicked' value will no longer be in
	-- 'commodities'
	if not c then
		return
	end

	local purchasable = Commodities[c.id_name].purchasable

	ui.text("Current system:\t" .. Game.system.name)
	ui.text("Tag:\t" .. c.id_name)
	ui.text("Purchasable:\t" .. tostring(purchasable))
	ui.text("Baseprice:\t" .. Format.Money(c.baseprice))
	if not purchasable then return purchasable end
	ui.text("Median price:\t" .. Format.Money(c:median()))
	ui.text("Average price:\t" .. Format.Money(c:mean()))
	ui.text("Standard deviation:\t" .. string.format("%.2f", c:std()))

	ui.spacing()

	if ui.collapsingHeader("Lowest", {"DefaultOpen"}) then
		show_system_info(c.min, c.minsys, "Lowest")
	end
	if ui.collapsingHeader("Highest", {"DefaultOpen"}) then
		show_system_info(c.max, c.maxsys, "Highest")
	end

	if ui.collapsingHeader("Difference", {"DefaultOpen"}) then
		local diff = c.max - c.min
		ui.text("Absolute price difference/spread:\t " .. Format.Money(diff))
		ui.text("Relative price difference:\t " .. string.format("%.2f", 100*diff/c.min) .. " %")
		local dist = string.format("%.2f", c.minsys:DistanceTo(c.maxsys))
		ui.text("Distance between mininum and maximum systems:\t " .. dist .. " ly")

		local data = c:get_bins()
		local y_max = math.max(table.unpack(data))
		-- ui.dummy(Vector2(0,50))

		ui.plotHistogram("##price distribution", data, #data, 0, "", 0, y_max, Vector2(400, 100))
		ui.text("Min: " .. Format.Money(c.min))
		ui.text("Max: " .. Format.Money(c.max))

		ui.spacing()
	end

	return purchasable
end


local function station_economy(commodities, clicked, station)
	if not clicked or not station then
		return
	end
	-- random selection, element 4 out of my ass
	local cargo_item = commodities[clicked]
	cargo_item = Commodities[cargo_item.id_name]

	local flow, affinity = Economy.GetStationFlowParams(station, cargo_item)

	local equilibrium_stock, equilibrium_demand = Economy.GetCommodityStockFromFlow(cargo_item, flow, affinity)

	local price = station:GetCommodityPrice(cargo_item)
	local stock = station:GetCommodityStock(cargo_item)
	local demand = station:GetCommodityDemand(cargo_item)

	ui.text("At station: " .. station.label)
	ui.text("Local price " .. price)
	ui.text("Local stock " .. stock)
	ui.text("Local demand: " .. demand)
	ui.text("Local flow: " .. flow)
	ui.text("Local affinity: " .. affinity)
	ui.text("equilibrium_stock: " .. equilibrium_stock)
	ui.text("equilibrium_demand: " .. equilibrium_demand)

	-- Just stub for experiment with changing market prices
	local mod = 2
	if ui.button("Double", Vector2(100, 0)) then
		station:SetCommodityPrice(cargo_item, mod*price)
		station:SetCommodityStock(cargo_item, mod*stock, mod*demand)
	end
end



local function main()

	-- HEADER
	_, include_illegal = ui.checkbox("Include Illegal goods", include_illegal)
	_, include_only_purchasable = ui.checkbox("Include only purchasable goods", include_only_purchasable)

	if ui.button("scan", Vector2(0,0)) then
		commodities, N = scan_systems(radius)
	end
	ui.sameLine()
	ui.pushItemWidth(ui.getWindowSize().x/8)
	radius = ui.sliderInt("Radius", radius, 1, 100, "%d")

	ui.text("Systems scanned: ")
	ui.sameLine()
	ui.text(N or 0)
	ui.separator()

	-- COLUMN 1
	ui.columns(2, "commodity", true)
	ui.setColumnWidth(0, ui.getWindowSize().x/2)
	show_price_table(commodities)

	-- COLUMN 2
	ui.nextColumn()
	local purchasable = show_details_on_commodity(commodities, clicked)

	local station = Game.player:GetDockedWith() or false
	if station and purchasable and ui.collapsingHeader("Station economy", {"DefaultOpen"}) then
		station_economy(commodities, clicked, station)
	end
	ui.columns(1)
end


debugView.registerTab("debug-commodity-price", {
	icon = ui.theme.icons.money,
	label = "Commodities",
	show = function() return Game.player ~= nil end,
	draw = main
})
