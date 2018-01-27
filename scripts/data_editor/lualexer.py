import ast
import re
from enum import Enum


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

    def IsEof(self):
        return self.type == TokenType.EOF

    def IsWord(self, val=None):
        return self.type == TokenType.ID and (val is None or self.val == val)

    def IsPunct(self, val):
        return self.type == TokenType.PUNCT and self.val == val

    def IsLiteral(self):
        return self.type in [
            TokenType.INTEGER, TokenType.FLOAT, TokenType.STRING
        ]

    def LiteralVal(self):
        if not self.IsLiteral():
            raise ValueError("Literal expected.")
        return self.val

    def WordVal(self):
        if self.type != TokenType.ID:
            raise ValueError("Identifier expected.")
        return self.val

    def ExpectPunct(self, val):
        if self.type != TokenType.PUNCT or self.val != val:
            raise ValueError("Expected puctuation [%s], got %s:[%s]." %
                             (val, self.type, self.val))


EOF_RE = re.compile(br'^$')
# Whitespace regex also skips comments
WHITESPACE_RE = re.compile(br'(?:[ \n\r\f\t]|(?:--.*))+')
IDENTIFIER_RE = re.compile(br'[a-zA-Z_][0-9a-zA-Z_.]*')
INTEGER_RE = re.compile(br'-?[0-9]+')
FLOAT_RE = re.compile(br'-?[0-9]*\.[0-9]+(?:[eE][-+]?[0-9]+)?')
PUNCT_RE = re.compile(br'[\[\]=:{}*(),\-+*/]')
# Single-quoted string
STRING_SQ_RE = re.compile(br"('[^'\\]*(?:\\.[^'\\]*)*')")
# Double-quoted string
STRING_DQ_RE = re.compile(br'("[^"\\]*(?:\\.[^"\\]*)*")')
# [==[Bracket]==] string
STRING_BB_RE = re.compile(br'\[(=*)\[(.*)?\]\1\]', re.DOTALL)

MATCHERS = [
    (WHITESPACE_RE, lambda m: None),
    (EOF_RE, lambda x: Token(TokenType.EOF)),
    (IDENTIFIER_RE, lambda m: Token(TokenType.ID, m.group())),
    (FLOAT_RE, lambda m: Token(TokenType.FLOAT, float(m.group()))),
    (INTEGER_RE, lambda m: Token(TokenType.INTEGER, int(m.group()))),
    (STRING_SQ_RE,
     lambda m: Token(TokenType.STRING,
                     ast.literal_eval(m.group(1).decode('utf-8')))),
    (STRING_DQ_RE,
     lambda m: Token(TokenType.STRING,
                     ast.literal_eval(m.group(1).decode('utf-8')))),
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
        # When not none, the token which was Peek()ed or Unget() is there.
        self.buf = None

    def Next(self):
        if self.buf:
            tmp = self.buf
            self.buf = None
            return tmp
        for (regex, tokengen) in MATCHERS:
            m = regex.match(self.contents)
            if m:
                token = tokengen(m)
                self.contents = self.contents[m.end():]
                if token:
                    return token
        raise TokenException(self.contents)

    def Peek(self):
        if self.buf:
            return self.buf
        t = self.Next()
        self.Unget(t)
        return t

    def Unget(self, t):
        if self.buf:
            raise ValueError("Not possible to Unget() more than once.")
        self.buf = t


# For debug
if __name__ == "__main__":
    x = TokenStream('../../data/systems/00_sol.lua')
    while True:
        t = x.Next()
        print(t.type, t.val)
        if t.type == TokenType.EOF:
            break