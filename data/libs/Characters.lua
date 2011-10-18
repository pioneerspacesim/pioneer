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

	new = function (self,character)
		-- initialise new character
		local character = character or {}
		-- preserve default name against randomization
		local name = character.name
		-- set inherited characteristics
		setmetatable(character,{__index = self})
		-- randomize name if it wasn't specified
		character.name = name or NameGen.FullName(character.isfemale)
		-- allocate a new table for character relationships
		character.Relationships = {}
		return character
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

	PrintStats = function (self)
		print('Name:',self.name)
		print('Luck:',self.luck)
		print('Charisma:',self.charisma)
		print('Notoriety:',self.notoriety)
		print('Lawfulness:',self.lawfulness)
	end,
}
