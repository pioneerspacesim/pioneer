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
-- Method: GetTranslationFunction
--
-- Selects or changes the language used by the translator
--
-- Parameters:
--
-- language - optional. A string which represents the language to be used.
--            Defaults to either the last selected language, or 'English'.
--
-- > Translate:GetTranslationFunction()
--
-- Returns:
--
-- t - a function which takes a single token in string form, and returns the
--     translated string.  If the token isn't defined, it returns the token.
--
-- Example:
--
-- > local t = Translate:GetTranslationFunction('Deutsch')
-- > UI.Message(t('YOU_ARE_LATE'))
--
-- Status:
--
-- experimental
--
    GetTranslationFunction = function (self, language)
        self.language = language or self.language
        return function (token)
            return self.dictionary[token] or token
        end
    end,

--
-- Method: add
--
-- Adds a dictionary to the selected language only.
--
-- Parameters:
--
-- dictionary - a table of tables of tokens, by language
--
-- Example:
--
-- > Translate:add({
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
    add = function (self, dictionary)
        if (dictionary[self.language]) then
            for token, definition in pairs(dictionary[self.language]) do
                self.dictionary[token] = definition
            end
        end
    end,
}

-- Copy, don't use, the system dictionary, which is read-only
for token, definition in pairs(Lang.GetDictionary()) do
    Translate.dictionary[token] = definition
end
