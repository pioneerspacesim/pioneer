-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

--
-- Class: Character
--
-- A character sheet, used to keep characteristics of any non-player character
-- which should be persistent, allowing re-use of characters over time and
-- across Lua scripts.
--
-- Additionally, one character sheet is initialised to keep the player's
-- information.  Attributes in that character sheet begin as default values,
-- and should be modified by the player's conduct in the game.
--
-- Character is inspired by the GURPS table-top roleplaying system.  In addition
-- to details such as the character's name, face, rank and so forth are a
-- number of attributes.  These attributes usually range from 4 to 64, although
-- lower and higher numbers are valid.  The attributes define the persona of
-- the character, and represent the chance of that character's success in
-- various endeavours.  These chances are tested by the virtual roll of four
-- sixteen-sided dice.  The target is to "roll under" for success.
--
-- So, if a character is trying to steal something, and there is an attribute
-- called "theft" (hypothetical as I write), then we can determine whether that
-- character succeeds by rolling the dice.  A character who is a good thief
-- would have a high number in their "theft" attribute, making it more likely
-- that the dice roll result will be lower.  If it's lower, the theft succeeds.
--
-- Character sheets inherit their methods, and many default values, from the
-- class itself.
--
-- Whilst some role-playing attributes are defined here, and can be given some
-- random values using RollNew(), in practice any arbitrary attribute can be
-- added to a character on demand, and these attributes can be safely tested
-- on any character, whether defined or not.  Undefined values return failure
-- if tested using TestRoll().
--
-- *Saving characters*
--
-- Within the Character class there is a table called "persistent".  Character
-- sheets can be stored in this table using the Save() method.  This method
-- checks whether the character sheet already exists in that table, and also
-- updates the values of the persistence members (see below).  Saved characters
-- become available to other scripts.
--
-- Saved characters are indexed numerically, and can be retrieved with ipairs()
-- and counted with the # operator.  The player's sheet is stored as
-- Character.persistent.player, and is the special case.

local Engine = require 'Engine'
local Game = require 'Game'
local Event = require 'Event'
local NameGen = require 'NameGen'
local Serializer = require 'Serializer'
local utils = require 'utils'

