-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Space = require 'Space'
local SpaceStation = require 'SpaceStation'

local StationView = require 'pigui.views.station-view'
local Table = require 'pigui.libs.table'
local ChatForm = require 'pigui.libs.chat-form'
local ModalWindow = require 'pigui.libs.modal-win'
local PiImage = require 'pigui.libs.image'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("core")
local lui = Lang.GetResource("ui-core")
local colors = ui.theme.colors
local icons = ui.theme.icons

local adTextColor = colors.white
local widgetSizes = ui.rescaleUI({
	chatButtonBase = Vector2(0, 24),
	chatButtonSize = Vector2(0, 24),
	itemSpacing = Vector2(18, 4),
	itemInnerSpacing = Vector2(8, 4),
	rowVerticalSpacing = Vector2(0, 6),
	popupSize = Vector2(1200, 0),
	popupBig = Vector2(1200, 0),
	popupSmall = Vector2(500, 0),
}, Vector2(1600, 900))

local bulletinBoard
local chatForm
local searchText = ""
local searchTextEntered = false

local images = {}
local chatWin = ModalWindow.New('bbChatWindow', function() end, function (self, drawPopupFn)
	ui.setNextWindowPosCenter('Always')
	ui.setNextWindowSize(widgetSizes.popupSize, 'Always')
	ui.withStyleColors({ PopupBg = ui.theme.colors.modalBackground }, drawPopupFn)
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
		if not searchText or searchText == ""
			or string.find(
				string.lower(ad.title),
				string.lower(searchText),
				1, true)
			or string.find(
				string.lower(ad.description),
				string.lower(searchText),
				1, true)
		then
			table.insert(bulletinBoard.items, ad)
		end
	end

	table.sort(bulletinBoard.items, bulletinBoard.funcs.sortingFunction)
end

bulletinBoard = Table.New("BulletinBoardTable", false, {
	columnCount = 1,
	initTable = function(self)
		ui.setColumnWidth(0, self.style.size.x)
	end,
	renderItem = function(self, item, key)
		local icon = item.icon or "default"
		local region = ui.getContentRegion()

		if(images[icon] == nil) then
			images[icon] = PiImage.New("icons/bbs/" .. icon .. ".png")
		end

		if (adActive(item.__ref, item)) then
			adTextColor = colors.white
		else
			adTextColor = colors.grey
		end

		ui.withFont(pionillium.title, function()
			images[icon]:Draw(Vector2(ui.getTextLineHeight()))
		end)
		ui.sameLine(0, widgetSizes.itemInnerSpacing.x)

		ui.withStyleColorsAndVars({Text = adTextColor}, {ItemSpacing = widgetSizes.rowVerticalSpacing}, function()
			ui.withFont(pionillium.title, function()
				ui.text(item.title)
			end)

			ui.withFont(pionillium.body, function()
				local textHeight = ui.getTextLineHeight()
				local iconSize = Vector2(textHeight)

				local maxDuration = textHeight * 6 + widgetSizes.itemSpacing.x
				local maxDistance = textHeight * 5 + widgetSizes.itemSpacing.x
				local maxReward = textHeight * 5

				if item.due then
					ui.sameLine(region.x - maxDuration - maxDistance - maxReward)
					ui.icon(icons.clock, iconSize, adTextColor)
					ui.sameLine()
					ui.text(ui.Format.Duration(item.due - Game.time, 3))
				end

				if item.location then
					ui.sameLine(region.x - maxDistance - maxReward)
					ui.icon(icons.distance, iconSize, adTextColor)
					ui.sameLine()

					if item.location:isa("Body") then
						local alt = Game.player:GetAltitudeRelTo(item.location)
						ui.text(ui.Format.Distance(alt))
					elseif Game.system and item.location:IsSameSystem(Game.system.path) then
						local alt = Game.player:GetAltitudeRelTo(Space.GetBody(item.location.bodyIndex))
						ui.text(ui.Format.Distance(alt))
					else
						local playerSystem = Game.system or Game.player:GetHyperspaceTarget()
						ui.text(string.format("%0.2f %s", item.location:DistanceTo(playerSystem), lui.LY))
					end
				end

				if item.reward then
					ui.sameLine(region.x - maxReward)
					ui.icon(icons.money, iconSize, adTextColor)
					ui.sameLine()
					ui.text(ui.Format.Number(item.reward, 0))
				end
			end)

			ui.withFont(pionillium.heading, function()
				ui.textWrapped(item.description)
				-- add a little bit of extra vertical space between rows
				ui.dummy(widgetSizes.rowVerticalSpacing)
			end)
			ui.nextColumn()
		end)
	end,
	onClickItem = function(self, item, key)
		local station = Game.player:GetDockedWith()
		local ref = item.__ref
		local ad = SpaceStation.adverts[station][ref]

		-- TODO: if the player is watching the BBS while an ad expires, the ad
		-- will be grayed-out but not removed until the player clicks on
		-- something.
		if not ad then
			refresh()
			return
		end

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
	sortingFunction = function(s1,s2)
		return s1.title < s2.title
			or (s1.title == s2.title and s1.description < s2.description)
	end,
	iterator = ipairs
})

local function renderBulletinBoard()
	ui.withFont(pionillium.title, function()
		bulletinBoard.style.size = ui.getContentRegion() * Vector2(1 / 1.6, 0)
		bulletinBoard:render()
		ui.sameLine()

		ui.child("BulletinBoardSearch", Vector2(0, 0), function()
			ui.withFont(orbiteer.title, function()
				ui.text(l.SEARCH)
			end)
			ui.pushItemWidth(ui.getContentRegion().x)
			searchText, searchTextEntered = ui.inputText("##searchText", searchText, {})
			if searchTextEntered then
				refresh()
			end
		end)
	end)
end

StationView:registerView({
	id = "bulletinBoard",
	name = lui.BULLETIN_BOARD,
	icon = ui.theme.icons.bbs,
	showView = true,
	draw = renderBulletinBoard,
	refresh = function ()
		refresh()
		bulletinBoard.scrollReset = true
	end,
	debugReload = function()
		package.reimport()
	end
})
