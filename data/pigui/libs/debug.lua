-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = import 'Lang'
local Game = import 'Game'
local Format = import 'Format'
local SpaceStation = import 'SpaceStation'

local StationView = import 'pigui/views/station-view'
local Table = import 'pigui/libs/table.lua'
local ChatForm = import 'pigui/libs/chat-form.lua'
local ModalWindow = import 'pigui/libs/modal-win.lua'
local PiImage = import 'ui/PiImage'

local ui = import 'pigui/pigui.lua'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("core")
local colors = ui.theme.colors

local vZero = Vector2(0,0)
local adTextColor = colors.white
local searchFlags = ui.WindowFlags {}
local containerFlags = ui.WindowFlags {"AlwaysUseWindowPadding"}
local entryFlags = ui.WindowFlags {"AlwaysUseWindowPadding", "AlwaysAutoResize"}
local widgetSizes = ui.rescaleUI({
	iconSize = Vector2(20, 20),
	chatButtonBase = Vector2(0, 24),
	chatButtonSize = Vector2(0, 24),
	itemSpacing = Vector2(18, 4),
	bbContainerSize = Vector2(0, 0),
	bbSearchSize = Vector2(0, 0),
	bbPadding = Vector2(14, 11),
	innerPadding = Vector2(0, 3),
	popupSize = Vector2(1200, 0),
	popupBig = Vector2(1200, 0),
	popupSmall = Vector2(500, 0),
}, Vector2(1600, 900))

local searchText = ""
local searchTextEntered = false
local textWrapWidth = 100
local scrollPosPrev = 0.0
local scrollPos = 0.0

local icons = {}
local currentIconSize = Vector2(0,0)

local function adActive(ref, ad)
	return not ((type(ad.isEnabled) == "function" and not ad.isEnabled(ref)) or Game.paused)
end

local defaultFuncs = {

	initTable = function(self)
	end,

	onMouseOverItem = function(self, item)
	end,

	onClickItem = function (self, e)

	end,

	renderItem = function(self, item)

	end,

	itemFrame = function(self, item, min, max)

	end,

	-- sort items in the market table
	sortingFunction = function(e1,e2)
		return e1 < e2
	end
}

local List = {}

function List.New(id, title, config)
	local defaultSizes = ui.rescaleUI({
		windowPadding = Vector2(14, 14),
		itemSpacing = Vector2(4, 9),
	}, Vector2(1600, 900))

	local self
	self = {
		scroll = 0,
		id = id,
		title = title,
		items = {},
		itemsMeta = {},
		size = config.size or Vector2(ui.screenWidth / 2,0),
		flags = config.flags or ui.WindowFlags {"AlwaysUseWindowPadding"},
		style = {
			titleFont = config.titleFont or ui.fonts.orbiteer.xlarge,
			highlightColor = config.highlightColor or Color(0,63,112),
			styleVars = {
				WindowPadding = config.windowPadding or defaultSizes.windowPadding,
				ItemSpacing = config.itemSpacing or defaultSizes.itemSpacing,
			},
			styleColors = {},
		},
		funcs = {
			beforeItems = config.beforeItems or function() end,
			canDisplayItem = config.canDisplayItem or defaultFuncs.canDisplayItem,
			beforeRenderItem = config.beforeRenderItem or function() end,
			renderItem = config.renderItem or defaultFuncs.renderItem,
			afterRenderItem = config.afterRenderItem or function() ui.dummy(vZero) end,
			onMouseOverItem = config.onMouseOverItem or defaultFuncs.onMouseOverItem,
			onClickItem = config.onClickItem or defaultFuncs.onClickItem,
			sortingFunction = config.sortingFunction or defaultFuncs.sortingFunction,
		},
	}

	setmetatable(self, {
		__index = List,
		class = "UI.List",
	})

	return self
end

