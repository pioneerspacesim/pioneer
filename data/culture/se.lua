-- Copyright © 2008-2025 Pioneer Developers. See AUTHORS.txt for details
-- Licensed under the terms of the GPL v3. See licenses/GPL-3.txt

-- Sami names

-- References
-- https://en.wiktionary.org/wiki/Appendix:Northern_Sami_given_names
-- https://en.wikipedia.org/wiki/Category:S%C3%A1mi-language_surnames
-- https://www.samer.se/1916

local CultureName = require './common'

local male = {
    "Ailo", -- Intelligent or wise
    "Áillu", -- Variant of Ailo
    "Ánte", -- From Anton
    "Áron", -- From Aaron
    "Beaska", -- From Swedish 'Björk', meaning 'birch'
    "Biehtár", -- From Peter
    "Dure", -- From Ture
    "Ealá", -- Reindeer
    "Geir", -- Spear
    "Guhtur", -- From Guttorm
    "Jovnna", -- From John
    "Lávdni", -- Leaf, variant of Lávdna
    "Mielat", -- Meaning 'mind' or 'thought'
    "Niillas", -- From Nils
    "Ole", -- Common name, variant of Olaf
    "Piera", -- From Peter
    "Risten", -- From Christopher
    "Sámmol", -- From Samuel
    "Sáve", -- Protective spirit or guardian
}

local female = {
    "Áidná", -- Often associated with 'forever' or 'eternal'
    "Áile",
    "Álehtta", -- From Aletta
    "Álles", -- From Alice
    "Áire", -- From Airi
    "Ánne", -- From Anne
    "Biret", -- From Birgitte
    "Duová", -- From Tove
    "Elle", -- From Ellen or Elin
    "Ida", -- Common name
    "Lávdna", -- Meaning 'leaf'
    "Márjá", -- From Maria
    "Máret", -- From Margaret
    "Nienná", -- Possibly related to 'Nen' or 'woman'
    "Risten", -- From Christine
    "Ruvsá", -- Meaning 'dew'
    "Sárá", -- Variant of Sarah
    "Sáráhkká", -- Linked to a Sami goddess of fertility and home
    "Sárra", -- From Sarah
    "Sihkku", -- Meaning 'sister'
    "Unni",
}

local surname = {
	"Aikio",
    "Balto", -- From the name of a river or mountain in Norway
    "Eira",
    "Fofonoff",
	"Gaup",
	"Gauriloff",
    "Guttorm", -- Derived from Old Norse name elements
    "Hætta", -- Often associated with reindeer herders
    "Huuva",
    "Jannok",
    "Kalla", -- Derived from Kállá, the Sami form of Karl
    "Kroik",
    "Kuoljok",
    "Labba",
    "Larsen", -- Patronymic, son of Lars (example)
    "Magga",
    "Niia",
    "Niska", -- From Finnish 'niska' meaning neck, referring to an isthmus
    "Niva", -- From Finnish 'niva' meaning small rapid in a river
    "Nutti", -- Derived from the given name Knut
    "Omma",
    "Oskal", -- Often associated with reindeer herding
    "Porsanger", -- From a place name in Norway
    "Rimpi",
    "Sara",
    "Simma",
    "Somby", -- Derived from the name of the village Sompio in Finland
    "Svonni",
    "Sunna",
    "Turi",
    "Utsi", -- Common among the Lule Sami
    "Vinka",
}


local Sami = CultureName.New({
    male = male,
    female = female,
    surname = surname,
    name = "Sami",
    code = "se", -- ISO 639-1 code for Northern Sami
    replace = {
        ['Á'] = 'A', ['á'] = 'a',
        ['Č'] = 'C', ['č'] = 'c',
        ['Đ'] = 'D', ['đ'] = 'd',
        ['Ŋ'] = 'N', ['ŋ'] = 'n',
        ['Š'] = 'S', ['š'] = 's',
        ['Ŧ'] = 'T', ['ŧ'] = 't',
        ['Ž'] = 'Z', ['ž'] = 'z',
    }
})

return Sami
