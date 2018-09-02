#!/bin/sh

CLANG_FORMAT="clang-format"
if [ "$TRAVIS" = "true" ]; then
    CLANG_FORMAT="$CLANG_FORMAT-6.0"
fi

if [ "$TRAVIS_PULL_REQUEST" != "false" ]; then
    # Check the whole commit range against $TRAVIS_BRANCH, the base merge branch
    # We could use $TRAVIS_COMMIT_RANGE but it doesn't play well with force pushes
    RANGE="$(git rev-parse $TRAVIS_BRANCH) HEAD"
else
    # Test only the last commit
    RANGE=HEAD
fi

if [ -n "$TRAVIS" ]; then
    GIT_DIFF_TOOL="git diff-tree"
else
    GIT_DIFF_TOOL="git diff-index"
fi

# Allow manually specifiying the files.
if [ -z "$FILES" ]; then
    FILES=$($GIT_DIFF_TOOL --no-commit-id --name-only -r $RANGE | grep -v contrib/ | grep -E "\.(c|h|cpp|hpp|cc|hh|cxx|m|mm|inc)$")
fi
echo -e "Checking files:\n$FILES"

prefix="static-check-clang-format"
suffix="$(date +%s)"
patch="/tmp/$prefix-$suffix.patch"

for file in $FILES; do
    "$CLANG_FORMAT" -style=file "$file" | \
    diff -u "$file" - | \
    sed -e "1s|--- |--- a/|" -e "2s|+++ -|+++ b/$file|" >> "$patch"
done

# if no patch has been generated all is ok, clean up the file stub and exit
if [ ! -s "$patch" ] ; then
    printf "Files in this commit comply with the clang-format rules.\n"
    rm -f "$patch"
    exit 0
fi

# a patch has been created, notify the user and exit
printf "\n*** The following differences were found between the code to commit "
printf "and the clang-format rules:\n-----\n"
cat "$patch"
rm -f "$patch"
printf "\n*** Aborting, please fix your commit(s) with 'git commit --amend' or 'git rebase -i <hash>'\n"
exit 1