local Character;
Character = {

--
-- Group: Attributes
--

--
-- Attribute: player
--
-- Boolean value; true if this instance of the table is for the player.  Only
-- one character sheet should be that of the player.  Useful if there's the
-- slightest chance of the player's own sheet getting mixed up with those of
-- NPCs.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	player = false, -- Almost always.  One exception. (-:

--
-- Attribute: name
--
--   Name of character
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
-- Attribute: female
--
--   Gender of character.  If true, character is female.  If false, male.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
-- Attribute: seed
--
--   Seed for predictable randomness, if one is required.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
-- Attribute: title
--
--   Job title, for use in BBS faces
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--

--
-- Attribute: available
--
--   Boolean value; whether this character is available for use by a mission.
--   Used to "lock" a persistent character, so that two missions don't use the
--   same character at the same time.  Not particularly meaningful for transient
--   characters.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	available = true,

--
-- Attribute: useCount
--
--   Count of the number of times a script has used this character.  Used to
--   determine how well used a character is.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	useCount = 0,

--
-- Attribute: lastSavedTime
--
--   The game time, in seconds since 12:00 01-01-3200, that this character was
--   last saved.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	lastSavedTime = 0,

--
-- Attribute: lastSavedSystemPath
--
--   The system path of the station or system at which this character was
--   last saved.  Save() sets this to that of Game.player, since character
--   sheets have no intrinsic location.  Can be directly set immediately after
--   the call to Save() if it needs to be some other path.
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	lastSavedSystemPath = nil,

--
-- Attribute: dead
--
-- Boolean attribute. If set to a true value, character is deceased and all test rolls will fail.
--
-- Availability:
--
--   alpha 31
--
-- Status:
--
--   experimental
--

--
-- Attribute: playerRelationship
--
-- Integer attribute for RPG style dice tests.  PlayerRelationship is
-- intended to reflect the character's willingness to accommodate the player.
-- Tested with 4xD16; useful values are 4 (despise) to 65 (adore).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	playerRelationship = 34,

--
-- Attribute: luck
--
-- Integer attribute for RPG style dice tests.  Luck is intended to reflect
-- the character's innate good fortune.
-- Tested with 4xD16; useful values are 4 (never lucky) to 65 (always lucky).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	luck = 34,

--
-- Attribute: intelligence
--
-- Integer attribute for RPG style dice tests.  Intelligence is intended to
-- reflect the character's ability to learn.
-- Tested with 4xD16; useful values are 4 (moron) to 65 (genius).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 30
--
-- Status:
--
--   experimental
--
	intelligence = 34,

--
-- Attribute: charisma
--
-- Integer attribute for RPG style dice tests.  Charisma is intended to reflect
-- the character's ability to win contracts or favrouable deals.
-- Tested with 4xD16; useful values are 4 (always hated) to 65 (always liked).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	charisma = 34,

--
-- Attribute: notoriety
--
-- Integer attribute for RPG style dice tests.  Notoriety is intended to reflect
-- how well the character's reputation (good or bad) is known.
-- Tested against 4xD16; useful values are 4 (complete nobody) to 65 (celebrity).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	notoriety = 15, -- Few people are notorious, so a low default

--
-- Attribute: lawfulness
--
-- Integer attribute for RPG style dice tests.  Lawfulness is intended to reflect
-- the character's willingness to break the law.
-- Tested with 4xD16; useful values are 4 (going straight) to 65 (criminal).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	lawfulness = 34,

--
-- Attribute: engineering
--
-- (Crew skill)
--
-- Integer attribute for RPG style dice tests.  Engineering is intended to reflect
-- the character's mechanical, electrical or other tecnical skills.  Tests might
-- be made against this attribute to see whether a character can repair a
-- damaged piece of equipment, partially repair a ship, fit new equipment and
-- so on.
-- Tested with 4xD16; useful values are 4 (always fails) to 65 (always succeeds).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	engineering = 15,

--
-- Attribute: piloting
--
-- (Crew skill)
--
-- Integer attribute for RPG style dice tests.  Piloting is intended to reflect
-- the character's skill at flying spacecraft.
-- Tested with 4xD16; useful values are 4 (always crashes) to 65 (aerobat).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	piloting = 15,

--
-- Attribute: navigation
--
-- (Crew skill)
--
-- Integer attribute for RPG style dice tests.  Navigation is intended to reflect
-- the character's skill at course plotting, mapping and so on.  This might be
-- tested to see whether a ship can gain additional range on a hyperspace jump,
-- by having this character involved, or if, during a mission, a
-- character succeeds in identifying a location based on clues, etc.
-- Tested with 4xD16; useful values are 4 (always lost) to 65 (human compass).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	navigation = 15,

--
-- Attribute: sensors
--
-- (Crew skill)
--
-- Integer attribute for RPG style dice tests.  Sensors is intended to reflect
-- the character's ability to get the most from a ship's scanner, radar, etc.
-- This skill might be tested to see if a hidden ship can be found, or an
-- unknown item in space can be identified.
-- Tested with 4xD16; useful values are 4 (blind) to 65 (no hiding from this guy).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	sensors = 15,

--
-- Attribute: killcount
--
-- Integer value; number of objects destroyed by this character. Automatically
-- incremented in the case of the player by one of the stock scripts.
--
-- Availability:
--
--   alpha 29
--
-- Status:
--
--   experimental
--
	killcount = 0,

--
-- Attribute: assistcount
--
-- Integer value; number of objects damaged by this character which were
-- subsequently destroyed by something else. Automatically incremented
-- in the case of the player by one of the stock scripts.
--
-- Availability:
--
--   alpha 29
--
-- Status:
--
--   experimental
--
	assistcount = 0,

--
-- Attribute: reputation
--
-- Integer value; reflects the reputation amassed through taking missions.
-- It should be reduced for failed missions, and increased for successful
-- missions.
--
-- Availability:
--
--   January 2014
--
-- Status:
--
--   experimental
--
	reputation = 0,

--
-- Group: Methods
--

--
-- Method: New
--
-- Returns a new character sheet, inheriting from Character, based on an
-- optional template.
--
-- character = Character.New(template,rand)
--
-- Return:
--
--   character - a character sheet, which inherits from Character class
--
-- Parameters:
--
--   template - (optional) a table containing default values
--   rand     - (optional) the Rand object to use to generate values. If omitted, Engine.rand will be used
--
-- Example:
--
-- > regular_joe = Character.New()
--
-- > lucky_guy = Character.New({luck = 180})
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	New = function (template,rand)
		if template and (type(template) ~= 'table') then
			error("Character template must be a table")
		end
		if rand and (type(rand) ~= 'userdata') then
			error("Character's Rand isn't correct")
		end
		local rand = rand or Engine.rand
		local test = getmetatable(template)
		if(test and (test.class == 'Character')) then
			error("Won't use a character as a template.  use Clone() instead.")
		end
		-- initialise new character
		local newCharacter = {}
		if template then
			for k,v in pairs(template) do
				newCharacter[k] = v
			end
		end
		-- set inherited characteristics (inherit from class only, not self)
		setmetatable(newCharacter,Character.meta)
		-- initialize face characteristics if they weren't fully specified
		if newCharacter.female == nil then newCharacter.female = (rand:Integer(1) ==1) end
		newCharacter.name = newCharacter.name or NameGen.FullName(newCharacter.female,rand)
		newCharacter.seed = newCharacter.seed or rand:Integer()
		newCharacter.armour = newCharacter.armour or false
		newCharacter.player = false -- Explicitly set this, if you need it.
		return newCharacter
	end,

--
-- Method: Clone
--
-- Clones a character sheet.
--
-- clone = original:Clone()
--
-- Return:
--
--   clone - a new character sheet, which inherits from Character class
--           and is based on the original
--
-- Example:
--
-- > -- How to clone lucky_guy
-- > new_guy = lucky_guy:Clone()
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	Clone = function (self)
		local test
		local clone
		if type(self) == 'table' then
			test = getmetatable(self)
		else
			if self then
				error("Clone() expects a character.")
			else
				error("Clone() expects a character.  Did you miss a colon?")
			end
		end
		if(test and (test.class == 'Character')) then
			-- We've been handed an actual character as a constructor argument.
			-- We'll duplicate him.
			clone = {}
			for k,v in pairs(self) do
				if type(v) ~= 'table' then -- don't want to share sub-tables
					clone[k] = v
				end
			end
		else
			error("Can't clone something that isn't a character.")
		end
		-- initialise new character
		-- set inherited characteristics (inherit from class only, not self)
		clone.player = false -- Just in case we cloned the player...
		setmetatable(clone,Character.meta)
		return clone
	end,

--
-- Method: DiceRoll
--
-- Returns the results of a simulated 4xD16 roll.  Results are random, but
-- distributed in a bell curve about the value 34.  The minimum result is
-- 4, and the maximum result is 64.
--
-- > roll = Character.DiceRoll()
--
-- Return:
--
--   roll - Integer value between 4 and 265 (inclusive), most likely 34
--
-- Example:
--
-- > new_character.charisma = Character.DiceRoll()
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	DiceRoll = function ()
		return ( -- 4xD16, range is 4..64 averaging 34)
			  Engine.rand:Integer(1,16)
			+ Engine.rand:Integer(1,16)
			+ Engine.rand:Integer(1,16)
			+ Engine.rand:Integer(1,16)
		)
	end,

--
-- Method: RollNew
--
-- Uses DiceRoll to initialise the following attributes to random values:-
--   luck,
--   charisma,
--   notoriety,
--   lawfulness.
--
-- If optional parameter "crew" is specified and is true, then also:-
--   engineering,
--   piloting,
--   navigation,
--   sensors.
--
-- Parameters:
--
--   crew - If true, then RollNew() also sets random crew skills, which will
--          normally become much higher than the default values.
--
-- Example:
--
-- > new_character:RollNew()
--
-- > new_character:RollNew(true)
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	RollNew = function (self,crew)
		self.luck = self.DiceRoll()
		self.intelligence = self.DiceRoll()
		self.charisma = self.DiceRoll()
		self.notoriety = self.DiceRoll()
		self.lawfulness = self.DiceRoll()
		if crew then
			self.engineering = self.DiceRoll()
			self.piloting = self.DiceRoll()
			self.navigation = self.DiceRoll()
			self.sensors = self.DiceRoll()
		end
	end,

--
-- Method: TestRoll
--
-- Uses DiceRoll to generate a random number, which it compares with the provided
-- attribute.  If the generated number is less than the sum of the attribute and
-- the provided modifier, it returns true.
-- If it is greater, or the attribute does not exist, it returns false.
--
-- > success = somebody:TestRoll('notoriety')
--
-- If the DiceRoll is from 4-8, then it was a critical failure and that
-- attribute is abused.  It is decremented by one for future tests.
--
-- If the DiceRoll is from 60-64, then it was a critical success and that
-- attribute is exercised.  It is incremented by one for future tests.
--
-- If the Character is dead, the test roll will always fail.
--
-- Return:
--
--   success - False if failed, otherwise the dice roll value (low is good).
--
-- Parameters:
--
--   attribute - The key of an attribute in this instance of the character table
--               (such as luck, charisma, or any arbitrarily added attribute)
--
--   modifier - An arbitrary integer used to increase or decrease the odds of
--              returning true or false.  Positive values increase the odds of
--              a true result, and negative values increase the odds of false.
--              Default is zero.
--
-- Example:
--
-- > if (player:TestRoll('lawfulness',20)) then Comms.Message('A fellow criminal!')
--
	TestRoll = function (self,attribute,modifier)
		local modifier = modifier or 0
		if type(modifier) ~= 'number' then error('TestRoll(): modifier must be numeric') end
		if self.dead then return false end -- dead characters fail all tests
		if self[attribute] and (type(self[attribute])=='number') then
			local result = self.DiceRoll()
			if result > 59 then -- punish critical failure
				self[attribute] = self[attribute] - 1
				modifier = modifier + 1 -- don't affect *this* result
			elseif result < 9 then -- reward critical success
				self[attribute] = self[attribute] + 1
				modifier = modifier - 1 -- don't affect *this* result
			end
			return (result < (self[attribute] + modifier) and result)
		else
			return false
		end
	end,

--
-- Method: SafeRoll
--
-- Uses DiceRoll to generate a random number, which it compares with the provided
-- attribute.  If the generated number is less than the sum of the attribute and
-- the provided modifier, it returns true.
-- If it is greater, or the attribute does not exist, it returns false.
--
-- > success = somebody:SafeRoll('notoriety')
--
-- Unlike TestRoll, this function never modifies the value of the attribute.
--
-- If the Character is dead, the test roll will always fail.
--
-- Return:
--
--   success - False if failed, otherwise the dice roll value (low is good).
--
-- Parameters:
--
--   attribute - The key of an attribute in this instance of the character table
--               (such as luck, charisma, or any arbitrarily added attribute)
--
--   modifier - An arbitrary integer used to increase or decrease the odds of
--              returning true or false.  Positive values increase the odds of
--              a true result, and negative values increase the odds of false.
--              Default is zero.
--
-- Example:
--
-- > if (player:SafeRoll('lawfulness',20)) then Comms.Message('A fellow criminal!')
--
	SafeRoll = function (self,attribute,modifier)
		local modifier = modifier or 0
		if type(modifier) ~= 'number' then error('SafeRoll(): modifier must be numeric') end
		if self.dead then return false end -- dead characters fail all tests
		if self[attribute] and (type(self[attribute])=='number') then
			local result = self.DiceRoll()
			return (result < (self[attribute] + modifier) and result)
		else
			return false
		end
	end,

--
--
-- Method: Save
--
-- If the character is not already in the table of persistent characters, inserts
-- the character into that table.  Also set available to true, and update the
-- timestamp and location members.
--
-- Return:
--
--   Index of this character in Character.persistent table
--
-- Parameters:
--
--   attribute - The key of an attribute in this instance of the character table
--               (such as luck, charisma, or any arbitrarily added attribute)
--
--   modifier - An arbitrary integer used to increase or decrease the odds of
--              returning true or false.  Positive values increase the odds of
--              a true result, and negative values increase the odds of false.
--              Default is zero.
--
-- Example:
--
--   local BBS_characterID = BBS_character:Save()
--
	Save = function (self)
		if self and (type(self) == 'table') then
			local test = getmetatable(self)
			if test and (test.class == 'Character') then
				self.available = true
				self.useCount = self.useCount + 1
				self.lastSavedTime = Game.time
				local station = Game.player:GetDockedWith()
				if station then
					self.lastSavedSystemPath = station.path
				else
					self.lastSavedSystemPath = Game.system.path
				end
				for i,NPC in ipairs(Character.persistent) do
					if NPC == self then return i end
				end
				table.insert(Character.persistent,self)
        		return #Character.persistent
			end
		end
		error('Cannot save character')
	end,

--
-- Method: Find
--
--   Returns an iterator across all persistent characters who match the
--   specified filter.
--
-- iterator = Character.Find(filter)
--
-- Parameters:
--
--   filter - an optional function.  If specified, the function will be
--            called once for each saved character with the Character object
--            as the only parameter.  If the filter function returns true then
--            the Character will be returned by the iterator, otherwise it will be
--            omitted.  If no filter function is specified then all Characters are
--            returned.
--
-- Return:
--
--   iterator - a function which will generate the returned results, returning
--              one each time it is called until it runs out, after which it
--              returns nil.
--
-- Example:
--
-- Print names of all female characters who have charisma > 36
--
-- > for party_girl in Character.Find( function (NPC)
-- >                                     return (NPC.female and (NPC.charisma > 36))
-- >                                   end)
-- > do
-- >   print(party_girl.name)
-- > end
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	Find = function (filter)
		-- We want a nice default filter
		local filter = filter or function () return true end
		if type(filter) ~= 'function' then
			error('Character.Find() expected a function or nil')
		end
		local NPC = 0
		return function ()
			NPC = NPC + 1
			while Character.persistent[NPC] and (not filter(Character.persistent[NPC])) do
				NPC = NPC + 1
			end
			return Character.persistent[NPC]
		end
	end,

--
-- Method: FindAvailable
--
--   Returns an iterator across all persistent characters where available is true
--   and dead is false
--
-- iterator = Character.FindAvailable()
--
-- Return:
--
--   iterator - a function which will generate the returned results, returning
--              one each time it is called until it runs out, after which it
--              returns nil.
--
-- Example:
--
-- Print names of all characters
--
-- > for person in Character.FindAvailable()
-- > do
-- >   print(person.name)
-- > end
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	FindAvailable = function ()
		-- We want a nice default filter
		local NPC = 0
		return function ()
			NPC = NPC + 1
			while Character.persistent[NPC] and (not (Character.persistent[NPC]).available or (Character.persistent[NPC]).dead) do
				NPC = NPC + 1
			end
			return Character.persistent[NPC]
		end
	end,

--
-- Method: CheckOut
--
--   "Checks out" a persistent character, flagging it for exclusive use.  Sets
--   available to false, meaning that FindAvailable() will not return this
--   character.
--
--   ch:CheckOut()
--
--   A character is checked back in using Save().
--
-- Return:
--
--   true - The character was available, and has been marked unavailable
--
--   false - The character wasn't available, or didn't exist
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	CheckOut = function (self)
		if self and (type(self) == 'table') then
			local test = getmetatable(self)
			if test and (test.class == 'Character') and self.available then
				self.available = false
				return true
			end
		end
		return false
	end,

--
-- Method: UnSave
--
--   Removes a character from the persistent characters table
--
-- ch:UnSave()
--
-- Availability:
--
--   alpha 17
--
-- Status:
--
--   experimental
--
	UnSave = function (self)
		for num,NPC in ipairs(Character.persistent) do
			if NPC == self then table.remove(Character.persistent,num) end
		end
	end,

	combatRatings = {
		{ 'HARMLESS'            },
		{ 'MOSTLY_HARMLESS',  8 },
		{ 'POOR',            16 },
		{ 'AVERAGE',         32 },
		{ 'ABOVE_AVERAGE',   64 },
		{ 'COMPETENT',      128 },
		{ 'DANGEROUS',      512 },
		{ 'DEADLY',        2400 },
		{ 'ELITE',         6000 }
	},

--
-- Method: GetCombatRating
--
--   Returns a translatable string giving the character's combat rating
--
-- rating = ch:GetCombatRating()
--
-- Return:
--
--   rating - Translatable string
--
-- Example:
--
-- Show player their own combat rating as a UI message
--
-- > l = Lang.GetResource("ui-core")
-- > UI.Message(('Your combat rating is {rating}'):interp({
-- >     rating = l[Character.persistent.player:GetCombatRating(]),
-- > }))
--
-- Availability:
--
--   alpha 29
--
-- Status:
--
--   experimental
--
	GetCombatRating = function (self)
		return utils.getFromIntervals(self.combatRatings, self.killcount)
	end,

--
-- Method: IsCombatRated
--
--   Tests whether a character has reached a specific combat rating
--
-- ch:IsCombatRated(rating)
--
-- Parameters:
--
--   rating - One of the following values:
-- >         'HARMLESS','MOSTLY_HARMLESS','POOR','AVERAGE','ABOVE_AVERAGE',
-- >         'COMPETENT','DANGEROUS','DEADLY','ELITE'
--
-- Return:
--
--   true - Character has reached the specified rating
--
--   false - Character has not reached the specified rating (or specified
--          rating was not a valid value)
--
-- Example:
--
-- Check to see if the player is rated "Deadly" or higher
--
-- > if Character.persistent.player:IsCombatRated('DEADLY') then
-- >   DoSomethingDeadly() -- Player is rated "Deadly" or higher
-- > end
--
-- Availability:
--
--   alpha 29
--
-- Status:
--
--   experimental
--
	IsCombatRated = function (self,rating)
		-- This function is completely agnostic of the values of the ratings.
		local ratingflag = false
		local combatrating = self:GetCombatRating()
		for i,testrating in ipairs(self.combatRatings) do
			if testrating[1] == rating then
				-- We have reached the desired rating
				ratingflag = true
			end
			if testrating[1] == combatrating and ratingflag then
				-- The character's rating is equal to the one we've rached, and
				-- we have either reached or passed the desired rating
				return true
			end
		end --for
		return false
	end,

	reputations = {
		{ 'INCOMPETENT'        },
		{ 'UNRELIABLE',     -8 },
		{ 'NOBODY',          0 },
		{ 'INEXPERIENCED',   4 },
		{ 'EXPERIENCED',     8 },
		{ 'CREDIBLE',       16 },
		{ 'RELIABLE',       32 },
		{ 'TRUSTWORTHY',    64 },
		{ 'PROFESSIONAL',  128 },
		{ 'EXPERT',        256 },
		{ 'MASTER',        512 }
	},


--
-- Method: GetReputationRating
--
--   Returns a translatable string giving the character's reputation rating
--
-- rating = ch:GetReputationRating()
--
-- Return:
--
--   rating - Translatable string
--
-- Example:
--
-- Show player their own reputation rating as a UI message
--
-- > l = Lang.GetResource("ui-core")
-- > UI.Message(('Your reputation is {reputation}'):interp({
-- >     reputation = l[Character.persistent.player:GetReputationRating(]),
-- > }))
--
-- Availability:
--
--   January 2014
--
-- Status:
--
--   experimental
--
	GetReputationRating = function (self)
		return utils.getFromIntervals(self.reputations, self.reputation)
	end,

	-- Debug function
	PrintStats = function (self)
		print('Name: ',self.name)
		print('Luck: ',self.luck)
		print('Intelligence: ',self.intelligence)
		print('Charisma: ',self.charisma)
		print('Notoriety: ',self.notoriety)
		print('Lawfulness: ',self.lawfulness)
		print('Engineering: ',self.engineering)
		print('Piloting: ',self.piloting)
		print('Navigation: ',self.navigation)
		print('Sensors: ',self.sensors)
	end,

	Serialize = function (self)
		return self
	end,

	Unserialize = function (data)
		setmetatable(data,Character.meta)
		return data
	end,
}

