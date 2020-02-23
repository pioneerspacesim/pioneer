-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local ui = require 'pigui'
local ModalWindow = require 'pigui.libs.modal-win'

local vZero = Vector2(0, 0)

local defaultFuncs = {
	beforeItems = function()

	end,
	canDisplayItem = function()

	end,
	beforeRenderItem = function()

	end,
	renderItem = function()

	end,
	afterRenderItem = function()

	end,
	onMouseOverItem = function()

	end,
	onClickItem = function()

	end,
	afterItems = function()

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
			beforeItems = config.beforeItems or function() end,
			canDisplayItem = config.canDisplayItem or defaultFuncs.canDisplayItem,
			beforeRenderItem = config.beforeRenderItem or function() end,
			renderItem = config.renderItem or defaultFuncs.renderItem,
			afterRenderItem = config.afterRenderItem or function() ui.dummy(vZero) end,
			onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem,
			onClickItem = config.onClickItem or defaultFuncs.onClickItem,
			afterItems = config.afterItems or function() end,
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
			local startPos
			local endPos

			local contentRegion = ui.getContentRegion()

			self.funcs.beforeItems(self)

			self.highlightStart = nil
			self.highlightEnd = nil

			for key, item in pairs(self.items) do
				self.funcs.beforeRenderItem(self, item, key)

				startPos = ui.getCursorScreenPos()
				startPos.x = startPos.x - self.style.styleVars.WindowPadding.x / 2

				self.funcs.renderItem(self, item, key)

				endPos = ui.getCursorScreenPos()
				endPos.x = endPos.x + contentRegion.x + self.style.styleVars.WindowPadding.x / 2

				self.funcs.afterRenderItem(self, item, key)

				if self.itemsMeta == nil then
					self.itemsMeta = {}
				end

				if self.itemsMeta[key] == nil then
					self.itemsMeta[key] = {}
				end

				self.itemsMeta[key].min = startPos
				self.itemsMeta[key].max = endPos

				if ui.isWindowHovered() and ui.isMouseHoveringRect(startPos, endPos, false) then
					self.funcs.onMouseOverItem(self, item, key)
					if ui.isMouseClicked(0) then
						self.funcs.onClickItem(self, item, key)
					end

					self.highlightStart = startPos
					self.highlightEnd = endPos
				end
			end
		end)
	end)
end


return ListWidget
