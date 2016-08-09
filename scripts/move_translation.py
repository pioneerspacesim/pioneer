#!/usr/bin/env python3
# vim: set ts=8 sts=4 sw=4 expandtab autoindent fileencoding=utf-8:

#
# Instructions for moving translations between resources:
#  1- Pull latest translation data from transifex with the command line client.
#  2- Run scripts/canonicalise_translations.py to bring all translations into
#     canonical formatting.
#  3- Commit any changes to the translation files at this point to give a clean
#     state.
#  3- Run scripts/move_translations.py --from=core --to=ui-core KEY_1 KEY_2
#     where 'core' is the language resource to move keys from, and 'ui-core'
#     is the language resource to move keys to, and KEY_1 KEY_2 KEY_3 (as many
#     as you want to move) are the translation keys that you want to move.
#  4- Check the diff to verify that the move has succeeded.
#  5- Commit the changed translation files.
#  6- Use the transifex client to push the updated translations upstream.
#

from optparse import OptionParser
import json
import sys

# 'import *' is bad style, but with a script this short who cares?
from canonicalise_translations import *

def transfer_keys(keys, fromdir, todir):
    fromfiles = set(os.listdir(fromdir))
    tofiles = set(os.listdir(todir))
    matches = (fromfiles & tofiles)
    for lang in matches:
        frompath = os.path.join(fromdir, lang)
        topath = os.path.join(todir, lang)
        print('transferring keys from {} to {}'.format(frompath, topath))
        fromdata = read_translation_file(frompath)
        todata = read_translation_file(topath)
        for k in keys:
            if k in fromdata:
                if k in todata:
                    print('Cannot transfer key %s; destination already has it.')
                else:
                    todata[k] = fromdata[k]
                    del fromdata[k]
        write_translation_file(frompath, fromdata)
        write_translation_file(topath, todata)

def main():
    oparse = OptionParser(usage='%prog [options] list-of-keys-to-move')
    oparse.add_option('--from', type='string', dest='from_res', default='core',
          help="Specify the language resource to transfer keys from.")
    oparse.add_option('--to', type='string', dest='to_res',
          help="Specify the language resource to transfer keys to.")

    (options, keys) = oparse.parse_args()

    if not options.from_res or not options.to_res:
        oparse.error("you must specify both --from and --to")
    if options.from_res == options.to_res:
        oparse.error("--from and --to must be different language resources")
    if not keys:
        oparse.error("you must specify at least one key to transfer")

    print('transferring keys: {}'.format(repr(keys)))

    fromdir = os.path.join('data/lang', options.from_res)
    todir = os.path.join('data/lang', options.to_res)
    transfer_keys(keys, fromdir, todir)

if __name__ == '__main__' and not sys.flags.interactive:
    main()
