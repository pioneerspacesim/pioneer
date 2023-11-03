-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local GameParam = require 'pigui.modules.new-game-window.game-param'
local Character = require 'Character'
local PiGuiFace = require 'pigui.libs.face'
local Lang = require 'Lang'
local ui = require 'pigui'
local lui = Lang.GetResource("ui-core")
local utils = require 'utils'
local NameGen = require 'NameGen'
local Format = require 'Format'
local Defs = require 'pigui.modules.new-game-window.defs'
local Widgets = require 'pigui.modules.new-game-window.widgets'
local Table = require 'pigui.libs.table'

local Crew

--
-- player char
--
-- value: Character
--
local PlayerChar = GameParam.New(lui.CHARACTER, "player.char")
PlayerChar.value = Character.New({ title = lui.COMMANDER })
PlayerChar.value.player = true
PlayerChar.face = false
PlayerChar.crewEntry = { valueRef = { char = PlayerChar.value }}

function PlayerChar:random()
	self.value.char = Character.New()
	self.value.char:RollNew(true)
	self.face = PiGuiFace.New(self.value.char, nil, false)
	self.face.style.showCharInfo = false
end

function PlayerChar:fromStartVariant(variant)
	self:setLock(false)
end

function PlayerChar:isValid()
	return #self.value.name > 0 and #self.value.name < 50
end

--
-- player log
--
-- value: string
--
local PlayerLog = GameParam.New(lui.LOG_CUSTOM, "player.log")

function PlayerLog:fromStartVariant(variant)
	self.value = variant.logmsg
end

function PlayerLog:isValid()
	return true
end

--
-- player money
--
-- value: number
--
local PlayerMoney = GameParam.New(lui.MONEY, "player.money")

PlayerMoney.value = 0

function PlayerMoney:draw()
	Widgets.alignLabel(lui.CASH, Crew.layout, function()
		local value, changed = Widgets.incrementDrag(self.lock, "##playermoney", self.value, math.max(self.value * 0.01, 1), 0, 1e15, Format.Money(self.value, false))
		if changed then
			self.value = value
		end
	end)
end

function PlayerMoney:fromStartVariant(variant)
	PlayerMoney.value = variant.money
	PlayerMoney:setLock(true)
end

function PlayerMoney:isValid()
	return self.value >= 0
end

--
-- player reputation
--
-- value: number
--
local PlayerReputation = GameParam.New(lui.REPUTATION, "player.reputation")

PlayerReputation.value = 0

PlayerReputation.layout = {}

-- create an array of levels corresponding to the given name of the interval
-- the element must be less than its value -> greater than or equal to the previous value
PlayerReputation.values = { Character.reputations[2][2] - 5 }
for i = 2, #Character.reputations do
	table.insert(PlayerReputation.values, Character.reputations[i][2])
end

PlayerReputation.names = {}
for _, v in ipairs(Character.reputations) do
	table.insert(PlayerReputation.names, lui[v[1]])
end

PlayerReputation.selected = utils.getIndexFromIntervals(Character.reputations, PlayerReputation.value)

function PlayerReputation:draw()
	Widgets.alignLabel(lui.REPUTATION, self.layout, function()
		local changed, ret = Widgets.combo(self.lock, "##reputation", self.selected - 1, self.names)
		if changed then
			self.selected = ret + 1
			self.value = PlayerReputation.values[self.selected]
		end
	end)
end

function PlayerReputation:random()
	self.value = utils.chooseEqual(self.values)
end

function PlayerReputation:fromStartVariant(variant)
	PlayerReputation.value = 0
	PlayerReputation:setLock(true)
end

function PlayerReputation:isValid()
	return self.value > -20 and self.value < 1000
end

--
-- player combat rating
--
-- value: number
--
local PlayerRating = GameParam.New(lui.RATING, "player.rating")

PlayerRating.value = 0

-- create an array of levels corresponding to the given name of the interval
-- the element must be less than its value -> greater than or equal to the previous value
PlayerRating.values = { Character.combatRatings[2][2] - 5 }
for i = 2, #Character.combatRatings do
	table.insert(PlayerRating.values, Character.combatRatings[i][2])
