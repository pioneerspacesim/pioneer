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
local Helpers = require 'pigui.modules.new-game-window.helpers'
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

function PlayerChar:random()
	self.value = Character.New()
	self.value:RollNew(true)
end

function PlayerChar:fromStartVariant(variant)
	self.lock = false
end

function PlayerChar:isValid()
	return #self.value.name > 0 and #self.value.name < 50
end

PlayerChar.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local player, errorString = Helpers.getByPath(saveGame, {
			"lua_modules_json/Characters/PersistentCharacters/player",
			"lua_modules_json/ShipClass/$1/#1"
		})
		if errorString then return nil, errorString end
		return Character.Unserialize(player)
	end
}}

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
	PlayerMoney.lock = true
end

function PlayerMoney:isValid()
	return self.value >= 0
end

PlayerMoney.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		return Helpers.getByPath(saveGame, "lua_modules_json/Player/cash")
	end
}}

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
	PlayerReputation.lock = true
end

function PlayerReputation:isValid()
	return self.value > -20 and self.value < 1000
end

PlayerReputation.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		return Helpers.getByPath(saveGame, {
			"lua_modules_json/Characters/PersistentCharacters/player/reputation",
			"lua_modules_json/ShipClass/$1/#1/reputation"
		})
	end
}}

--
-- player combat killcount
--
-- value: number
--
local PlayerKills = GameParam.New(lui.KILLS, "player.kills")

PlayerKills.value = 0

-- We don’t want to show the kill counter to a new player at all, but if this
-- is a save recovery, we will show it, but it will be locked
PlayerKills.showKills = true

-- create an array of levels corresponding to the given name of the interval
-- the element must be less than its value -> greater than or equal to the previous value
PlayerKills.values = { Character.combatRatings[2][2] - 5 }
for i = 2, #Character.combatRatings do
	table.insert(PlayerKills.values, Character.combatRatings[i][2])
end

PlayerKills.names = {}
for _, v in ipairs(Character.combatRatings) do
	table.insert(PlayerKills.names, lui[v[1]])
end

function PlayerKills:draw()
	if not self.lock or self.showKills then
		Widgets.alignLabel(lui.KILLS, Crew.layout, function()
			local value, changed = Widgets.incrementDrag(self.lock, "##playerkills", self.value, 1, 0, 1e15, '%.0f')
			if changed then
				self.value = value
				self.selected = utils.getIndexFromIntervals(Character.combatRatings, self.value)
			end
		end)
	end
	Widgets.alignLabel(lui.RATING, Crew.layout, function()
		local changed, ret = Widgets.combo(self.lock, "##combatrating", self.selected - 1, self.names)
		if changed then
			self.selected = ret + 1
			self.value = self.values[self.selected]
		end
	end)
end

function PlayerKills:random()
	self.value = utils.chooseEqual(self.values)
end

function PlayerKills:fromStartVariant(variant)
	self.value = 0
	self.lock = true
	self.showKills = false
end

function PlayerKills:isValid()
	return self.value >= 0 and self.value < 10000
end

function PlayerKills:fromSaveGame(saveGame)
	self.showKills = true
	return GameParam.fromSaveGame(self, saveGame)
end

PlayerKills.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		return Helpers.getByPath(saveGame, {
			"lua_modules_json/Characters/PersistentCharacters/player/killcount",
			"lua_modules_json/ShipClass/$1/#1/killcount"
		})
	end
}}


--
-- Crew
--
-- value: array of Character (excluding player)
--
Crew = GameParam.New(lui.CREW, "crew")

local function createCrewMember()
	local char = Character.New()
	char:RollNew(true)
	char.contract = { wage = 100 }
	return char
end

Crew.value = {
	createCrewMember(),
	createCrewMember(),
	createCrewMember(),
}

Crew.currentChar = 1
Crew.list = false
Crew.listItems = {}
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
	for k, testMember in ipairs(self.value) do
		if member == testMember then
			table.remove(self.value, k)
			-- remember that there is no player in self.value
			self.currentChar = #self.value > k and k + 1 or k
			break
		end
	end
	self:fillCrewListTable()
end

function Crew:initMemberFace(memberEntry)
	if memberEntry.face then return end
	memberEntry.face = PiGuiFace.New(memberEntry.value, nil, false)
	memberEntry.face.style.showCharInfo = false
end

local function isCrewMemberValid(member)
	return #member.name > 0 and member.contract.wage > 0
end

function Crew:isValid()
	local valid = true
	for _, member in pairs(self.value) do
		valid = valid and isCrewMemberValid(member)
	end
	return valid
end

local buttonLayout = {}

