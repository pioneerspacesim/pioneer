-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local SpaceStation = require 'SpaceStation'

local StationView = require 'pigui.views.station-view'
local Table = require 'pigui.libs.table'
local ChatForm = require 'pigui.libs.chat-form'
local ModalWindow = require 'pigui.libs.modal-win'
local PiImage = require 'ui.PiImage'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local colors = ui.theme.colors

local adTextColor = colors.white
local chatBackgroundColor = Color(20, 20, 80, 230)
local containerFlags = ui.WindowFlags {"AlwaysUseWindowPadding"}
local widgetSizes = ui.rescaleUI({
	iconSize = Vector2(20, 20),
	chatButtonBase = Vector2(0, 24),
	chatButtonSize = Vector2(0, 24),
	itemSpacing = Vector2(18, 4),
	bbContainerSize = Vector2(0, 0),
	bbSearchSize = Vector2(0, 0),
	bbPadding = Vector2(14, 11),
	innerPadding = Vector2(0, 3),
	rowVerticalSpacing = Vector2(0, 6),
	popupSize = Vector2(1200, 0),
	popupBig = Vector2(1200, 0),
	popupSmall = Vector2(500, 0),
}, Vector2(1600, 900))

local bulletinBoard
local chatForm
local searchText = ""
local searchTextEntered = false
local textWrapWidth = 100

local icons = {}
local currentIconSize = Vector2(0,0)
local chatWin = ModalWindow.New('bbChatWindow', function() end, function (self, drawPopupFn)
	ui.setNextWindowPosCenter('Always')
	ui.setNextWindowSize(widgetSizes.popupSize, "Always")
	ui.withStyleColorsAndVars({PopupBg = chatBackgroundColor}, {WindowBorderSize = 1, }, drawPopupFn)
end)

local function adActive(ref, ad)
	local station = Game.player:GetDockedWith()
	return not ((SpaceStation.adverts[station][ref] ~= nil and type(ad.isEnabled) == "function" and not ad.isEnabled(ref)) or Game.paused)
end

local function refresh()
	local station = Game.player:GetDockedWith()
	local ads = SpaceStation.adverts[station]
	bulletinBoard.items = {}

	for ref,ad in pairs(ads or {}) do
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

bulletinBoard = Table.New("BulletinBoardTable", false, {
	columnCount = 2,
	size = Vector2(ui.screenWidth * 0.8, 0),
	windowPadding = widgetSizes.innerPadding,
	initTable = function(self)
		local iconColumnWidth = widgetSizes.iconSize.x + widgetSizes.itemSpacing.x
		local columnWidth = (self.style.size.x - iconColumnWidth) / (self.columnCount-1)
		textWrapWidth = columnWidth
		ui.setColumnWidth(0, widgetSizes.iconSize.x + widgetSizes.itemSpacing.x)
		ui.setColumnWidth(1, columnWidth)
	end,
	renderItem = function(self, item, key)
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

		icons[icon]:Draw(widgetSizes.iconSize)
		ui.nextColumn()
		ui.withStyleColorsAndVars({Text = adTextColor}, {ItemSpacing = widgetSizes.rowVerticalSpacing}, function()
			ui.textWrapped(item.description)
			ui.nextColumn()
		end)
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
			refresh()
		end
		local closeFunc = function ()
			station:UnlockAdvert(ref)
			chatWin:close()
			refresh()
		end
		local resizeFunc = function ()
			if chatForm then
				chatForm.style.contentWidth = nil
				if chatForm.market then
					widgetSizes.popupSize = widgetSizes.popupBig
					-- ChatForm resizes the buttons so we need to reset their size when we resize the chat window.
					widgetSizes.chatButtonSize(widgetSizes.chatButtonBase.x, widgetSizes.chatButtonBase.y)
				else
					widgetSizes.popupSize = widgetSizes.popupSmall
					-- ChatForm resizes the buttons so we need to reset their size when we resize the chat window.
					widgetSizes.chatButtonSize(widgetSizes.chatButtonBase.x, widgetSizes.chatButtonBase.y)
				end
			end
		end

		chatForm = ChatForm.New(chatFunc, removeFunc, closeFunc, resizeFunc, ref, StationView, {buttonSize = widgetSizes.chatButtonSize})

		station:LockAdvert(ref)
		chatWin.innerHandler = function() chatForm:render() end
		chatForm.resizeFunc()
		chatWin:open()
	end,
	sortingFunction = function(s1,s2) return s1.description < s2.description end
})

local function renderBulletingBoard()
	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.withStyleVars({WindowPadding = widgetSizes.bbPadding}, function()
			ui.child("BulletinBoardContainer", widgetSizes.bbContainerSize(0, ui.getContentRegion().y - StationView.style.height), containerFlags, function()
				ui.withStyleVars({WindowPadding = widgetSizes.innerPadding}, function()
					ui.pushTextWrapPos(textWrapWidth)
					bulletinBoard:render()
					ui.popTextWrapPos()
					ui.sameLine()
					ui.child("BulletinBoardSearch", widgetSizes.bbSearchSize, containerFlags, function()
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

		StationView:shipSummary()
	end)
end

StationView:registerView({
	id = "bulletinBoard",
	name = lui.BULLETIN_BOARD,
	icon = ui.theme.icons.bbs,
	showView = true,
	draw = renderBulletingBoard,
	refresh = function ()
		refresh()
		bulletinBoard.scrollReset = true
	end,
})
