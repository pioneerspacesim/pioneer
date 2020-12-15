#!/usr/bin/env python3
# vim: set ts=8 sts=4 sw=4 expandtab autoindent fileencoding=utf-8:

import json
import os
import sys

def read_translation_file(path):
    """Reads a JSON translation file."""
    with open(path, 'r', encoding='utf-8') as fl:
        return json.load(fl)

def write_translation_file(path, data):
    """Writes a JSON translation file with canonical formatting."""
    with open(path, 'w', encoding='utf-8') as fl:
        json.dump(data, fl,
                  ensure_ascii=False,
                  indent=2,
                  separators=(',',': '),
                  sort_keys=True)
        fl.write('\n')

def main():
    for dirpath, _, filenames in os.walk('data/lang'):
        for path in (os.path.join(dirpath, fname)
                     for fname in filenames if fname.endswith('.json')):
            print('Canonicalising', path, '...')
            data = read_translation_file(path)
            write_translation_file(path, data)

if __name__ == '__main__' and not sys.flags.interactive:
    main()
