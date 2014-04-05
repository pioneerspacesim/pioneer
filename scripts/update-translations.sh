#!/bin/sh

# XXX this assumes you have push access to github master and transifex, and
# assume paths. you'll want to hack it before trying to use it yourself

set -x
set -e
export PATH=/usr/local/bin:$PATH
cd /home/robn/p/pioneer-translation
git fetch
git reset --hard origin/master
tx push -s
tx pull -a #--minimum-perc=25
find data/lang -name \*.json -not -name en.json | while read f ; do cat $f | perl -MJSON -e 'undef $/;$j=JSON->new->pretty->utf8->indent->canonical;print $j->encode($j->decode(<>))' | sponge $f ; done
git add data/lang/*/*.json
git commit -m 'auto-commit: translation updates'
git push
exit 0
