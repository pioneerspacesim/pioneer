--
-- This will be replaced with an API function that returns a similar value
-- Issue number #553
--
local defaultLanguage = function ()
    return 'English'
end

--
-- Populate default dictionaries from system languages
-- Warning: Hairy parsing code.  Proceed with caution.
--
local defaultDictionary = function (language)
    local dictionary = {}
    local languagefile = assert(io.open("data/lang/" .. language .. ".txt", 'r'))
    while true do
        local token = languagefile:read('*line')
        if not token then break end
        if not ((token == '') or (token:match("^%s*#"))) then
            token = token:match("^%s*(.-)%s*$")
            local translation = languagefile:read('*line')
            dictionary[token] = translation:match('^%s*"?(.-)"?%s*$')
        end
    end
    return dictionary
end

--
-- Class: Translate
--
Translate = {
    language = defaultLanguage(), -- Default
    dictionary = defaultDictionary(defaultLanguage()),

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
        dictionary = defaultDictionary(self.language)
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