-- Meta table to be given to all children
Character.meta = {
	__index = Character,
	class = "Character",
}

-- This will be a numerically indexed global table of characters.  There
-- will also be one non-numerically keyed value - ['player'].
Character.persistent = {}

-- We'll try to save our persistent characters...
local loaded_data

local onGameStart = function ()
	if loaded_data then
		for k,newCharacter in pairs(loaded_data.PersistentCharacters) do
			Character.persistent[k] = newCharacter
		end
	elseif not Character.persistent.player then
		-- Make a new character sheet for the player, with just
		-- the average values.  We'll find some way to ask the
		-- player for a new name in the future.
		local PlayerCharacter = Character.New()
		PlayerCharacter.title = 'Commander'
		PlayerCharacter.player = true
		-- Gave the player a missions table (for Misssions.lua)
		PlayerCharacter.missions = {}
		-- Insert the player character into the persistent character
		-- table.  Player won't be ennumerated with NPCs, because player
		-- is not numerically keyed.
		Character.persistent = { player = PlayerCharacter }
		-- Enroll the player in their own crew
		Game.player:Enroll(PlayerCharacter)
	end
	loaded_data = nil
end

local serialize = function ()
    return { PersistentCharacters = Character.persistent}
end

local function onGameEnd ()
	Character.persistent = {}
