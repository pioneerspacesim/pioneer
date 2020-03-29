-- Copyright © 2008-2020 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local Lang = require 'Lang'
local Game = require 'Game'
local Format = require 'Format'
local Mission = require 'Mission'
local Space = require 'Space'
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
local missionDetailsButtonColor = Color(12,36,96)
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
	itemSpacing = Vector2(9, 4),
	bbContainerSize = Vector2(0, 0),
	bbSearchSize = Vector2(0, 0),
	bbPadding = Vector2(12, 12),
	innerPadding = Vector2(0, 3),
	popupSize = Vector2(1200, 0),
	popupBig = Vector2(1200, 0),
	popupSmall = Vector2(500, 0),
	faceSize = Vector2(280, 320),
	buttonFrameAlign = Vector2(0, 4),
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
local scrollMax = 0

local icons = {}

local filterIcons = {
	'assassination',
	'combat',
	'delivery',
	'haul',
	'searchrescue',
	'taxi',
}

local function getIcon(iconName)
	if not icons[iconName] then icons[iconName] = PiImage.New("icons/bbs/" .. iconName .. ".png") end
	return icons[iconName]
end

local function adActive(ref, ad)
	return not ((type(ad.isEnabled) == "function" and not ad.isEnabled(ref)) or Game.paused)
end

