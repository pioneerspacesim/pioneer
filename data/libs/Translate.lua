--
-- Class: Translate
--
Translate = {
	language = Lang.GetCurrentLanguage(), -- Default
	dictionary = {},
	flavours = {},

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
-- > Translate:GetFlavours()
--
-- Example:
--
-- > local flavours = Translate:GetFlavours()
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

	GetFlavours = function (self)
		return
			self.flavours[self.language] or
				self.flavours.English or
				error("No flavours defined")
	end,

--
-- Method: AddFlavour
--
-- Adds a flavour to the table of flavours for the specified language
--
-- > Translate:AddFlavour(lang,flavour)
--
-- Parameters:
--
--   lang - language to use
--
--   flavour - single table of flavour texts, for one flavour
--
-- Example:
--
-- > Translate:AddFlavour('English',{
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

	AddFlavour = function (self, lang, flavour)
		if not self.flavours[lang] then
			self.flavours[lang] = {} -- Initialise a table
			local newFlavour = {}
			-- "first" is a flag, that this is the first English flavour.
			-- We're going to compare non-first flavours with that first
			-- one.  The purpose here is ensuring that all flavours have
			-- the same keys, so that we can flag up translation mistakes
			-- as early as possible.
			first = ( (lang == 'English') and #self.flavours.English > 0)
			for key,value in pairs(flavour) do
				if (not first) and (self.flavours.English[1][key] == nil) then
					error(("Key mis-match: {key} is not defined in first flavour"):interp({key = key}))
				end
				newFlavour[key] = value
			end
			-- now check that all keys were specified
			if not first then
				for key,value in pairs(self.flavours.English[1]) do
					if (newFlavour[key] == nil) then
						error(("Key mismatch: {key} is not defined in {lang}, flavour {num}"):interp({key = key,lang = lang,num = #self.flavours[lang] + 1}))
					end
				end
			end
			table.insert(self.flavours[lang],newFlavour)
		end
	end,

}

-- Copy, don't use, the system dictionary, which is read-only
Translate.dictionary[Translate.language] = {}
for token, definition in pairs(Lang.GetDictionary()) do
	Translate.dictionary[Translate.language][token] = definition
end
