-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

local utils = require 'utils'

local CultureName = {
	male = {},     -- List of 100 most common male first names
	female = {},   -- List of 100 most common female first names
	surname = {},  -- List of 100 most common last names
	name = "Name", -- Name of language / culture
	code = "xx",   -- ISO ISO 639-1 language code
	replace = {}   -- Non-ascii char to replacement char -table
}

CultureName.__index = CultureName

function CultureName.New (self)
	self = setmetatable(self or {}, CultureName)

	-- Populate self.surname_ascii
	self.surname_ascii = self:AsciiReplace(self.surname)

	return self
end

-- Generate an ASCII-replaced name table from the given input data and the
-- culture's character replacement data.
function CultureName:AsciiReplace(nametable)
	-- if replacement table empty, names are assumed to be already ascii
	if next(nametable) == nil then return end

	-- else, loop over all names and replacement chars
	local ascii = {}
	for _, name in ipairs(nametable) do
		-- print("Replacing name", name)
		for old, new in pairs(self.replace) do
			name = name:gsub(old, new)
		end
		table.insert(ascii, name)
	end

	return ascii
end

function CultureName:FirstName (isFemale, rand)
	local array = isFemale and self.female or self.male
	return utils.chooseEqual(array, rand)
end

-- Some cultures have gender specific surnames
function CultureName:Surname (isFemale, rand, ascii)
	local surname = ascii and self.surname_ascii or self.surname
	-- if ascii then print(#surname, surname == self.surname_ascii) end
	return utils.chooseEqual(surname, rand)
end

function CultureName:FullName (isFemale, rand)
	return self:FirstName(isFemale, rand) .. " " .. self:Surname(isFemale, rand)
end

return CultureName
