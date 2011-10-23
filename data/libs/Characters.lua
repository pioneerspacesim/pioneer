--
-- Class: Character
--

Character = {

--
-- Group: Attributes
--

--
-- Attribute: player
--
-- Boolean value; true if this instance of the table is for the player.  Only
-- one character sheet should be that of the player.
--
-- Availability:
--
--   future
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
--   future
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
--   future
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
--   future
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
--   future
-- 
-- Status:
-- 
--   experimental
-- 

--
-- Attribute: luck
--
-- Integer attribute for roll-play style dice tests.  Luck is intended to reflect
-- the character's innate good fortune.
-- Tested with 4xD16; useful values are 4 (never lucky) to 65 (always lucky).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	luck = 34,

--
-- Attribute: charisma
--
-- Integer attribute for roll-play style dice tests.  Charisma is intended to reflect
-- the character's ability to win contracts or favrouable deals.
-- Tested with 4xD16; useful values are 4 (always hated) to 65 (always liked).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	charisma = 34,

--
-- Attribute: notoriety
--
-- Integer attribute for roll-play style dice tests.  Notoriety is intended to reflect
-- how well the character's reputation (good or bad) is known.
-- Tested against 4xD16; useful values are 4 (complete nobody) to 65 (celebrity).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	notoriety = 15, -- Few people are notorious, so a low default

--
-- Attribute: lawfulness
--
-- Integer attribute for roll-play style dice tests.  Lawfulness is intended to reflect
-- the character's willingness to break the law.
-- Tested with 4xD16; useful values are 4 (never lucky) to 65 (always lucky).
-- Modifiers can cause numbers outside this range to become useful (see TestRoll).
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	lawfulness = 34,

--
-- Group: Methods
--

--
-- Method: New
--
-- Returns a new character sheet, inheriting from Character, based on an
-- optional template.
--
-- character = Character.New(template)
--
-- Return:
--
--   character - a character sheet, which inherits from Character class
--
-- Parameters:
--
--   template - (optional) a table containing default values
--
-- Example:
--
-- > regular_joe = Character.New()
--
-- > lucky_guy = Character.New({luck = 180})
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	New = function (template)
		if template and (type(template) ~= 'table') then
			error("Character template must be a table")
		end
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
		newCharacter.female = (newCharacter.female == nil) and (Engine.rand:Integer(1) ==1)
		newCharacter.name = newCharacter.name or NameGen.FullName(newCharacter.female)
		newCharacter.seed = newCharacter.seed or Engine.rand:Integer()
		newCharacter.armour = newCharacter.armour or false
		-- allocate a new table for character relationships
		newCharacter.Relationships = {}
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
--   future
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
--   future
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
-- Uses DiceRoll to initialise the following attributes to random values:
--   luck
--   charisma
--   notoriety
--   lawfulnes
--
-- Example:
--
-- > new_character:RollNew()
--
-- Availability:
--
--   future
--
-- Status:
--
--   experimental
--
	RollNew = function (self)
		self.luck = Character.DiceRoll()
		self.charisma = Character.DiceRoll()
		self.notoriety = Character.DiceRoll()
		self.lawfulness = Character.DiceRoll()
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
-- Return:
--
--   success - Boolean value indicating that the test roll passed or failed
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
-- > if (player:TestRoll('lawfulness',20)) then UI.Message('A fellow criminal!')
--
	TestRoll = function (self,attribute,modifier)
		local modifier = modifier or 0
		if self[attribute] then
			local result = Character.DiceRoll()
			if result < 9 then -- punish critical failure
				self[attribute] = self[attribute] - 1
				modifier = modifier + 1 -- don't affect *this* result
			elseif result > 59 then -- reward critical success
				self[attribute] = self[attribute] + 1
				modifier = modifier - 1 -- don't affect *this* result
			end
			return (result < (self[attribute] + modifier))
		else
			return false
		end
	end,

--
-- Method: Save
--
-- If the character is not already in the table of persistent characters, inserts
-- the character into that table.
--
-- Return:
--
--   Index of this character in PersistentCharacters table
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
				for i,NPC in ipairs(PersistentCharacters) do
					if NPC == self then return i end
				end
				table.insert(PersistentCharacters,self)
        		return #PersistentCharacters
			end
		end
		error('Cannot save character')
	end,


	-- Debug function
	PrintStats = function (self)
		print('Name:',self.name)
		print('Luck:',self.luck)
		print('Charisma:',self.charisma)
		print('Notoriety:',self.notoriety)
		print('Lawfulness:',self.lawfulness)
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
PersistentCharacters = {}

-- We'll try to save our persistent characters...
local loaded_data

local onGameStart = function ()
	if loaded_data then
		for k,newCharacter in pairs(loaded_data.PersistentCharacters) do
		--	setmetatable(newCharacter,{__index = Character})
			PersistentCharacters[k] = newCharacter
		end
	else
		-- Make a new character sheet for the player, with just
		-- the average values.  We'll find some way to ask the
		-- player for a new name in the future.
		local PlayerCharacter = Character.New({name = 'Peter Jameson', player = true})
		-- Insert the player character into the persistent character
		-- table.  Player won't be ennumerated with NPCs, because player
		-- is not numerically keyed.
		PersistentCharacters.player = PlayerCharacter
	end
	loaded_data = nil
end

local serialize = function ()
    return { PersistentCharacters = PersistentCharacters}
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
-- > ch = Character:New()
-- > form:SetFace(ch)
--

EventQueue.onGameStart:Connect(onGameStart)
Serializer:Register("Characters", serialize, unserialize)