local jobList = List.New("JobList", false, {
	size = Vector2(ui.screenWidth * 0.5, 0),
	style = {
		padding = Vector2(0,0),
		itemPadding = Vector2(8,8),
		--itemSpacing = Vector2(6, 20),
		flags = ui.WindowFlags {"AlwaysUseWindowPadding", "NoScrollbar"},
		itemBgColor = Color(8, 19, 40),
		highlightColor = Color(12,56,112),
		selectColor = Color(12,48,96),
		--selectColor = Color(12,63,112),
		itemFrameColor = Color(25, 64, 90),
	},
	beforeItems = function(self)
		scrollMax = ui.getScrollMaxY()
		local sc = math.ceil(-ui.getScrollY()* 100 / scrollMax)
		if scrollPosPrev ~= sc then
			scrollPos = sc
		elseif scrollPosPrev ~= scrollPos then
			ui.setScrollY(math.ceil(-scrollPos/100 * scrollMax))
		end

		scrollPosPrev = scrollPos
	end,
	beforeRenderItem = function(self, item, key)
		if self.itemsMeta[key] ~= nil and self.itemsMeta[key].min and self.itemsMeta[key].max then
			if self.highlightedItem == key then
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, self.style.highlightColor, 0, 0)
			elseif selectedItem ~= item then
				ui.addRectFilled(self.itemsMeta[key].min, self.itemsMeta[key].max, self.style.itemBgColor, 0, 0)
			end

			ui.addRect(self.itemsMeta[key].min, self.itemsMeta[key].max, self.style.selectColor, 0, 0, 2)
		end
	end,
	renderItem = function(self, item, key)
		local icon = item.icon or "default"

		ui.group(function()
			ui.withStyleColorsAndVars({}, {}, function()
				ui.dummy(self.style.paddingDummy)

				local rightAlignPos = ui.getContentRegion().x - widgetSizes.iconSize.x - widgetSizes.itemSpacing.x
				ui.withFont(orbiteer.large.name, orbiteer.large.size, function()
					ui.dummy(vZero)
					ui.sameLine(self.style.itemPadding.x)
					ui.text(string.upper(item:GetTypeDescription()))
				end)

				ui.sameLine(rightAlignPos)
				getIcon(icon):Draw(widgetSizes.iconSize)

				ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
				local dist = Game.system and string.format('%.2f %s', Game.system:DistanceTo(item.location) or "???", l.LY)
				rightAlignPos = rightAlignPos - widgetSizes.missionDesc.distLength - widgetSizes.itemSpacing.x
				ui.sameLine(rightAlignPos)
				ui.text(string.upper(dist))

				rightAlignPos = rightAlignPos - widgetSizes.missionDesc.distLength - widgetSizes.itemSpacing.x
				local days = string.format(l.D_DAYS_LEFT, math.max(0, (item.due - Game.time) / (24*60*60)))
				ui.sameLine(rightAlignPos)
				ui.text(days)

				rightAlignPos = rightAlignPos - widgetSizes.missionDesc.distLength - widgetSizes.itemSpacing.x
				ui.sameLine(rightAlignPos)
				ui.text(Format.Money(item.reward))
				end)

				ui.dummy(vZero)
				ui.sameLine(self.style.itemPadding.x)
				ui.text(item.description or '')
			end)
		end)
	end,
	afterRenderItem = function(self, item, key)
		ui.dummy(vZero)
	end,
	onClickItem = function(self, item, key)
		selectedItem = item
		selectedItem.key = key
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
	jobList.items = {}
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
		ui.withStyleVars({WindowPadding = widgetSizes.bbPadding, ItemSpacing = widgetSizes.itemSpacing}, function()
			ui.child("MissionsContainer", vZero, containerFlags, function()
				if scrollMax > 0 then
					scrollPos = ui.vSliderInt('###MissionsScrollbar', Vector2(10, ui.getContentRegion().y), scrollPosPrev, -100, 0, "")
					ui.sameLine()
				end

				if selectedItem then
					ui.addRectFilled(jobList.itemsMeta[selectedItem.key].min, jobList.itemsMeta[selectedItem.key].max + Vector2(200, 0), jobList.style.selectColor, 0, 0)
				end

				ui.child("MissionList", Vector2(ui.screenWidth * 0.5, 0), listFlags, function()
					local buttonPadding = Vector2(16,4)
					local buttonSize = ui.calcTextSize("Type") + buttonPadding
					local rightAlignPos = ui.getContentRegion().x - buttonSize.x
					ui.setCursorPos(ui.getCursorPos() + widgetSizes.buttonFrameAlign)
					ui.text("Filter:")
					ui.sameLine()
					ui.setCursorPos(ui.getCursorPos() - widgetSizes.buttonFrameAlign)
					local icon
					--Tooltip Padding
					ui.withStyleColorsAndVars({},{WindowPadding = Vector2(6, 6)}, function()
						ui.withFont(orbiteer.small.name, orbiteer.small.size, function()
							for idx, iconName in ipairs(filterIcons) do
								icon = getIcon(iconName)
								ui.coloredSelectedImgIconButton({texture = icon.texture.id, uv0 = vZero, uv1 = icon.texture.uv}, widgetSizes.iconSize, false, 4, colors.buttonBlue, colors.white, iconName)
								ui.sameLine()
							end
						end)
					end)

					ui.withFont(pionillium.medlarge.name, pionillium.medlarge.size, function()
						ui.sameLine(rightAlignPos)
						ui.setCursorPos(ui.getCursorPos() + widgetSizes.buttonFrameAlign)
						ui.coloredSelectedButton("Type", buttonSize, false, colors.buttonBlue, nil, true)

						buttonSize.x = ui.calcTextSize("Importance").x + buttonPadding.x
						rightAlignPos = rightAlignPos - buttonSize.x - widgetSizes.itemSpacing.x
						ui.sameLine(rightAlignPos)
						ui.coloredSelectedButton("Importance", buttonSize, false, colors.buttonBlue, nil, true)

						buttonSize.x = ui.calcTextSize("Dist").x + buttonPadding.x
						rightAlignPos = rightAlignPos - buttonSize.x - widgetSizes.itemSpacing.x
						ui.sameLine(rightAlignPos)
						ui.coloredSelectedButton("Dist", buttonSize, false, colors.buttonBlue, nil, true)

						buttonSize.x = ui.calcTextSize("Due").x + buttonPadding.x
						rightAlignPos = rightAlignPos - buttonSize.x - widgetSizes.itemSpacing.x
						ui.sameLine(rightAlignPos)
						ui.coloredSelectedButton("Due", buttonSize, false, colors.buttonBlue, nil, true)
					end)

					rightAlignPos = rightAlignPos - ui.calcTextSize("Sort:").x - widgetSizes.itemSpacing.x
					ui.sameLine(rightAlignPos)
					ui.text("Sort: ")

					ui.pushTextWrapPos(textWrapWidth)
					jobList:Render()
					ui.popTextWrapPos()
				end)

				if selectedItem then
					ui.sameLine()
					ui.withStyleColorsAndVars({ChildWindowBg = jobList.style.selectColor},{WindowPadding = widgetSizes.bbPadding}, function()
						ui.child("MissionDetails", vZero, detailsFlags, function()
							selectedItem:GetViewHandler('missions')(selectedItem, widgetSizes)
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
		print('ICON!!!', ui.theme.icons.star)
		refresh()
	end,
})

Mission.RegisterViewHandler('missions', 'default', function(mission, style)
	local winSize = ui.getContentRegion() - style.faceSize
	ui.columns(2, "MissionDetailsColumns", false)
	ui.setColumnWidth(0, winSize.x)
	ui.setColumnWidth(1, style.faceSize.x)
	ui.withFont(orbiteer.xlarge.name, orbiteer.xlarge.size, function()
		ui.text(string.upper(mission:GetTypeDescription()))
	end)
	ui.text('')

	ui.pushTextWrapPos(winSize.x*0.8)
	ui.textWrapped(mission.introtext)
	ui.popTextWrapPos()

	ui.text('')
	ui.text(string.format('Destination: %s, %s', mission.location:GetSystemBody().name, mission.location:GetStarSystem().name))
	ui.sameLine()
	ui.setCursorPos(ui.getCursorPos() - style.buttonFrameAlign)
	if ui.coloredSelectedIconButton(ui.theme.icons.display_navtarget, style.iconSize, false, 4, colors.buttonBlue, colors.white, 'Set navigation target') then
		if mission.location:isa("Body") and mission.location:IsDynamic() then
			Game.player:SetNavTarget(target)
		elseif Game.system and mission.location:IsSameSystem(Game.system.path) then
			if mission.location.bodyIndex then
				Game.player:SetNavTarget(Space.GetBody(mission.location.bodyIndex))
			end
		elseif not Game.InHyperspace() then
			Game.player:SetHyperspaceTarget(mission.location:GetStarSystem().path)
		end
		ui.playBoinkNoise()
	end
	ui.text(string.format('Deadline: %s', Format.Date(mission.due)))
	ui.text(string.format('Wage: %s', Format.Money(mission.reward)))

	ui.text('')
	ui.nextColumn()
	clientFace:render()
	ui.columns(1, "", false)
end)