end

PlayerRating.names = {}
for _, v in ipairs(Character.combatRatings) do
	table.insert(PlayerRating.names, lui[v[1]])
end

PlayerRating.selected = utils.getIndexFromIntervals(Character.combatRatings, PlayerRating.value)

function PlayerRating:draw()
	Widgets.alignLabel(lui.RATING, Crew.layout, function()
		local changed, ret = Widgets.combo(self.lock, "##combatrating", self.selected - 1, self.names)
		if changed then
			self.selected = ret + 1
			self.value = self.values[self.selected]
		end
	end)
end

function PlayerRating:random()
	self.value = utils.chooseEqual(self.values)
end

function PlayerRating:fromStartVariant(variant)
	PlayerRating.value = 0
	PlayerRating:setLock(true)
end

function PlayerRating:isValid()
	return self.value >= 0 and self.value < 10000
end


--
-- Crew
--
-- value: see below
--
Crew = GameParam.New(lui.CREW, "crew")

local function createCrewMember()
	local char = Character.New()
	char:RollNew(true)
	return {
		char = char,
		wage = 100,
	}
end

Crew.value = {
	createCrewMember(),
	createCrewMember(),
	createCrewMember(),
}

Crew.currentChar = 1
Crew.list = false
Crew.layout = {}

local function crewTraits(lock, char, tableWidth)
	local even = 0
	for _, i in ipairs{ 1, 6, 2, 7, 3, 8, 4, 9, 5 } do
		if even % 2 == 1 then ui.sameLine() end
		local param = Defs.charParams[i]
		ui.nextItemWidth(tableWidth / 2 - ui.getItemSpacing().x / 2)
		local value, changed = Widgets.incrementDrag(lock, "##crewparamdrag"..tostring(i).."_"..tostring(i), char[param], 1, Defs.charInterval[1], Defs.charInterval[2], lui[string.upper(param)], true)
		if changed then
			char[param] = value
		end
		even = even + 1
	end
end

function Crew:removeMember(member)
	for k, v in pairs(self.value) do
		if v == member.valueRef then
			table.remove(self.value, k)
			-- remember that there is no player in self.value
			self.currentChar = #self.value > k and k + 1 or k
			break
		end
	end
	self:updateTableItems()
end

function Crew:initMemberFace(member)
	if member.face then return end
	member.face = PiGuiFace.New(member.valueRef.char, nil, false)
	member.face.style.showCharInfo = false
end

local function isCrewMemberValid(member)
	return #member.char.name > 0 and member.wage > 0
end

function Crew:isValid()
	local valid = true
	for _, member in pairs(self.value) do
		valid = valid and isCrewMemberValid(member)
	end
	return valid
end

local buttonLayout = {}

