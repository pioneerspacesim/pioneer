from enum import Enum
import re


class TokenType(Enum):
    EOF = 1  # End of file
    ID = 2  # Mix of letters and dots, starting from a letter
    INTEGER = 3
    FLOAT = 4
    STRING = 5  # 'string' or "string" or [[string]] or [===[string]===]
    PUNCT = 6  # Puctuation


class Token:
    def __init__(self, type, val=None):
        self.type = type
        self.val = val


EOF_RE = re.compile(br'^$')
# Whitespace regex also skips comments
WHITESPACE_RE = re.compile(br'(?:[ \n\r\f\t]|(?:--.*))+')
IDENTIFIER_RE = re.compile(br'[a-zA-Z_][a-zA-Z_.]*')
INTEGER_RE = re.compile(br'-?[0-9]+')
FLOAT_RE = re.compile(br'-?[0-9]*\.?[0-9]+(?:[eE][-+]?[0-9]+)?')
PUNCT_RE = re.compile(br'[\[\]=:{}*(),\-+*/]')
# Single-quoted string
STRING_SQ_RE = re.compile(br"'([^'\\]*(?:\\.[^'\\]*)*)'")
# Double-quoted string
STRING_DQ_RE = re.compile(br'"([^"\\]*(?:\\.[^"\\]*)*)"')
# [==[Bracket]==] string
STRING_BB_RE = re.compile(br'\[(=*)\[(.*)?\]\1\]')

MATCHERS = [
    (WHITESPACE_RE, lambda m: None),
    (EOF_RE, lambda x: Token(TokenType.EOF)),
    (IDENTIFIER_RE, lambda m: Token(TokenType.ID, m.group())),
    (FLOAT_RE, lambda m: Token(TokenType.FLOAT, float(m.group()))),
    (INTEGER_RE, lambda m: Token(TokenType.INTEGER, int(m.group()))),
    (STRING_SQ_RE,
     lambda m: Token(TokenType.STRING, m.group(1).decode('unicode_escape'))),
    (STRING_DQ_RE,
     lambda m: Token(TokenType.STRING, m.group(1).decode('unicode_escape'))),
    (STRING_BB_RE,
     lambda m: Token(TokenType.STRING, m.group(2).decode('utf-8'))),
    (PUNCT_RE, lambda m: Token(TokenType.PUNCT, m.group())),
]


class TokenException(Exception):
    def __init__(self, contents):
        self.contents = contents.tobytes().decode(encoding='utf-8')[:20]

    def __str__(self):
        return "Cannot parse: [%s...]" % self.contents


class TokenStream:
    def __init__(self, filename):
        with open(filename, 'rb') as f:
            self.contents = memoryview(f.read())

    def Next(self):
        for (regex, tokengen) in MATCHERS:
            m = regex.match(self.contents)
            if m:
                token = tokengen(m)
                self.contents = self.contents[m.end():]
                if token:
                    return token
        raise TokenException(self.contents)


# For debug
if __name__ == "__main__":
    x = TokenStream('../../data/systems/00_sol.lua')
    while True:
        t = x.Next()
        print(t.type, t.val)
        if t.type == TokenType.EOF:
            break