function Crew:drawMember(memberEntry)

	self:initMemberFace(memberEntry)

	local faceGenButtonsSize = PiGuiFace.getFaceGenButtonsSize()
	local crewPictureSize = faceGenButtonsSize.y

	-- common lock for the whole crew except for the player
	local lock
	if memberEntry.value.player then
		lock = PlayerChar.lock
	else
		lock = self.lock
	end

	if not lock then
		memberEntry.face:renderFaceGenButtons(true)
	else
		ui.dummy(faceGenButtonsSize)
	end

	ui.sameLine()

	ui.child("Face", Vector2(crewPictureSize, crewPictureSize), {}, function()
		memberEntry.face:renderFaceDisplay()
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
				local txt, changed = Widgets.inputText(lock, memberEntry:isValid(), "##charname" .. tostring(memberEntry), tostring(memberEntry.value.name), function()
					memberEntry.value.name = NameGen.FullName(memberEntry.value.female)
				end)
				if changed then
					memberEntry.value.name = txt
				end
			end)
			if memberEntry.value.player then
				PlayerMoney:draw()
				PlayerKills:draw()
				PlayerReputation:draw()
			else
				local member = memberEntry.value
				Widgets.alignLabel(lui.WAGE, self.layout, function()
					local value, changed = Widgets.incrementDrag(lock, "##crewwage", member.contract.wage, 1, 0, 100000, Format.Money(member.contract.wage, false))
					if changed then
						member.contract.wage = value
					end
				end)
				ui.text("")
				crewTraits(lock, member, self.layout.width)
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

function Crew:initCrewListTable(namesWidth)
	-- visual table to display a flat list including the player and the rest of
	-- the crew
	-- item (memberEntry):
	--     isValid - function(memberEntry) -> boolean
	--     value -   Character or string 'ADD_NEW_CHARACTER' for '+' item
	--     face -    PiGuiFace
	--
	--     such an interface for an item allows you to put a PlayerChar directly,
	--     and also create an element referencing the crew member using the 'value' key
	--
	self.list = Table.New("CrewListTable", false, {
		columnCount = 1,
		initTable = function(selfInternal)
			ui.setColumnWidth(0, namesWidth)
		end,
		renderItem = function(selfInternal, memberEntry, key)
			if memberEntry.value == 'ADD_NEW_CHARACTER' then ui.text("+") ui.nextColumn() return end

			ui.withFont(Defs.mainFont, function()
				ui.text(memberEntry.value.name)
			end)

			ui.withFont(Defs.subFont, function()
				ui.text(memberEntry.value.title or lui.GENERAL_CREW)
			end)

			ui.nextColumn()
		end,
		onClickItem = function(selfInternal, memberEntry, key)
			self.currentChar = key
			if memberEntry.value == 'ADD_NEW_CHARACTER' then
				table.insert(self.value, createCrewMember())
				self:fillCrewListTable()
			else
				selfInternal.selectedItem = memberEntry
			end
		end,
		iterator = ipairs
	})
end

function Crew:fillCrewListTable()

	self.listItems = { PlayerChar }
	for _, member in ipairs(self.value) do
		local memberEntry = {
			isValid = function(entry) return isCrewMemberValid(entry.value) end,
			value = member
		}
		table.insert(self.listItems, memberEntry)
	end

	-- create faces, including player's
	for _, memberEntry in pairs(self.listItems) do
		memberEntry.face = PiGuiFace.New(memberEntry.value, nil, false)
		memberEntry.face.style.showCharInfo = false
	end

	if not self.lock then
		-- "+" element
		table.insert(self.listItems, { value = 'ADD_NEW_CHARACTER' })
	end
	if (self.list) then
		self.list.items = self.listItems
		self.list.selectedItem = self.listItems[self.currentChar]
	end
end

function Crew:updateLayout()

	local namesWidth
	ui.withFont(Defs.mainFont, function()
		namesWidth = ui.calcTextSize("-Johannes Samuelsson-").x
	end)

	if not self.list then
		self:initCrewListTable(namesWidth)
		self:fillCrewListTable()
	end
	self.list.style.size = Vector2(namesWidth, Defs.contentRegion.y)
end

function Crew:updateParams()
	self:fillCrewListTable()
	PlayerReputation.selected = utils.getIndexFromIntervals(Character.reputations, PlayerReputation.value)
	PlayerKills.selected = utils.getIndexFromIntervals(Character.combatRatings, PlayerKills.value)
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
	Crew.lock = true
end

Crew.reader = Helpers.versioned {{
	version = 89,
	fnc = function(saveGame)
		local crew, errorString = Helpers.getByPath(saveGame, "lua_modules_json/ShipClass/$1")
		if errorString then return nil, errorString end
		local value = {}
		-- the first element is the player - it is a separate parameter
		for i = 2, #crew do
			local char = Character.Unserialize(crew[i])
			table.insert(value, char)
		end
		return value
	end
}}

Crew.TabName = lui.CREW
Crew.Player = {
	Char = PlayerChar,
	Money = PlayerMoney,
	Reputation = PlayerReputation,
	Kills = PlayerKills,
}

return Crew
