-- Character class

Character = {
	-- Persona
	Player = false, -- Almost always.  One exception. (-:
	Name = '',
	Face = 0,

	-- Attributes
	Luck = 0,
	Charisma = 0,
	Notoriety = 0,
	Lawfulness = 0,

	-- Experience
	KillCount = 0,

	-- Methods

	new = function (self,character)
		local character = character or {}
		character.Relationships = {}
		setmetatable(character,{__index = self})
		return character
	end,

	SetName = function (self,name)
		self.Name = name or 'Test'
	end,

	PrintStats = function (self)
		print('Name:',self.Name)
		print('Luck:',self.Luck)
		print('Charisma:',self.Charisma)
		print('Notoriety:',self.Notoriety)
		print('Lawfulness:',self.Lawfulness)
	end,
}
