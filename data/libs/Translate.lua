--
-- Class: Translate
--
Translate = {
	language = Lang.GetCurrentLanguage(), -- Default
	dictionary = {}, -- Initialise the dictionary table
	flavours = {English = {}}, -- Explicitly initialise the English flavours table

--
-- Group: Methods
--

--
-- Method: GetTranslator
--
-- Get a function that can translate strings for the current language
--
-- > Translate:GetTranslator()
--
-- Returns:
--
--   t - a function which takes a single token in string form, and returns the
--       translated string.  If the token isn't defined, it returns the token.
--
-- Example:
--
-- > local t = Translate:GetTranslator()
-- > UI.Message(t('YOU_ARE_LATE'))
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--
	GetTranslator = function (self)
		return function (token)
			return
				(self.dictionary[self.language] and self.dictionary[self.language][token]) or
				(self.dictionary.English and self.dictionary.English[token]) or
				error("Translation token not found: "..token)
		end
	end,

--
-- Method: Add
--
-- Adds a dictionary to the selected language only.
--
-- Parameters:
--
--   dictionary - a table of tables of tokens, by language
--
-- Example:
--
-- > Translate:Add({
-- >     English = {
-- >         WELCOME = 'Welcome',
-- >         THATSBEAUT = "That's beautiful",
-- >     },
-- >     Deutsch = {
-- >         WELCOME = 'Willkommen',
-- >         THATSBEAUT = 'Das ist wunderschön',
-- >     }
-- > })
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--
	Add = function (self, dictionaries)
		for lang, dictionary in pairs(dictionaries) do
			if self.dictionary[lang] == nil then self.dictionary[lang] = {} end
			for token, definition in pairs(dictionary) do
				self.dictionary[lang][token] = definition
			end
		end
	end,

--
-- Method: SetLanguage
--
-- Changes the current language used by the transltor
--
-- > Translate:SetLanguage(lang)
--
-- Parameters:
--
--   lang - language to use
--
-- Example:
--
-- > Translate:SetLanguage('Deutsch')
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--
	SetLanguage = function (self, language)
		self.language = language or self.language
	end,

--
-- Method: GetFlavours
--
-- Returns a table of flavours, in the current language, or English
-- if that language has no defined flavours.
--
-- > Translate:GetFlavours(modname)
--
-- Parameters:
--
--   modname - string; a unique identifier for the module
--
-- Example:
--
-- > local flavours = Translate:GetFlavours('ThisMod')
-- > flavour = flavours[rand:Integer(1,#flavours)]
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--

	GetFlavours = function (self,modname)
		return
			(self.flavours[self.language] and self.flavours[self.language][modname]) or
				self.flavours.English[modname] or
				error(("No flavours defined for {modname}"):interp({modname = modname}))
	end,

--
-- Method: AddFlavour
--
-- Adds a flavour to the table of flavours for the specified language
--
-- > Translate:AddFlavour(lang, modname, flavour)
--
-- Parameters:
--
--   lang - language to use
--
--   modname - string; a unique identifier for the module
--
--   flavour - single table of flavour texts, for one flavour
--
-- Example:
--
-- > Translate:AddFlavour('English','ThisMod',{
-- >      title = "Shill bidder wanted for auction",
-- >      greeting = "Hi there.  Want to earn some quick cash?",
-- >      yesplease = "Sure.  What do you need me to do?",
-- >      nothanks = "No, ta - this looks a bit shady.",
-- > })
--
-- Availability:
--
--   alpha 15
--
-- Status:
--
--   experimental
--

	AddFlavour = function (self, lang, modname, flavour)
		if self.flavours[lang] == nil then
			self.flavours[lang] = {} -- Initialise the flavours table for the specified language
		end
		if self.flavours.English[modname] == nil then
			self.flavours.English[modname] = {} -- Explicitly initialise English flavour table
		end
		if self.flavours[lang][modname] == nil then
			self.flavours[lang][modname] = {} -- Initialise specified language flavour table
		end
		local newFlavour = {}
		----------------------------------------------------------------
		-- "first" is a flag, that this is the first English flavour.
		-- We're going to compare non-first flavours with that first
		-- one.  The purpose here is ensuring that all flavours have
		-- the same keys, so that we can flag up translation mistakes
		-- as early as possible.
		----------------------------------------------------------------
		local first = ((lang == 'English') and (#self.flavours.English[modname] == 0))
		-- First, iterate over the new flavour, and reject those keys
		-- that aren't in the first flavour.
		for key,value in pairs(flavour) do
			if (not first) and (self.flavours.English[modname][1][key] == nil) then
				error(("Key mis-match: {key} is not defined in first English flavour, flavour {num} of {modname} in {lang}"):interp({key = key,lang = lang,num = #self.flavours[lang][modname] + 1,modname = modname}))
			end
			newFlavour[key] = value
		end
		-- Second, check that all keys that were specified in the
		-- first flavour are also in this new one.
		if not first then
			for key,value in pairs(self.flavours.English[modname][1]) do
				if (newFlavour[key] == nil) then
					error(("Key mismatch: {key} is not defined in {lang}, flavour {num} of {modname}"):interp({key = key,lang = lang,num = #self.flavours[lang][modname] + 1,modname = modname}))
				end
			end
		end
		table.insert(self.flavours[lang][modname],newFlavour)
	end,

}

-- Copy, don't use, the system dictionary, which is read-only
Translate.dictionary[Translate.language] = {}
for token, definition in pairs(Lang.GetDictionary()) do
	Translate.dictionary[Translate.language][token] = definition
end
