#!/usr/bin/env python
# vim: set ts=8 sts=4 sw=4 expandtab autoindent:

import sys
import os
from optparse import OptionParser
import re
import fnmatch
import glob

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

class CppParseError(Exception):
    def __init__(s, value):
        s.value = value

    def __str__(s):
        return repr(s.value)

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
            raise CppParseError('Unclosed block comment')
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
    raise CppParseError('Unmatched token: ' + repr(ln))

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
                ln = next(lines).lstrip()
            except StopIteration:
                break

        # skip blank lines
        if ln == '':
            continue

        ln, toktype, toktext = match_pp_token(ln, lines)
        # preprocessor include directives are a little special
        # (the source path is a different token type that only matches in this context)
        # this if predicate also copes with null directives (C99 6.10.7)
        if toktype == 'punctuation' and toktext == '#' and ln != '':
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

    def read_attrs(s, attrs):
        if 'skip' in attrs:
            s.skip = attrs['skip']
        if 'name' in attrs:
            s.name = attrs['name']

class EnumData:
    def __init__(s, identifier):
        s.identifier = identifier
        s.name = None
        s.prefix = None
        s.scope = None
        s.items = []

    def __str__(s):
        x = [ 'EnumData:'
            , '  identifier: ' + repr(s.identifier)
            , '  name: ' + repr(s.name)
            , '  prefix: ' + repr(s.prefix)
            , '  items:']
        if s.items:
            for item in s.items:
                x.append('    ' + str(item))
        else:
            x.append('    pass')
        return '\n'.join(x)

    def read_attrs(s, attrs):
        if 'name' in attrs:
            s.name = attrs['name']
        if 'prefix' in attrs:
            s.prefix = attrs['prefix']
        if 'scope' in attrs:
            s.scope = attrs['scope']

    def ident(s):
        if s.name is not None:
            id = s.name
        else:
            if s.identifier != '':
                id = s.identifier
            else:
                raise Exception('Enum with no identifier')
        return id

    def write_c_table(s, fl):
        id = s.ident()
        scope_prefix = '' if s.scope is None else s.scope + '::'
        fl.write('const struct EnumItem ENUM_' + id + '[] = {\n')
        for item in s.items:
            if item.skip:
                continue
            id = item.name if item.name is not None else item.identifier
            fl.write('\t{ "' + id + '", ' + scope_prefix + item.identifier + ' },\n')
        fl.write('\t{ 0, 0 },\n')
        fl.write('};\n')

    def write_c_header(s, fl):
        id = s.ident()
        fl.write('extern const struct EnumItem ENUM_' + id + '[];\n')

RX_ENUM_TAG = re.compile(r'<\s*enum((?:\s+[a-zA-Z_]+(?:=(\w+|\'[^\']*\'|"[^"]*"))?)*)\s*>')
RX_ENUM_ATTR = re.compile(r'([a-zA-Z_]+)(?:=(\w+|\'[^\']*\'|"[^"]*"))?')

def extract_attributes(text):
    attr = {}
    for m in RX_ENUM_ATTR.finditer(text):
        if m.group(2) is not None:
            value = m.group(2)
            if (value[0] == '"' and value[-1] == '"') or (value[0] == "'" and value[-1] == "'"):
                value = value[1:-1]
            attr[m.group(1)] = value
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
        e.read_attrs(extract_attributes(tag.group(1)))

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
                item.read_attrs(extract_attributes(tag.group(1)))

            if item.name is None and e.prefix is not None and item.identifier.startswith(e.prefix):
                item.name = item.identifier[len(e.prefix):]
            e.items.append(item)

        if toktype != 'punctuation' or toktext != '}':
            raise Exception('Bad enum')

        return e
    else:
        # might be a forward declaration or a variable declaration
        return None

def write_generation_header(fl):
    fl.write('/* THIS FILE IS AUTO-GENERATED, CHANGES WILL BE OVERWRITTEN */\n')
    fl.write('/* enum table generated by scan_enums.py */\n\n')

def write_header(enums, fl):
    fl.write('#ifndef HX_GEN_ENUM_TABLES\n')
    fl.write('#define HX_GEN_ENUM_TABLES\n\n')
    write_generation_header(fl)
    fl.write('struct EnumItem { const char *name; int value; };\n\n')
    for e in enums:
        e.write_c_header(fl)
    fl.write('\n')
    fl.write('#endif\n')

