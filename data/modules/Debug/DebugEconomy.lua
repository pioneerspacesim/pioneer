-- Copyright © 2008-2026 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Game = require 'Game'
local Economy = require 'Economy'
local Commodities = require 'Commodities'
local Timer       = require 'Timer'

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

local selected_commodity = 1

---@param sbody SystemBody
local function drawIndustries(sbody, economy)

	if ui.collapsingHeader("Station Stats") then
		ui.text("Population (thousands): " .. sbody.population * 1000000)
		ui.text("Size class: " .. Economy.GetStationSizeClass(sbody))
		ui.text("Station industries:")

		for _, id in pairs(economy.industries) do
			ui.text(id)
		end

		ui.spacing()
	end

	if ui.collapsingHeader("Commodity Stock Debug") then

		local changed, _i = ui.combo("Visualize Commodity Stock", selected_commodity - 1, comm_list)
		if changed then selected_commodity = _i + 1 end

		do
			local stock_data = {}
			local stock_id = comm_list[selected_commodity]

			local max_val = Economy.FlowToAmount(math.max(Economy.GetCommodityFlowParams(sbody, stock_id)))
			local local_supp = Economy.FlowToAmount(Economy.GetLocalSupply(sbody, stock_id))

			for i = 0, 50 do
				table.insert(stock_data, Economy.GetCommodityStockTargetEquilibrium(sbody, stock_id, max_val, max_val * (i/50)))
			end

			for i = 0, 50 do
				table.insert(stock_data, Economy.GetCommodityStockTargetEquilibrium(sbody, stock_id, max_val * (1 - i/50), max_val))
			end

			ui.textWrapped(stock_id .. " instantaneous stock values over max supply/demand of " .. max_val .. " with local supply " .. local_supp)
			ui.plotHistogram("##Stock Values", stock_data)
		end

		ui.spacing()

	end

	if ui.collapsingHeader("Market Debug") then

		local market = Economy.GetStationMarket(sbody.path)

		if ui.button("Buy All x100") then
			for _, id in pairs(comm_list) do
				local amt = math.min(market.stock[id], 100)

				market.stock[id] = market.stock[id] - amt
				market.history[id] = (market.history[id] or 0) - amt
			end
		end

		ui.sameLine()

		if ui.button("Sell All x100") then
			for _, id in pairs(comm_list) do
				local amt = 100

				market.stock[id] = market.stock[id] + amt
				market.history[id] = (market.history[id] or 0) + amt
			end
		end

		if ui.beginTable("##CommodityDebug", 11) then
			ui.tableSetupColumn("Name")
			ui.tableSetupColumn("fS", "WidthFixed")
			ui.tableSetupColumn("fD", "WidthFixed")
			ui.tableSetupColumn("lS", "WidthFixed")
			ui.tableSetupColumn("fMax", "WidthFixed")
			ui.tableSetupColumn("Price", "WidthFixed")
			ui.tableSetupColumn("Stock", "WidthFixed")
			ui.tableSetupColumn("Prod.", "WidthFixed")
			ui.tableSetupColumn("Demand", "WidthFixed")
			ui.tableSetupColumn("History", "WidthFixed")
			ui.tableSetupColumn("Mod", "WidthFixed")

			ui.tableHeadersRow()

			for _, id in ipairs(comm_list) do
				local comm = Commodities[id]

				ui.tableNextRow()

				ui.tableNextColumn()
				ui.text(comm:GetName())

				local s, d = Economy.GetCommodityFlowParams(sbody, id)
				local ls = Economy.GetLocalSupply(sbody.path, id)

				ui.tableNextColumn()
				ui.textColored(s > 0 and colors.econMinorExport or colors.fontDim, "+" .. ui.Format.Number(s, 0))

				ui.setItemTooltip("Median Supply: {}" % { Economy.FlowToAmount(s) })

				ui.tableNextColumn()
				ui.textColored(d > 0 and colors.econMinorImport or colors.fontDim, "-" .. ui.Format.Number(d, 0))

				ui.setItemTooltip("Median Demand: {}" % { Economy.FlowToAmount(d) })

				ui.tableNextColumn()
				ui.textColored(ls > s and colors.econMinorExport or colors.fontDim, "+" .. ui.Format.Number(ls, 0))

				ui.setItemTooltip("Median Local Supply: {}" % { Economy.FlowToAmount(ls) })

				-- local stock = Economy.GetCommodityStockEquilibrium(sbody, id)

				ui.tableNextColumn()
				ui.text(ui.Format.Number(Economy.GetMaxFlowForPrice(comm.price), 1))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(comm.price, 0))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(market.stock[id], 0))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(market.supply[id], 0))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(market.demand[id], 0))

				ui.tableNextColumn()
				ui.text(ui.Format.Number(market.history[id] or 0, 0))

				local pricemod = Economy.GetCommodityPriceMod(sbody.path, id, market) or 0

				ui.tableNextColumn()
				ui.text(ui.Format.Number(pricemod, 3))

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
		if not body then
			ui.text("Select a station or planet to inspect economy data.")

			return
		end

		local sbody = body:GetSystemBody()
		if not sbody then return end

		ui.withFont(ui.fonts.pionillium.heading, function()
			ui.spacing()
			ui.textAligned(sbody.name, 0.5)
			ui.spacing()
		end)

		if ui.collapsingHeader("Condition Tags") then
			drawConditions(sbody)
		end

		if not body:IsStation() then
			return
		end

		local econ = Economy.GetStationEconomy(sbody)

		ui.spacing()
		ui.separatorText("Starport Industries")

		drawIndustries(sbody, econ)
	end
})