function Crew:drawMember(member)

	self:initMemberFace(member)

	local faceGenButtonsSize = PiGuiFace.getFaceGenButtonsSize()
	local crewPictureSize = faceGenButtonsSize.y

	-- common lock for the whole crew except for the player
	local lock, valid
	if member.valueRef.char.player then
		lock = PlayerChar.lock
		valid = PlayerChar:isValid()
	else
		lock = self.lock
		valid = member:isValid()
	end

	if not lock then
		member.face:renderFaceGenButtons(true)
	else
		ui.dummy(faceGenButtonsSize)
	end

	ui.sameLine()

	ui.child("Face", Vector2(crewPictureSize, crewPictureSize), {}, function()
		member.face:renderFaceDisplay()
	end)

	ui.sameLine()

	local margin = Defs.gap.x * 3
	self.layout.width = ui.getContentRegion().x - margin * 2
	self.layout.height = faceGenButtonsSize.y
	PlayerReputation.layout.width = self.layout.width
	ui.sameLine(0, margin + ui.getWindowPadding().x)
	ui.child("character_params", Vector2(0, crewPictureSize), function()
		Widgets.verticalCenter(self.layout, function()
			Widgets.alignLabel(lui.NAME_PERSON, self.layout, function()
				local txt, changed = Widgets.inputText(lock, valid, "##charname" .. tostring(member), tostring(member.valueRef.char.name), function()
					member.valueRef.char.name = NameGen.FullName(member.valueRef.char.female)
				end)
				if changed then
					member.valueRef.char.name = txt
				end
			end)
			if member.valueRef.char.player then
				PlayerMoney:draw()
				PlayerRating:draw()
				PlayerReputation:draw()
			else
				Widgets.alignLabel(lui.WAGE, self.layout, function()
					local value, changed = Widgets.incrementDrag(lock, "##crewwage", member.valueRef.wage, 1, 0, 100000, Format.Money(member.valueRef.wage, false))
					if changed then
						member.valueRef.wage = value
					end
				end)
				ui.text("")
				crewTraits(lock, member.valueRef.char, self.layout.width)
				if not lock then
					ui.text("")
					buttonLayout.width = self.layout.width
					Widgets.horizontalCenter(buttonLayout, function()
						if ui.button(lui.DISMISS) then
							local popup = Widgets.ConfirmPopup()
							popup.drawQuestion = function()
								ui.text(lui.DISMISS)
							end
							popup.yesAction = function()
								self:removeMember(member)
							end
							popup:open()
						end
					end)
				end
			end
		end)
	end)
end

-- the function can run outside the imgui frame
function Crew:updateTableItems()
	if not self.list then return end
	self.list.items = { PlayerChar.crewEntry }
	for _,v in pairs(self.value) do
		local crewMemberEntry = {
			valueRef = v,
			lock = true,
			valid = true,
			isValid = function(selfInternal) return isCrewMemberValid(selfInternal.valueRef) end
		}
		self:initMemberFace(crewMemberEntry)
		table.insert(self.list.items, crewMemberEntry)
	end
	if not self.lock then
		-- "+" element
		table.insert(self.list.items, { addNewCharacter = true })
	end
	self.list.selectedItem = self.list.items[self.currentChar]
end

function Crew:updateLayout()

	local namesWidth
	ui.withFont(Defs.mainFont, function()
		namesWidth = ui.calcTextSize("-Johannes Samuelsson-").x
	end)

	if not self.list then
		self.list = Table.New("CrewListTable", false, {
			columnCount = 1,
			initTable = function(selfInternal)
				ui.setColumnWidth(0, namesWidth)
			end,
			renderItem = function(selfInternal, item, key)
				if item.addNewCharacter then ui.text("+") ui.nextColumn() return end

				local spacing = ui.getItemSpacing()
				ui.withStyleVars({ItemSpacing = spacing}, function()
					ui.withFont(Defs.mainFont, function()
						ui.text(item.valueRef.char.name)
					end)

					ui.withFont(Defs.subFont, function()
						ui.text(item.valueRef.char.title or lui.GENERAL_CREW)
					end)

					ui.nextColumn()
				end)
			end,
			onClickItem = function(selfInternal, item, key)
				if item.addNewCharacter then
					table.insert(self.value, createCrewMember())
					self.currentChar = key
					selfInternal.selectedItem = self.value[#self.value - 1]
					self:updateLayout()
				else
					self.currentChar = key
					selfInternal.selectedItem = item
				end
			end,
			iterator = ipairs
		})
	end

	self.list.style.size = Vector2(namesWidth, Defs.contentRegion.y)
end

function Crew:updateParams()
	self:updateTableItems()
end

function Crew:draw()
	if self.list then
		self.list:render()
		ui.sameLine()
		self:drawMember(self.list.selectedItem)
	end
end

function Crew:fromStartVariant(variant)
	Crew.value = {
--		createCrewMember(),
--		createCrewMember()
	}
	Crew.currentChar = 1
	Crew:setLock(true)
end

Crew.TabName = lui.CREW
Crew.Player = {
	Char = PlayerChar,
	Money = PlayerMoney,
	Reputation = PlayerReputation,
	Rating = PlayerRating,
	Log = PlayerLog
}

return Crew
