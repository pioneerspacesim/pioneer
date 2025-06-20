-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Rand = require 'Rand'
local Economy = require 'Economy'
local Commodities = require 'Commodities'

local Industry   = require 'Economy.Industry'
local Conditions = require 'Economy.Conditions'

local utils = require 'utils'

local ui = require 'pigui'
local colors = ui.theme.colors
local icons = ui.theme.icons

local debugView = require 'pigui.views.debug'

local function tsub(a, b)
	for k, v in pairs(b) do
		a[k] = (a[k] or 0) - v
	end

	return a
end

local commodities = utils.filter_table(Commodities, function(k, v) return v.purchasable == true end)
local comm_list = utils.build_array(utils.keys(commodities))

table.sort(comm_list)

---@param sbody SystemBody
local function drawIndustries(sbody, economy)
	ui.text("Population (thousands): " .. sbody.population * 1000000)
	ui.text("Size class: " .. Economy.GetStationSizeClass(sbody))

	for _, id in pairs(economy.industries) do
		ui.text(id)
	end

	ui.spacing()

	if ui.collapsingHeader("Industry Stats") then

		ui.separatorText("Production:")

		for k, v in pairs(economy.supply) do
			ui.text(k .. " => " .. v)
		end

		ui.spacing()

		ui.separatorText("Demand:")

		for k, v in pairs(economy.demand) do
			ui.text(k .. " => " .. v)
		end

		ui.spacing()

		ui.separatorText("Aggregate:")

		local aggregate = tsub(table.copy(economy.supply), economy.demand)

		for k, v in pairs(aggregate) do
			ui.text(k .. " => " .. v)
		end
	end

	if ui.collapsingHeader("Economy Stats") then

		if ui.beginTable("##CommodityDebug", 8) then
			ui.tableSetupColumn("Name")
			ui.tableSetupColumn("fS", "WidthFixed")
			ui.tableSetupColumn("fD", "WidthFixed")
			ui.tableSetupColumn("lS", "WidthFixed")
			ui.tableSetupColumn("Stock", "WidthFixed")
			ui.tableSetupColumn("fMax", "WidthFixed")
			ui.tableSetupColumn("Price", "WidthFixed")
			ui.tableSetupColumn("Mod", "WidthFixed")

			ui.tableHeadersRow()

			for _, id in ipairs(comm_list) do
				local comm = Commodities[id]

				ui.tableNextRow()

				ui.tableNextColumn()
				ui.text(comm:GetName())

				-- local rand = Rand.New("{}-stock-{}" % { sbody.seed, id })
				local s, d = Economy.GetCommodityFlowParams(sbody, id)

				ui.tableNextColumn()
				ui.textColored(s > 0 and colors.econMinorExport or colors.fontDim, "+" .. ui.Format.Number(s, 0))

				ui.tableNextColumn()
				ui.textColored(d > 0 and colors.econMinorImport or colors.fontDim, "-" .. ui.Format.Number(d, 0))

				local ls = Economy.GetLocalSupply(sbody, id)

				ui.tableNextColumn()
				ui.textColored(ls > s and colors.econMinorExport or colors.fontDim, "+" .. ui.Format.Number(ls, 0))

				local stock = Economy.GetCommodityStockEquilibrium(sbody, id)

				ui.tableNextColumn()
				ui.text(ui.Format.Number(stock, 0))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(Economy.GetMaxFlowForPrice(comm.price), 1))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(comm.price, 0))

				local pricemod = Economy.GetCommodityPriceMod(sbody, id) or 0

				ui.tableNextColumn()
				ui.text(ui.Format.Number(pricemod, 0))

			end

			ui.endTable()
		end
	end

end

---@param sbody SystemBody
local function drawConditions(sbody)

	ui.separatorText("System Conditions")

	for cond in pairs(Conditions.Evaluate(sbody.system)) do
		ui.text(cond)
	end

	ui.separatorText("Planet Conditions")

	for cond in pairs(Conditions.Evaluate(sbody.superType == "STARPORT" and sbody.parent or sbody)) do
		ui.text(cond)
	end

	if sbody.superType == "STARPORT" then

		ui.separatorText("Starport Conditions")

		for cond in pairs(Conditions.Evaluate(sbody)) do
			ui.text(cond)
		end

	end

end

debugView.registerTab("Loader", {
	label = "Economy/Industry Debug",
	icon = icons.spacestation,
	show = function() return Game.player end,
	draw = function(self)
		local body = Game.player:GetNavTarget()
		if not body then return end

		local sbody = body:GetSystemBody()
		if not sbody then return end

		drawConditions(sbody)

		if not body:IsStation() then
			return
		end

		local econ = Economy.GetStationEconomy2(sbody)

		ui.spacing()
		ui.separatorText("Starport Industries")

		drawIndustries(sbody, econ)
	end
})

require 'Event'.Register("onGameStart", function() Economy.PrecacheSystem(Game.system) end)
