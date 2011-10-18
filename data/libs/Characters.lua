-- Character class

Character = {
	-- Persona
	player = false, -- Almost always.  One exception. (-:
	name = '',
	isfemale = true,
	face = 0,

	-- Attributes
	luck = 130,
	charisma = 130,
	notoriety = 130,
	lawfulness = 130,

	-- Experience
	killcount = 0,

	-- Methods

	new = function (self,newCharacter)
		-- initialise new character
		local newCharacter = newCharacter or {}
		-- preserve default name/gender against randomization
		local name = newCharacter.name
		local isfemale = newCharacter.isfemale
		-- set inherited characteristics (inherit from class only, not self)
		setmetatable(newCharacter,{__index = Character})
		-- randomize name if it wasn't specified
		newCharacter.isfemale = (isfemale == nil) and (Engine.rand:Integer(1) ==1)
		newCharacter.name = name or NameGen.FullName(newCharacter.isfemale)
		-- allocate a new table for character relationships
		newCharacter.Relationships = {}
		return newCharacter
	end,

	DiceRoll = function ()
		return ( -- 4xD64, range is 4..256 averaging 130)
			  Engine.rand:Integer(1,64)
			+ Engine.rand:Integer(1,64)
			+ Engine.rand:Integer(1,64)
			+ Engine.rand:Integer(1,64)
		)
	end,

	RollNew = function (self)
		self.luck = self.DiceRoll()
		self.charisma = self.DiceRoll()
		self.notoriety = self.DiceRoll()
		self.lawfulness = self.DiceRoll()
	end,

	-- Save into persistent table of characters as an NPC
	Save = function (self)
		for i,NPC in ipairs(PersistentCharacters) do
			if NPC == self then return end
		end
		table.insert(PersistentCharacters,self)
	end,


	PrintStats = function (self)
		print('Name:',self.name)
		print('Luck:',self.luck)
		print('Charisma:',self.charisma)
		print('Notoriety:',self.notoriety)
		print('Lawfulness:',self.lawfulness)
	end,
}

-- This will be a numerically indexed global table of characters.  There
-- will also be one non-numerically keyed value - ['player'].
PersistentCharacters = {}

-- We'll try to save our persistent characters...
local loaded_data

local onGameStart = function ()
	if loaded_data then
		for k,newCharacter in pairs(loaded_data.PersistentCharacters) do
			setmetatable(newCharacter,{__index = Character})
			PersistentCharacters[k] = newCharacter
		end
	else
		-- Make a new character sheet for the player, with just
		-- the average values.  We'll find some way to ask the
		-- player for a new name in the future.
		local PlayerCharacter = Character:new({name = 'Peter Jameson', player = true})
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

EventQueue.onGameStart:Connect(onGameStart)
Serializer:Register("Characters", serialize, unserialize)
