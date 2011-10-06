--
-- Class: Translate
--
Translate = {
    language = Lang.GetCurrentLanguage(), -- Default
    dictionary = {},

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

}

-- Copy, don't use, the system dictionary, which is read-only
Translate.dictionary[Translate.language] = {}
for token, definition in pairs(Lang.GetDictionary()) do
    Translate.dictionary[Translate.language][token] = definition
end
