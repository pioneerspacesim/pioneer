#!/bin/bash

# Pre-requisites:
#
# 0. Install cmd json processor, (note: Debian installed old incompatible version):
#    curl -o- https://raw.githubusercontent.com/transifex/cli/master/install.sh | bash
#    apt install jq
#
# 1. Copy/set up transifex authentication configuration in ~/.transifexrc
#
# 2. github authentication in ~/.ssh/pioneer-transifex.id_rsa
#    2.1 add to ssh/config:
#
#    Host pioneer-transifex-github
#      Hostname github.com
#      User git
#      IdentityFile ~/.ssh/pioneer-transifex.id_rsa

set -x

# Exit immediately if a command exits with a non-zero status, and
# return exit status code of failing command (cron might not report
# error unless captured manually)
set -e

# Cron is too bare bones to know my PATH, evident when manually running cron (i.e. minimal bash env):
#     bash
#     env -i /bin/bash -c 'cd /home/kf && /home/kf/pioneer-update-translation.sh'
# shows we need to add:
export PATH=$PATH:/home/kf/.local/bin/

# Log environment details
echo "Running as user: $(whoami)" > /home/kf/script-debug.log
echo "Environment: $(env)" >> /home/kf/script-debug.log
echo "PATH: $PATH" >> /home/kf/script-debug.log
echo "Current directory: $(pwd)" >> /home/kf/script-debug.log

if [ "$1" = "init" ]
then
    git clone --depth=1 git@pioneer-transifex-github:pioneerspacesim/pioneer
    cd pioneer
    git config user.name 'Pioneer Transifex'
    git config user.email 'pioneer-transifex@pioneerspacesim.net'
else
    echo $(pwd)
    cd pioneer
    git fetch
    git reset --hard origin/master

    # push only our en.json source strings to transifex (-s source, -t translation)
    tx push -s
    echo "Transifex push exit code: $?"

    # (force) pull translations from transifex to local machine
    # -t: pull translation files only (exclude en.json source)
    # -f: force
    # -a: for new modules: also pull down new xx.json files that do not (yet) exist locaylly
    tx pull -t -f -a
    echo "Transifex pull exit code: $?"
    ## NOTE: If $? is 0 after push, the issue is elsewhere (e.g., environment variables, permissions).

    # Remove transifex's indentation:
    for i in $(find data/lang -name '*.json')
    do
        jq -S . < $i > /tmp/${i//\//_}
        mv /tmp/${i//\//_} $i ;
    done

    git add data/lang/
    git commit -m 'auto-commit: translation updates'
    git push
fi

echo "Finished successfully" >> /home/kf/script-debug.log
exit 0
