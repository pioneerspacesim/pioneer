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
            try:
                sys.stderr.write('Warning: ' + lines.filename() + ' does not end with a newline.')
            except AttributeError:
                sys.stderr.write('Warning: input file does not end with a newline.')

        if ln.endswith('\\\n'):
            accum.append(ln[:-2])
        else:
            accum.append(ln)
            yield ''.join(accum)
            accum = []

#RX_STRING = re.compile()
RX_LINE_COMMENT = re.compile(r'//(.*)')
RX_BLOCK_COMMENT_BEGIN = re.compile(r'/\*')
RX_BLOCK_COMMENT_END = re.compile(r'\*/')
# std: lex.operators (2.12.1)
RX_PUNCTUATION = re.compile(
    r""" # 3-character puncutation
         \.\.\.                     # varargs ellipsis
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
    """, re.VERBOSE)
RX_IDENTIFIER = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')
RX_INCLUDE_NAME = re.compile(r'(<[^>]+>)|("[^"]+")')
RX_STRING = re.compile(r'(L?)"(([^"\\]|\\.)*)"')
RX_CHARACTER = re.compile(r"(L?)'(([^'\\]|\\.)+)'")
# RX_NUMBER corresponds to pp-number (std: lex.ppnumber, 2.9)
RX_NUMBER = re.compile(r'\.?\d([a-zA-Z0-9_\.]|[eE][+-])*')

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

def lex(lines):
    in_block_comment = False
    comment_accum = []
    for ln in lines:
        if in_block_comment:
            end = RX_BLOCK_COMMENT_END.search(ln)
            if end is not None:
                comment_accum.append(ln[:end.start()])
                ln = ln[end.end():]
                in_block_comment = False
                comment = ''.join(comment_accum)
                comment_accum = []
                yield 'block_comment', comment
            else:
                comment_accum.append(ln)
                continue

        assert len(comment_accum) == 0

        ln = ln.lstrip()
        while ln != '':
            tok = RX_LINE_COMMENT.match(ln)
            if tok is not None:
                yield 'line_comment', tok.group(1)
                ln = ''
                continue
            tok = RX_BLOCK_COMMENT_BEGIN.match(ln)
            if tok is not None:
                end = RX_BLOCK_COMMENT_END.search(ln, 2)
                if end is None:
                    comment_accum.append(ln[2:])
                    in_block_comment = True
                    ln = ''
                    continue
                else:
                    yield 'oneline_block_comment', ln[2:end.start()]
                    ln = ln[end.end():].lstrip()
                    continue
            tok = RX_STRING.match(ln)
            if tok is not None:
                yield 'string', tok.group(2)
                ln = ln[tok.end():].lstrip()
                continue
            tok = RX_CHARACTER.match(ln)
            if tok is not None:
                yield 'character', tok.group(2)
                ln = ln[tok.end():].lstrip()
                continue
            # has to come after string & character, because they can have an 'L' prefix
            tok = RX_IDENTIFIER.match(ln)
            if tok is not None:
                toktext = tok.group()
                if toktext in KEYWORDS:
                    yield 'keyword', toktext
                else:
                    yield 'identifier', toktext
                ln = ln[tok.end():].lstrip()
                continue
            tok = RX_NUMBER.match(ln)
            if tok is not None:
                yield 'number', tok.group()
                ln = ln[tok.end():].lstrip()
                continue
            # has to come after block comment, because block comments start with two valid punctuation characters
            # also has to come after pp-number, because pp-number can start with a '.'
            tok = RX_PUNCTUATION.match(ln)
            if tok is not None:
                yield 'punctuation', tok.group()
                ln = ln[tok.end():].lstrip()
                continue
            sys.stderr.write('Unmatched token: ' + repr(ln))
            assert False, "One of the above should have matched..."

def main():
    oparse = OptionParser(usage='%prog [options] inputs')
    (options, args) = oparse.parse_args()

    lines = splice_lines(fileinput.input(args, inplace=False, mode='rU'))
    tokens = lex(lines)

    for ttype, tword in tokens:
        print ttype, repr(tword)

if __name__ == '__main__':
    main()
