-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local vZero = Vector2(0, 0)

local defaultFuncs = {
	beforeItems = function(self)

	end,
	canDisplayItem = function(self, item, key)

	end,
	beforeRenderItem = function(self, item, key)

	end,
	renderItem = function(self, item, key)

	end,
	afterRenderItem = function(self, item, key)
		ui.dummy(vZero)
	end,
	isMouseOverItem = function(self, item, key)
		return (ui.isWindowHovered() and ui.isMouseHoveringRect(self.itemsMeta[key].min, self.itemsMeta[key].max, false))
	end,
	onMouseOverItem = function(self, item, key)
		self.clearHighlight = false
		self.highlightedItem = key
	end,
	onClickItem = function(self, item, key)

	end,
	afterItems = function(self)

	end,

	-- sort items in the market table
	sortingFunction = function(e1,e2)
		return e1 < e2
	end
}

local ListWidget = {}

function ListWidget.New(id, title, config)
	local defaultSizes = ui.rescaleUI({
		padding = Vector2(14, 14),
		itemSpacing = Vector2(4, 9),
	}, Vector2(1600, 900))

	local self
	self = {
		scroll = 0,
		id = id,
		popup = config.popup or ModalWindow.New('popupMsg' .. id, function()
			ui.text(self.popup.msg)
			ui.dummy(Vector2((ui.getContentRegion().x - 100) / 2, 0))
			ui.sameLine()
			if ui.button("OK", Vector2(100, 0)) then
				self.popup:close()
			end
		end),
		title = title,
		items = {},
		itemsMeta = {},
		highlightedItem = nil,
		clearHighlight = true,
		scrollReset = false,
		itemTypes = config.itemTypes or {},
		columnCount = config.columnCount or 0,
		style = {
			flags = config.style.flags or ui.WindowFlags {"AlwaysUseWindowPadding"},
			size = config.style.size or Vector2(ui.screenWidth / 2,0),
			titleFont = config.style.titleFont or ui.fonts.orbiteer.xlarge,
			highlightColor = config.style.highlightColor or Color(0,63,112),
			styleColors = config.style.styleColors or {},
			styleVars = config.style.styleVars or {
				WindowPadding = config.style.padding or defaultSizes.padding,
				ItemSpacing = config.style.itemSpacing or defaultSizes.itemSpacing,
			},
		},
		funcs = {
			beforeItems = config.beforeItems or defaultFuncs.beforeItems,
			canDisplayItem = config.canDisplayItem or defaultFuncs.canDisplayItem,
			beforeRenderItem = config.beforeRenderItem or defaultFuncs.beforeRenderItem,
			renderItem = config.renderItem or defaultFuncs.renderItem,
			afterRenderItem = config.afterRenderItem or defaultFuncs.afterRenderItem,
			isMouseOverItem = config.isMouseOverItem or defaultFuncs.isMouseOverItem,
			onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem,
			onClickItem = config.onClickItem or defaultFuncs.onClickItem,
			afterItems = config.afterItems or defaultFuncs.afterItems,
			sortingFunction = config.sortingFunction or defaultFuncs.sortingFunction,
		},
	}

	setmetatable(self, {
		__index = ListWidget,
		class = "UI.ListWidget",
	})

	return self
end

function ListWidget:Render()
	ui.withStyleColorsAndVars(self.style.styleColors, self.style.styleVars, function()
		ui.child("List##" .. self.id, self.style.size, self.style.flags, function()
			self.contentRegion = ui.getContentRegion()
			self.clearHighlight = true
			self.funcs.beforeItems(self)

			for key, item in pairs(self.items) do
				if self.itemsMeta[key] == nil then
					self.itemsMeta[key] = {}
				end

				self.itemsMeta[key].min = ui.getCursorScreenPos()
				self.itemsMeta[key].min.x = self.itemsMeta[key].min.x - self.style.styleVars.WindowPadding.x / 2

				self.funcs.beforeRenderItem(self, item, key)

				self.funcs.renderItem(self, item, key)

				self.itemsMeta[key].max = ui.getCursorScreenPos()
				self.itemsMeta[key].max.x = self.itemsMeta[key].max.x + self.contentRegion.x + self.style.styleVars.WindowPadding.x / 2

				self.funcs.afterRenderItem(self, item, key)

				if self.funcs.isMouseOverItem(self, item, key) then
					self.funcs.onMouseOverItem(self, item, key)
					if ui.isMouseClicked(0) then
						self.funcs.onClickItem(self, item, key)
					end
				end
			end

			self.funcs.afterItems(self)
			if self.clearHighlight then
				self.highlightedItem = nil
			end
		end)
	end)
end


return ListWidget
