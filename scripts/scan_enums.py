#!/usr/bin/env python

import sys
from optparse import OptionParser
import re
import fileinput

# splice lines (std: lex.phase -- Phases of translation: 2.1.1.2)
def splice_lines(lines):
    accum = []
    for ln in lines:
        if not ln.endswith('\n'):
            if ln.endswith('\\'):
                err = ' ends with a backslash'
            else:
                err = ' does not end with a newline'
            try:
                f = lines.filename
            except AttributeError:
                sys.stderr.write('Warning: input file' + err + '\n')
            else:
                sys.stderr.write('Warning: ' + f() + err + '\n')

        if ln.endswith('\\\n'):
            accum.append(ln[:-2])
        else:
            accum.append(ln)
            yield ''.join(accum)
            accum = []

#RX_STRING = re.compile()
RX_LINE_COMMENT = re.compile(r'//(.*)')
RX_BLOCK_COMMENT_BEGIN = re.compile(r'/\*')
RX_BLOCK_COMMENT_END = re.compile(r'\*/\s*') # skip trailing whitespace
# std: lex.operators (2.12.1)
RX_PUNCTUATION = re.compile(
    r""" # 3-character puncutation
       (  \.\.\.                     # varargs ellipsis
       | ->\*                       # dereference member function pointer
       | <<= | >>=                  # bit-shift-assignment
         # 2-character puncutation
       | \#\#                       # preprocessor token concatenation
       | ::                         # scope resolution operator
       | \.\*                       # dereference member function pointer
       | \+= | -= | \*= | /= | %=   # arithmetic-assignment
       | &= | \|= | \^=             # bitwise op-assignment
       | ->                         # dereference member access
       | >> | <<                    # bit-shift and bit-shift-assignment
       | \+\+ | --                  # increment and decrement
       | == | != | <= | >=          # comparison
       | && | \|\|                  # logical and, logical or
         # 1-character puncutation
       | [{}#();:?+*/%^&|~!=<>,\[\]\.\-]
       ) \s*                        # skip trailing whitespace
    """, re.VERBOSE)
RX_IDENTIFIER = re.compile(r'([a-zA-Z_][a-zA-Z0-9_]*)\s*')
RX_PP_HEADERNAME = re.compile(r'((?:<[^>]+>)|(?:"[^"]+"))\s*')
RX_LITERAL = re.compile(
    r"""(?:   (L?)               # wide-char prefix
          (?: "((?: [^"\\] | \\. )*)"  # string literal
          |   '((?: [^'\\] | \\. )+)'  # character literal
        ) \s* )
        | (\.?\d (?: [a-zA-Z0-9_\.] | [eE][+-] )* ) \s*  # pp-number literal
    """, re.VERBOSE)

KEYWORDS = set([
    # alternative representations (std: lex.key, 2.11.2)
    'and','and_eq','bitand','bitor','compl','not','not_eq','or','or_eq','xor','xor_eq',

    # keywords
    'asm', 'auto', 'bool', 'break', 'case', 'catch',
    'char', 'class', 'const', 'const_cast', 'continue',
    'default', 'delete', 'do', 'double', 'dynamic_cast',
    'else', 'enum', 'explicit', 'export', 'extern',
    'false', 'float', 'for', 'friend', 'goto', 'if',
    'inline', 'int', 'long', 'mutable', 'namespace',
    'new', 'operator', 'private', 'protected', 'public',
    'register', 'reinterpret_cast', 'return', 'short',
    'signed', 'sizeof', 'static', 'static_cast', 'struct',
    'switch', 'template', 'this', 'throw', 'true', 'try',
    'typedef', 'typeid', 'typename', 'union', 'unsigned',
    'using', 'virtual', 'void', 'volatile', 'wchar_t', 'while',
])
assert (len(KEYWORDS) == 74)

def match_pp_token(ln, lines):
    tok = RX_LINE_COMMENT.match(ln)
    if tok is not None:
        return '', 'comment', tok.group(1)
    tok = RX_BLOCK_COMMENT_BEGIN.match(ln)
    if tok is not None:
        end = RX_BLOCK_COMMENT_END.search(ln, 2)
        if end is None:
            accum = [ln[2:]]
            for xln in lines:
                end = RX_BLOCK_COMMENT_END.search(xln)
                if end is not None:
                    accum.append(xln[:end.start()])
                    return xln[end.end():], 'comment', ''.join(accum)
                else:
                    accum.append(xln)
            raise Exception('Unclosed block comment')
        else:
            return ln[end.end():], 'comment', ln[2:end.start()]
    tok = RX_LITERAL.match(ln)
    if tok is not None:
        ln = ln[tok.end():]
        if tok.group(2) is not None:
            return ln, 'string', tok.group(2)
        elif tok.group(3) is not None:
            return ln, 'character', tok.group(3)
        elif tok.group(4) is not None:
            return ln, 'number', tok.group(4)
        else:
            assert False, "One of the above should have matched"
    # has to come after string & character, because they can have an 'L' prefix
    tok = RX_IDENTIFIER.match(ln)
    if tok is not None:
        ln = ln[tok.end():]
        if tok.group(1) in KEYWORDS:
            return ln, 'keyword', tok.group(1)
        else:
            return ln, 'identifier', tok.group(1)
    # has to come after block comment, because block comments start with two valid punctuation characters
    # also has to come after pp-number, because pp-number can start with a '.'
    tok = RX_PUNCTUATION.match(ln)
    if tok is not None:
        return ln[tok.end():], 'punctuation', tok.group(1)
    # one of the above should have matched...
    raise Exception('Unmatched token: ' + repr(ln))

def match_pp_headername_or_token(ln, lines):
    tok = RX_PP_HEADERNAME.match(ln)
    if tok is not None:
        return ln[tok.end():], 'headername', tok.group(1)
    return match_pp_token(ln, lines)

def lex(lines):
    ln = ''
    while True:
        # grab the next line if this one is blank
        if ln == '':
            try:
                ln = lines.next().lstrip()
            except StopIteration:
                break

        # skip blank lines
        if ln == '':
            continue

        ln, toktype, toktext = match_pp_token(ln, lines)
        # preprocessor include directives are a little special
        # (the source path is a different token type that only matches in this context)
        if toktype == 'punctuation' and toktext == '#':
            yield toktype, toktext
            ln, toktype, toktext = match_pp_token(ln, lines)
            while toktype == 'comment':
                yield toktype, toktext
                ln, toktype, toktext = match_pp_token(ln, lines)
            if toktype == 'identifier' and toktext == 'include':
                yield toktype, toktext
                ln, toktype, toktext = match_pp_headername_or_token(ln, lines)
                while toktype == 'comment':
                    yield toktype, toktext
                    ln, toktype, toktext = match_pp_headername_or_token(ln, lines)
                # if it didn't match a comment, it should have matched a headername, or some other pp-token
                # either way, it can't match another headername (though it might match more pp-tokens)
        yield toktype, toktext

def main():
    oparse = OptionParser(usage='%prog [options] inputs')
    (options, args) = oparse.parse_args()

    lines = splice_lines(fileinput.input(args, inplace=False, mode='rU'))
    tokens = lex(lines)

    for ttype, tword in tokens:
        print ttype, repr(tword)

if __name__ == '__main__':
    main()