def write_tables(enums, headers, hpath, fl):
    write_generation_header(fl)
    if hpath is not None:
       fl.write('#include "' + hpath + '"\n')
    for h in headers:
        fl.write('#include "' + h + '"\n')
    fl.write('\n')
    for e in enums:
        e.write_c_table(fl)
        fl.write('\n')

def extract_enums(lines):
    lines = splice_lines(lines)
    tokens = lex(lines)
    lastcomment = ''
    for toktype, toktext in tokens:
        if toktype == 'comment':
            lastcomment = toktext
        elif toktype == 'keyword' and toktext == 'enum':
            e = parse_enum(toktype, toktext, tokens, lastcomment)
            if e is not None:
                yield e
        else:
            # comments that don't immediately precede the 'enum' keyword
            # are discarded
            lastcomment = ''

def recursive_glob(basedir, pattern):
    for root, dirnames, filenames in os.walk(basedir):
        for name in fnmatch.filter(filenames, pattern):
            yield os.path.join(root, name)

def expand_dirs(args, pattern, recursive):
    for path in args:
        if path != '-' and os.path.isdir(path):
            if not pattern:
                sys.stderr.write("Warning: skipping directory input '" + path + "'\n")
                continue

            if recursive:
                for name in recursive_glob(path, pattern):
                    yield name
            else:
                for name in glob.iglob(os.path.join(path, pattern)):
                    yield name
        else:
            yield path

def main():
    oparse = OptionParser(usage='%prog [options] headers-to-scan')
    oparse.add_option('-o', '--output', type="string", dest="outfile", default='-',
          help="Specify the output file to write to (typically with a .c or .cpp extension). " +
               "Specify '-' to write to stdout (this is the default).")
    oparse.add_option('--header', type="string", dest="headerfile",
          help="Specify the header file to write to. If the main output file is not stdout " +
               "then this defaults to a file of the same name with the extension changed to .h; " +
               "otherwise, then no header content is written.")
    oparse.add_option('--pattern', type='string', dest='pattern',
          help="Specify a file pattern to match for the input files (e.g., *.h). " +
               "This pattern is used to scan any directory inputs.")
    oparse.add_option('-r','--recursive', dest='recursive', action='store_true', default=False,
          help="Scan directory inputs recursively (used with the --pattern argument).")
    (options, args) = oparse.parse_args()

    if options.headerfile is not None and options.outfile is None:
        oparse.error('if you specify --header you must also specify --output')

    # if no input files are specified, default to reading from stdin
    if not args:
        args = ['-']

    # scan input files and record list of headers that have enums
    enums = []
    headers = []
    allinputs = list(expand_dirs(args, options.pattern, options.recursive))
    allinputs.sort()
    for path in allinputs:
        try:
            if path == '-':
                es = list(extract_enums(sys.stdin))
            else:
                with open(path, 'rU') as fl:
                    # skip an optional UTF-8 Byte Order Mark
                    if sys.version_info[0] >= 3:
                        hasbom = (fl.read(1) == '\uFEFF')
                    else:
                        hasbom = (fl.read(3) == '\xef\xbb\xbf')
                    if hasbom:
                        sys.stderr.write("Warning: file '" + path + "' uses a UTF-8 Byte Order Mark\n")
                    else:
                        fl.seek(0)

                    es = list(extract_enums(fl))
                if es:
                    if options.outfile == '-':
                        hpath = os.path.basename(path)
                    else:
                        hpath = os.path.relpath(path, os.path.dirname(options.outfile))
                    headers.append(hpath)
            enums += es
        except CppParseError as e:
            if path == '-':
                prettypath = 'input'
            else:
                prettypath = "'" + path + "'"
            sys.stderr.write("Warning: C++ parse error in " + prettypath + ":\n")
            sys.stderr.write('    ' + e.value + '\n')

    if options.outfile == '-':
        # write to stdout (no header)
        write_tables(enums, headers, None, sys.stdout)
    else:
        # write to the specified file(s)
        assert options.outfile is not None
        cpath = options.outfile
        if options.headerfile is None:
            fname, ext = os.path.splitext(options.outfile)
            hpath = fname + '.h'
        else:
            hpath = options.headerfile

        with open(hpath, 'w') as fl:
            write_header(enums, fl)
        with open(cpath, 'w') as fl:
            write_tables(enums, headers, os.path.basename(hpath), fl)

if __name__ == '__main__' and not sys.flags.interactive:
    main()
