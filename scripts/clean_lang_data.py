#!/usr/bin/env python3
# vim: set ts=8 sts=4 sw=4 expandtab autoindent fileencoding=utf-8:

import sys
import os
from optparse import OptionParser
import re
import fnmatch
import glob

RX_IDENT = re.compile(r'[a-zA-Z_][a-zA-Z0-9_]*')
RX_DECLARATION = re.compile(r'DECLARE_STRING\(([a-zA-Z_][a-zA-Z0-9_]*)\)')

def parse_header(fl):
    tokens = []
    for line in fl:
        line = line.strip()
        m = RX_DECLARATION.match(line)
        if m is not None:
            tokens.append(m.group(1))
    return tokens

def strip_unknown_entries(fl, tokens, lines):
    head = True
    key = ''
    value = ''
    for line in lines:
        line = line.strip()
        if line == '' or line.startswith('#'):
            fl.write(line)
            fl.write('\n')
            continue
        if head:
            if RX_IDENT.match(line):
                head = False
                key = line
            else:
                raise Exception('bad syntax')
        else:
            if line.startswith('"') and line.endswith('"'):
                line = line[1:-1]
            value = line.replace('\\n', '\n')
            if key in tokens:
                fl.write(key)
                fl.write('\n    ')
                add_quotes = (value != value.strip())
                if add_quotes:
                    fl.write('"')
                fl.write(value.replace('\n', '\\n'))
                if add_quotes:
                    fl.write('"')
                fl.write('\n')
            else:
                sys.stderr.write("# skipping unknown token '" + key + "'\n")
            key = ''
            head = True

def main():
    oparse = OptionParser(usage='%prog [options] header data-files')
    (options, args) = oparse.parse_args()

    # if no input files are specified, default to reading from stdin
    if len(args) == 0:
        oparse.error('You must specify the language header')

    headerpath = args[0]
    inpaths = args[1:]

    with open(headerpath, 'rU') as fl:
        tokens = parse_header(fl)
        tokens = set(tokens)

    for path in inpaths:
        try:
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

                lines = fl.readlines()

            with open(path, 'w', encoding='utf-8') as fl:
                strip_unknown_entries(fl, tokens, lines)
        except UnicodeDecodeError as e:
            if path == '-':
                prettypath = 'input'
            else:
                prettypath = "'" + path + "'"
            sys.stderr.write("Warning: UTF-8 decode error in " + prettypath + ":\n")
            sys.stderr.write('    ' + str(e) + '\n')

if __name__ == '__main__' and not sys.flags.interactive:
    main()
