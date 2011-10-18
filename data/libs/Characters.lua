Character = {
	-- Persona
	Name = '',
	Face = 0,

	-- Attributes
	Luck = 0,
	Charisma = 0,
	Notoriety = 0,
	Lawfulness = 0,

	-- Experience
	KillCount = 0,

	new = function (self,character)
		local character = character or {}
		character.Relationships = {}
		setmetatable(character,{__index = self})
		return character
	end
}
