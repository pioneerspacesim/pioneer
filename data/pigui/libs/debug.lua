-- Copyright © 2008-2019 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'
local SpaceStation = require 'SpaceStation'

local StationView = require 'pigui.views.station-view'
local Table = require 'pigui.libs.table'
local ChatForm = require 'pigui.libs.chat-form'
local ModalWindow = require 'pigui.libs.modal-win'
local List = require 'pigui.libs.list'
local PiImage = require 'ui.PiImage'

local ui = require 'pigui'
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

local jobList = List.New("JobList", false, {
	size = Vector2(ui.screenWidth * 0.5, 0),
	style = {
		windowPadding = widgetSizes.innerPadding,
		--itemSpacing = Vector2(6, 20),
		flags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar"},
	},
	beforeItems = function(self)
		local sc = math.ceil(-ui.getScrollY()* 100 / ui.getScrollMaxY())
		if scrollPosPrev ~= sc then
			scrollPos = sc
		elseif scrollPosPrev ~= scrollPos then
			ui.setScrollY(math.ceil(-scrollPos/100 * ui.getScrollMaxY()))
		end

		scrollPosPrev = scrollPos
	end,
	beforeRenderItem = function(self, item, key)
		if self.itemsMeta[key] ~= nil and self.itemsMeta[key].min and self.itemsMeta[key].max then
			if self.highlightedItem == key then
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, self.style.highlightColor, 0, 0)
			else
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(8, 19, 40), 0, 0)
			end
			ui.addRect(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(25, 64, 90), 0, 0, 2)
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
	afterRenderItem = function(self, item, key)
		ui.dummy(vZero)
	end,
	onClickItem = function(self, item, key)
		local station = Game.player:GetDockedWith()
		local ref = key
		local ad = SpaceStation.adverts[station][ref]

		if Game.paused then
			return
		end
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
			ui.child("BulletinBoardContainer", vZero, containerFlags, function()
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
					jobList:Render()
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
