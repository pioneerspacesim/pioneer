-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'
local SpaceStation = require 'SpaceStation'
local Character = require 'Character'
local InfoFace = require 'ui.PiguiFace'

local InfoView = require 'pigui.views.info-view'
local ModalWindow = require 'pigui.libs.modal-win'
local List = require 'pigui.libs.list'
local PiImage = require 'ui.PiImage'

local ui = require 'pigui'
local pionillium = ui.fonts.pionillium
local orbiteer = ui.fonts.orbiteer
local l = Lang.GetResource("ui-core")

local colors = ui.theme.colors
local missionDetailsBg = Color(12, 29, 52)

local vZero = Vector2(0,0)
local adTextColor = colors.white
local containerFlags = ui.WindowFlags {"AlwaysUseWindowPadding"}
local listFlags = ui.WindowFlags {}
local detailsFlags = ui.WindowFlags {"AlwaysUseWindowPadding", "AlwaysAutoResize"}
local widgetSizes = ui.rescaleUI({
	iconSize = Vector2(20, 20),
	chatButtonBase = Vector2(0, 24),
	chatButtonSize = Vector2(0, 24),
	itemSpacing = Vector2(18, 4),
	bbContainerSize = Vector2(0, 0),
	bbSearchSize = Vector2(0, 0),
	bbPadding = Vector2(12, 12),
	innerPadding = Vector2(0, 3),
	popupSize = Vector2(1200, 0),
	popupBig = Vector2(1200, 0),
	popupSmall = Vector2(500, 0),
	faceSize = Vector2(280, 320),
	missionDesc = {
		-- distLength = ui.calcTextSize(string.format('%.2f %s', 99999, l.LY))
		distLength = 150,
	}
}, Vector2(1600, 900))


local selectedItem
local clientFace
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
			elseif selectedItem == item then
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, missionDetailsBg, 0, 0)
			else
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(8, 19, 40), 0, 0)
			end
			ui.addRect(self.itemsMeta[key].min, self.itemsMeta[key].max, Color(25, 64, 90), 0, 0, 2)
		end
	end,
	renderItem = function(self, item, key)
		local icon = item.icon or "default"

		if(icons[icon] == nil) then
			icons[icon] = PiImage.New("icons/bbs/" .. icon .. ".png")
			currentIconSize = icons[icon].texture.size
		end

		ui.group(function()
			ui.withStyleColorsAndVars({}, {}, function()
				ui.dummy(Vector2(0,0))

				local rightAlignPos = ui.getContentRegion().x - currentIconSize.x - widgetSizes.itemSpacing.x
				ui.text(string.upper(item:GetTypeDescription()))

				ui.sameLine(rightAlignPos)
				icons[icon]:Draw(widgetSizes.iconSize)

				local dist = Game.system and string.format('%.2f %s', Game.system:DistanceTo(item.location) or "???", l.LY)
				rightAlignPos = rightAlignPos - widgetSizes.missionDesc.distLength - widgetSizes.itemSpacing.x
				ui.sameLine(rightAlignPos)
				ui.text(string.upper(dist))

				rightAlignPos = rightAlignPos - widgetSizes.missionDesc.distLength - widgetSizes.itemSpacing.x
				local days = string.format(l.D_DAYS_LEFT, math.max(0, (item.due - Game.time) / (24*60*60)))
				ui.sameLine(rightAlignPos)
				ui.text(days)


				ui.text(item.description or '')
			end)
		end)
	end,
	afterRenderItem = function(self, item, key)
		ui.dummy(vZero)
	end,
	onClickItem = function(self, item, key)
		if Game.paused then
			return
		end

		selectedItem = item
		clientFace = InfoFace.New(item.client, {
			windowPadding = widgetSizes.windowPadding,
			itemSpacing = widgetSizes.itemSpacing,
			size = widgetSizes.faceSize,
			charInfoPadding = Vector2(12,12),
			nameFont = orbiteer.large,
			titleFont = orbiteer.medlarge,
		})
	end,
	sortingFunction = function(s1,s2) return s1.description < s2.description end
})


local function refresh()
	selectedItem = nil
	for ref,mission in pairs(Character.persistent.player.missions) do
		for k, v in pairs(mission) do
			print(k,v)
		end
		jobList.items[ref] = mission
	end

	--[[
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
	]]--
end

local function renderView()
	ui.withFont(pionillium.large.name, pionillium.large.size, function()
		ui.withStyleVars({WindowPadding = widgetSizes.bbPadding}, function()
			ui.child("MissionsContainer", vZero, containerFlags, function()
				--print("start", scrollPos)
				scrollPos = ui.vSliderInt('###MissionsScrollbar', Vector2(10, ui.getContentRegion().y), scrollPosPrev, -100, 0, "")
				ui.sameLine()
				ui.child("MissionList", Vector2(ui.screenWidth * 0.5, 0), listFlags, function()
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

				if selectedItem then
					ui.sameLine()
					ui.withStyleColorsAndVars({ChildWindowBg = missionDetailsBg},{WindowPadding = Vector2(8, 16)}, function()
						ui.child("MissionDetails", vZero, detailsFlags, function()
							local winSize = ui.getContentRegion() - widgetSizes.faceSize
							ui.columns(2, "MissionDetailsColumns", false)
							ui.setColumnWidth(0, winSize.x)
							ui.setColumnWidth(1, widgetSizes.faceSize.x)
							ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
								ui.text(string.upper(selectedItem:GetTypeDescription()))
							end)
							ui.text('')

							ui.pushTextWrapPos(winSize.x*0.8)
							ui.textWrapped(selectedItem.introtext)
							ui.popTextWrapPos()

							ui.text('')
							ui.text(string.format('Destination: %s, %s', selectedItem.location:GetSystemBody().name, selectedItem.location:GetStarSystem().name))
							ui.text(string.format('Deadline: %s', Format.Date(selectedItem.due)))
							ui.text(string.format('Wage: %s', Format.Money(selectedItem.reward)))

							ui.nextColumn()
							clientFace:render()
						end)
					end)
				end
			end)
		end)
	end)
end

InfoView:registerView({
	id = "missions",
	name = l.MISSIONS,
	icon = ui.theme.icons.star,
	showView = true,
	draw = renderView,
	refresh = function()
		refresh()
	end,
})
