#!/bin/bash

CLANG_FORMAT=`which "clang-format"`
if [ ! $PATCH_MODE ]; then
    echo "clang-format = $CLANG_FORMAT"
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
    GIT_DIFF_TOOL="git diff-index --cached"
fi

FILES="$@"
# Allow manually specifiying the files.
if [ -z "$FILES" ]; then
    FILES=$($GIT_DIFF_TOOL --no-commit-id --name-only --diff-filter=d -r $RANGE | grep -v contrib/ | grep -E "\.(c|h|cpp|hpp|cc|hh|cxx|m|mm|inc)$")
fi

if [ ! $PATCH_MODE ]; then
    echo -e "Checking files:"
    for file in $FILES; do
        echo -e "\t$file"
    done
fi

prefix="static-check-clang-format"
suffix="$(date +%s)"
patch="/tmp/$prefix-$suffix.patch"

if [ -z "$TRAVIS" ] && [ ! $PATCH_MODE ]; then
	DIFF_COLOR="--color=always"
else
	DIFF_COLOR=""
fi

for file in $FILES; do
    CLANG_MESSAGE=$("$CLANG_FORMAT" -style=file "$file")

    if [ "$?" = "0" ]; then
        diff $DIFF_COLOR -u "$file" - <<< "$CLANG_MESSAGE" | \
        sed -e "1s|--- |--- a/|" -e "2s|+++ -|+++ b/$file|" >> "$patch"
    fi
done

# if no patch has been generated all is ok, clean up the file stub and exit
if [ ! -s "$patch" ] ; then
    printf "Files in this commit comply with the clang-format rules.\n"
    rm -f "$patch"
    exit 0
fi

if [ $PATCH_MODE ]; then
	# Print the filename of the generated patch.
	cat "$patch"
    rm -f "$patch"
	exit 1
else
	# a patch has been created, notify the user and exit
	printf "\n*** The following differences were found between the code to commit "
	printf "and the clang-format rules:\n-----\n"
	cat "$patch"
	rm -f "$patch"
	printf "\n*** Aborting, please fix your commit(s) with 'git commit --amend' or 'git rebase -i <hash>'\n"
	exit 1
fi
