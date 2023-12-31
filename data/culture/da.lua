-- Copyright © 2008-2024 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- https://www.dst.dk/da/Statistik/emner/befolkning-og-valg/navne/navne-i-hele-befolkningen

local CultureName = require './common'

local male = {
	"Peter",
	"Michael",
	"Jens",
	"Lars",
	"Thomas",
	"Henrik",
	"Søren",
	"Christian",
	"Jan",
	"Martin",
	"Niels",
	"Anders",
	"Morten",
	"Jesper",
	"Mads",
	"Rasmus",
	"Hans",
	"Per",
	"Jørgen",
	"Ole"
}

local female = {
	"Anne",
	"Kirsten",
	"Mette",
	"Hanne",
	"Helle",
	"Anna",
	"Susanne",
	"Lene",
	"Maria",
	"Marianne",
	"Lone",
	"Camilla",
	"Pia",
	"Louise",
	"Charlotte",
	"Tina",
	"Gitte",
	"Bente",
	"Jette",
	"Karen"
}

local surname = {
	"Nielsen",
	"Jensen",
	"Hansen",
	"Andersen",
	"Pedersen",
	"Christensen",
	"Larsen",
	"Sørensen",
	"Rasmussen",
	"Jørgensen",
	"Petersen",
	"Madsen",
	"Kristensen",
	"Olsen",
	"Thomsen",
	"Christiansen",
	"Poulsen",
	"Johansen",
	"Møller",
	"Mortensen"
}

local Danish = CultureName.New({
	male = male,
	female = female,
	surname = surname,
	name = "Danish",
	code = "da",
	replace = {
		["æ"] = "a", ["Æ"] = "A",
		["ø"] = "o", ["Ø"] = "O",
		["å"] = "aa", ["Å"] = "Aa",
	}
})

return Danish
