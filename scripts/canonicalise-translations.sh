#!/bin/sh

# XXX this assumes you have push access to github master and transifex, and
# assume paths. you'll want to hack it before trying to use it yourself

set -x
set -e
find data/lang -name \*.json -not -name en.json | while read f ; do cat $f | perl -MJSON -e 'undef $/;$j=JSON->new->pretty->utf8->indent->canonical;print $j->encode($j->decode(<>))' | sponge $f ; done
exit 0
