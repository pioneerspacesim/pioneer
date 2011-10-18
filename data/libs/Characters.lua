-- Character class

Character = {
	-- Persona
	player = false, -- Almost always.  One exception. (-:
	name = '',
	face = 0,

	-- Attributes
	luck = 0,
	charisma = 0,
	notoriety = 0,
	lawfulness = 0,

	-- Experience
	killCount = 0,

	-- Methods

	new = function (self,character)
		local character = character or {}
		character.Relationships = {}
		setmetatable(character,{__index = self})
		return character
	end,

	SetName = function (self,name)
		self.name = name or 'Test'
	end,

	PrintStats = function (self)
		print('Name:',self.name)
		print('Luck:',self.luck)
		print('Charisma:',self.charisma)
		print('Notoriety:',self.notoriety)
		print('Lawfulness:',self.lawfulness)
	end,
}