end

local unserialize = function (data)
    loaded_data = data
end

--
-- Group: Notes
--
--
-- Note: ChatForm.SetFace()
--
-- You can pass a character directly to the SetFace() method of a ChatForm.
--
-- Example:
--
-- > ch = Character.New()
-- > form:SetFace(ch)
--
-- Note: Arbitrary attributes
--
-- You are not limited to the documented attributes, and can define more on
-- the fly.  Any attribute is testable using TestRoll(), and will return false
-- if the attribute is undefined, meaning that it is not necessary to add an
-- attribute to all instances of Character.  This is handy for mission-specific
-- skills or attributes.
--
-- Example:
--
-- To define a new attribute, "geology," for testing planets:
--
-- > some_character.geology = 45 -- a fairly high value
--
-- To set it to a random amount:
--
-- > some_character.geology = Character.DiceRoll()
--
-- To test it:
--
-- > -- returns true or false, depending on chance
-- > if some_character:TestRoll('geology') then success() end
--
-- This will work for characters who have not had "geology" defined:
--
-- > -- always returns false
-- > if other_character:TestRoll('geology') then success() end
--

Event.Register("onGameStart", onGameStart)
Event.Register("onGameEnd", onGameEnd)
Serializer:Register("Characters", serialize, unserialize)
Serializer:RegisterClass("Character", Character)

return Character
