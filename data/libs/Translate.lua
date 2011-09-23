local Languages = {
    English = {
        FAIL = 'Failure',
        YES = 'Yes',
        NO = 'No',
    },
    Deutsch = {
        YES = 'Ja',
        NO = 'Nein',
    },
    Francais = {
        YES = 'Oui',
        NO = 'Non',
    },
}

--
-- Class: Translate
--
Translate = {
    language = 'English', -- Default
    dictionary = Languages['English'],

--
-- Group: Methods
--

--
-- Method: getLanguage
--
-- Selects or changes the language used by the translator
--
-- Parameters:
--
-- language - optional. A string which represents the language to be used.
--            Defaults to either the last selected language, or 'English'.
--
-- > Translate:getLanguage()
--
-- Returns:
--
-- t - a function which takes a single token in string form, and returns the
--     translated string.  If the token isn't defined, it returns the token.
--
-- Example:
--
-- > local t = Translate:getLanguage('Deutsch')
-- > UI.Message(t('YOU_ARE_LATE'))
--
-- Status:
--
-- experimental
--
    getLanguage = function (self, language)
        self.language = language or self.language
        for token, definition in pairs(Languages[self.language]) do
            self.dictionary[token] = definition
        end
        return function (token)
            return self.dictionary[token] or Languages['English'][token] or token
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
        for token, definition in pairs(dictionary[self.language]) do
            self.dictionary[token] = definition
        end
    end,
}