function List:render()
	ui.withStyleColorsAndVars(self.style.styleColors, self.style.styleVars, function()
		ui.child("List##" .. self.id, self.size, self.flags, function()
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

local
jobList = List.New("JobList", false, {
	size = Vector2(ui.screenWidth * 0.5, 0),
	flags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar"},
	style = {
		windowPadding = widgetSizes.innerPadding,
		--itemSpacing = Vector2(6, 20),
	},
	beforeItems = function(self)
		--scrollPos = math.ceil(-scrollPos * ui.getScrollMaxY())

		local sc = math.ceil(-ui.getScrollY()* 100 / ui.getScrollMaxY())
		if scrollPosPrev ~= sc then
			scrollPos = sc
		elseif scrollPosPrev ~= scrollPos then
			ui.setScrollY(math.ceil(-scrollPos/100 * ui.getScrollMaxY()))
		end

		scrollPosPrev = scrollPos
	end,
	beforeRenderItem = function(self, item, key)
		if self.itemsMeta[key] ~= nil then
			ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(8, 19, 40, 230), 0, 0)
			ui.addRect(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(25, 64, 90, 230), 0, 0, 2)
		end
	end,
	renderItem = function(self, item, key)
		local station = Game.player:GetDockedWith()
		local ref = key
		local ad = SpaceStation.adverts[station][ref]
		local icon = item.icon or "default"

		if(icons[icon] == nil) then
			icons[icon] = PiImage.New("icons/bbs/" .. icon .. ".png")
			currentIconSize = icons[icon].texture.size
		end

		if (adActive(key, item)) then
			adTextColor = colors.white
		else
			adTextColor = colors.grey
		end

		ui.group(function()
			ui.withStyleColorsAndVars({Text = adTextColor}, {}, function()
				ui.dummy(Vector2(0,0))
				ui.text(string.upper(icon))
				ui.sameLine()
				icons[icon]:Draw(widgetSizes.iconSize)
				ui.text(item.description)
			end)
		end)
	end,
	onClickItem = function(self, item, key)
		local station = Game.player:GetDockedWith()
		local ref = key
		local ad = SpaceStation.adverts[station][ref]

		if Game.paused then
			return
		end

		print('click')
	end,
	sortingFunction = function(s1,s2) return s1.description < s2.description end
})

local function refresh()

	local station = Game.player:GetDockedWith()
	local ads = SpaceStation.adverts[station]
	jobList.items = {}

	for ref,ad in pairs(ads) do
		if searchText == ""
				or searchText ~= "" and string.find(
				string.lower(ad.description),
				string.lower(searchText),
				1, true)
		then
			jobList.items[ref] = ad
		end
	end
end

local function drawCommoditytView()
	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.withStyleVars({WindowPadding = widgetSizes.bbPadding}, function()
			ui.child("BulletinBoardContainer", widgetSizes.bbContainerSize(0, ui.getContentRegion().y - StationView.style.height), containerFlags, function()
				--print("start", scrollPos)
				scrollPos = ui.vSliderInt('###BulletinBoardScrollbar', Vector2(10, ui.getContentRegion().y), scrollPosPrev, -100, 0, "")
				ui.sameLine()
				ui.child("BulletinBoardList", Vector2(ui.screenWidth * 0.5, 0), searchFlags, function()
					ui.text("Filter")
					ui.sameLine()
					ui.text("Sort:")
					ui.sameLine()
					ui.text("Due")
					ui.sameLine()
					ui.text("Dist")
					ui.sameLine()
					ui.text("Importance")
					ui.sameLine()
					ui.text("Type none")
					ui.pushTextWrapPos(textWrapWidth)
					jobList:render()
					--print("end", scrollPos)
					ui.popTextWrapPos()
				end)
				ui.sameLine()
				ui.child("BulletinBoardSearch", widgetSizes.bbSearchSize, searchFlags, function()
					ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
						ui.text(l.SEARCH)
					end)
					ui.pushItemWidth(ui.getContentRegion().x)
					searchText, searchTextEntered = ui.inputText("", searchText, {})
					if searchTextEntered then
						refresh()
					end
				end)
			end)
		end)

		StationView:shipSummary()
	end)
end

local importTestView = {

}

importTestView.showView = true

function importTestView:render()
	refresh()
	drawCommoditytView()
end

return importTestView
