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
       ( \.\.\.                     # varargs ellipsis
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

def collect_comments(tokens, comments):
    for toktype, toktext in tokens:
        if toktype != 'comment':
            return toktype, toktext
        else:
            comments.append(toktext)

def skip_comments(tokens):
    for toktype, toktext in tokens:
        if toktype != 'comment':
            return toktype, toktext

class EnumItem:
    def __init__(s, identifier):
        s.identifier = identifier
        s.name = None
        s.skip = False
    def __str__(s):
        x = ['EnumItem(', repr(s.identifier)]
        if s.name is not None:
            x += [', name=', repr(s.name)]
        if s.skip:
            x += [', skip=True']
        x.append(')')
        return ''.join(x)

class EnumData:
    def __init__(s, identifier):
        s.identifier = identifier
        s.name = None
        s.prefix = None
        s.items = []

    def __str__(s):
        x = [ 'EnumData:'
            , '  identifier: ' + repr(s.identifier)
            , '  name: ' + repr(s.name)
            , '  prefix: ' + repr(s.prefix)
            , '  items:']
        if len(s.items) > 0:
            for item in s.items:
                x.append('    ' + str(item))
        else:
            x.append('    pass')
        return '\n'.join(x)

RX_ENUM_TAG = re.compile(r'<\s*enum((?:\s+[a-zA-Z_]+(?:=\w+)?)*)\s*>')
RX_ENUM_ATTR = re.compile(r'([a-zA-Z_]+)(?:=(\w+))?')

def extract_attributes(text):
    attr = {}
    for m in RX_ENUM_ATTR.finditer(text):
        if m.group(2) is not None:
            attr[m.group(1)] = m.group(2)
        else:
            attr[m.group(1)] = True
    return attr

def parse_enum(toktype, toktext, tokens, preceding_comment=None):
    assert toktype == 'keyword'
    assert toktext == 'enum'
    tag = []
    if preceding_comment is not None:
        tag.append(preceding_comment)
    toktype, toktext = collect_comments(tokens, tag)

    if toktype == 'identifier':
        identifier = toktext
        toktype, toktext = collect_comments(tokens, tag)
    else:
        identifier = ''

    if toktype == 'punctuation' and toktext == '{':
        # comments become part of the enum tag right up until
        # the identifier for the first elements
        toktype, toktext = collect_comments(tokens, tag)

        tag = RX_ENUM_TAG.search(' '.join(tag))
        if tag is None:
            return None

        e = EnumData(identifier)

        attributes = extract_attributes(tag.group(1))
        if 'name' in attributes:
            e.name = attributes['name']
        if 'prefix' in attributes:
            e.prefix = attributes['prefix']

        while toktype == 'identifier':
            item = EnumItem(toktext)
            tag = []
            toktype, toktext = collect_comments(tokens, tag)
            while toktype != 'punctuation' or toktext not in [',', '}']:
                toktype, toktext = collect_comments(tokens, tag)
            if toktype == 'punctuation' and toktext == ',':
                toktype, toktext = collect_comments(tokens, tag)
            
            tag = RX_ENUM_TAG.search(' '.join(tag))
            if tag is not None:
                attributes = extract_attributes(tag.group(1))
                if 'skip' in attributes:
                    item.skip = attributes['skip']
                if 'name' in attributes:
                    item.name = attributes['name']

            if item.name is None and e.prefix is not None and item.identifier.startswith(e.prefix):
                item.name = item.identifier[len(e.prefix):]
            e.items.append(item)

        if toktype != 'punctuation' or toktext != '}':
            raise Exception('Bad enum')

        return e
    else:
        # might be a forward declaration or a variable declaration
        return None

def main():
    oparse = OptionParser(usage='%prog [options] inputs')
    (options, args) = oparse.parse_args()

    lines = splice_lines(fileinput.input(args, inplace=False, mode='rU'))
    tokens = lex(lines)

    enums = []

    lastcomment = ''
    for toktype, toktext in tokens:
        if toktype == 'comment':
            lastcomment = toktext
        elif toktype == 'keyword' and toktext == 'enum':
            e = parse_enum(toktype, toktext, tokens, lastcomment)
            if e is not None:
                enums.append(e)
        else:
            # comments that don't immediately precede the 'enum' keyword
            # are discarded
            lastcomment = ''

    for e in enums:
        print e

if __name__ == '__main__':
    main()
