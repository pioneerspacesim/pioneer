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

local bulletinBoard
local chatForm
local searchText = ""
local searchTextEntered = false
local textWrapWidth = 100
local scrollPosPrev = 0.0
local scrollPos = 0.0

local icons = {}
local currentIconSize = Vector2(0,0)
local chatWin = ModalWindow.New('bbChatWindow', function() end, function (self, drawPopupFn)
	ui.setNextWindowPosCenter('Always')
	ui.setNextWindowSize(widgetSizes.popupSize, "Always")
	ui.withStyleColorsAndVars({PopupBg = Color(20, 20, 80, 230)}, {WindowBorderSize = 1, }, drawPopupFn)
end)

local function adActive(ref, ad)
	return not ((type(ad.isEnabled) == "function" and not ad.isEnabled(ref)) or Game.paused)
end

bulletinBoard = Table.New("BulletinBoardTable", false, {
	columnCount = 1,
	size = Vector2(ui.screenWidth * 0.5, 0),
	padding = widgetSizes.innerPadding,
	--itemSpacing = Vector2(6, 20),
	flags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar"},
	initTable = function(self)
		--scrollPos = math.ceil(-scrollPos * ui.getScrollMaxY())
		ui.setColumnWidth(0, self.style.size.x)

		local sc = math.ceil(-ui.getScrollY()* 100 / ui.getScrollMaxY())
		if scrollPosPrev ~= sc then
			scrollPos = sc
		elseif scrollPosPrev ~= scrollPos then
			ui.setScrollY(math.ceil(-scrollPos/100 * ui.getScrollMaxY()))
		end

		scrollPosPrev = scrollPos
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

		ui.withStyleColorsAndVars({Text = adTextColor}, {}, function()
			ui.dummy(Vector2(0,0))
			ui.text(string.upper(icon))
			ui.sameLine()
			icons[icon]:Draw(widgetSizes.iconSize)
			ui.text(item.description)
		end)
		ui.nextColumn()
	end,
	itemFrame = function(self, item, min, max)
		local ofs = Vector2(self.style.itemSpacing.x, 0)
		ui.addRect(min, max, Color(25, 64, 90, 230), 0, 0, 2)
	end,
	onClickItem = function(self, item, key)
		local station = Game.player:GetDockedWith()
		local ref = key
		local ad = SpaceStation.adverts[station][ref]

		if Game.paused then
			return
		end

		local chatFunc = function (form, option)
			return ad.onChat(form, ref, option)
		end
		local removeFunc = function ()
			station:RemoveAdvert(ref)
		end
		local closeFunc = function ()
			station:UnlockAdvert(ref)
			chatWin:close()
		end

		chatForm = ChatForm.New(chatFunc, removeFunc, closeFunc, ref, tabGroup, {buttonSize = Vector2(0, 24)})
		if chatForm.market then
			widgetSizes.popupSize = Vector2(1200, 0)
		else
			widgetSizes.popupSize = Vector2(500, 0)
		end

		station:LockAdvert(ref)
		chatWin.innerHandler = function() chatForm:render() end
		chatWin:open()
	end,
	sortingFunction = function(s1,s2) return s1.description < s2.description end
})

local function refresh()

	local station = Game.player:GetDockedWith()
	local ads = SpaceStation.adverts[station]
	bulletinBoard.items = {}

	for ref,ad in pairs(ads) do
		if searchText == ""
				or searchText ~= "" and string.find(
				string.lower(ad.description),
				string.lower(searchText),
				1, true)
		then
			bulletinBoard.items[ref] = ad
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
					bulletinBoard:render()
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
