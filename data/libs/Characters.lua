-- Character class

Character = {
	-- Persona
	player = false, -- Almost always.  One exception. (-:
	name = '',
	isfemale = true,
	face = 0,

	-- Attributes
	luck = 0,
	charisma = 0,
	notoriety = 0,
	lawfulness = 0,

	-- Experience
	killcount = 0,

	-- Methods

	new = function (self,character)
		local character = character or {}
		local name = character.name
		setmetatable(character,{__index = self})
		character.name = name or NameGen.FullName(character.isfemale)
		character.Relationships = {}
		return character
	end,

	PrintStats = function (self)
		print('Name:',self.name)
		print('Luck:',self.luck)
		print('Charisma:',self.charisma)
		print('Notoriety:',self.notoriety)
		print('Lawfulness:',self.lawfulness)
	end,
}
