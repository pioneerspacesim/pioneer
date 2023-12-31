-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt
-- impaktor's script for monitoring commodity prices

local Game = require "Game"
local ui = require 'pigui'
local Engine = require "Engine"
local Rand = require "Rand"
local Format = require "Format"
local debugView = require 'pigui.views.debug'
local Format = require 'Format'
local Commodities = require 'Commodities'

-- (local) Global options
local radius = 20
local N = 0
local commodities = nil
local include_illegal, changed = false, false


-- Create class to document our commodity
local Commodity = {}
Commodity.__index = Commodity

function Commodity.new (o, name, baseprice)
	-- o = o or {}
	local self = setmetatable({}, Commodity)
	-- self.__index = self
	self.name = name		    -- name of commodity
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
	for k, v in pairs(self.prices) do
		sum = sum + v
	end
	return sum / #self.prices
end

function Commodity:std()
	local std = 0
	local mean = self:mean()
	for k, v in pairs(self.prices) do
		std = std + (v - mean)^2
	end
	return std / #self.prices
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

	for key, val in pairs(self.prices) do
		for i=1,#self.price_bins do
			if xaxis[i] <= val and val < xaxis[i+1] then
				self.price_bins[i] = self.price_bins[i] + 1
			end
		end
	end

	-- https://rosettacode.org/wiki/Statistics/Normal_distribution
	return self.price_bins
end



local findNearbyStations = function (station, minDist)
	local nearbystations = {}
	for _,s in ipairs(Game.system:GetStationPaths()) do
		if s ~= station.path then
			local dist = station:DistanceTo(Space.GetBody(s.bodyIndex))
			if dist >= minDist then
				table.insert(nearbystations, { s, dist })
			end
		end
	end
	return nearbystations
end


local build_nearby_systems = function (dist, display)
	local max_dist = dist or 30
	local display = display or false

	local nearbysystems = Game.system:GetNearbySystems(max_dist, function (s) return #s:GetStationPaths() > 0 end)

	if display then
		for key, sys in pairs(nearbysystems) do
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

			-- TODO: also include negative products, right?
			-- if comm.price > 0 then

				local name = comm:GetName()
				local price = getprice(comm, sys)

				if not commodities[name] then
					commodities[name] = Commodity:new(name, comm.price)
					print(name, comm.price)
				end

				-- if interesting, get name, and check price
				if include then
					commodities[name]:add_price(price, sys)
				end
			-- end
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
	ui.text("") 	-- PolitEcon

	-- Add from C++ side, E.g:
	-- System type: Mining colony
	-- Government type: Earth Federation Democrac
	-- Economy type: capitalist
	ui.text("")
end

local function show_details_on_commodity(commodities, clicked)
	if not clicked then
		return
	end
	ui.text(string.upper(clicked))
	local c = commodities[clicked]

	ui.text("Current system:\t " .. Game.system.name)
	ui.text("Baseprice:\t " .. Format.Money(c.baseprice))
	ui.text("Median price:\t " .. Format.Money(c:median()))
	ui.text("Average price:\t " .. Format.Money(c:mean()))
	ui.text("Standard deviation:\t " .. string.format("%.2f", c:std()))
	ui.text("")

	show_system_info(c.min, c.minsys, "Lowest")
	show_system_info(c.max, c.maxsys, "Highest")

	local diff = c.max - c.min
	ui.text("Absolute price difference/spread:\t " .. Format.Money(diff))
	ui.text("Relative price difference:\t " .. string.format("%.2f", 100*diff/c.min) .. " %")
	local dist = string.format("%.2f", c.minsys:DistanceTo(c.maxsys))
	ui.text("Distance between mininum and maximum systems:\t " .. dist .. " ly")

	local data = c:get_bins()
	local y_max = math.max(table.unpack(data))
	ui.dummy(Vector2(0,50))

	ui.plotHistogram("##price distribution", data, #data, 0, "", 0, y_max, Vector2(400, 100))
	ui.text("Min: " .. Format.Money(c.min))
	ui.text("Max: " .. Format.Money(c.max))
end


local function main()
	local station = Game.player:GetDockedWith()

	-- HEADER
	changed, include_illegal = ui.checkbox("Include Illegal goods", include_illegal)

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
	show_details_on_commodity(commodities, clicked)
	ui.columns(1)
end


-- debugView.registerTab('debug-commodity-prices', function()
--     if not ui.beginTabItem("CommodityPrices") then return end
-- 	main()
-- end)

debugView.registerTab("Commodity Price", function()
  if Game.player == nil then return end
  if ui.beginTabItem("Commodity Price") then
    main()
    ui.endTabItem()
  end
end)
