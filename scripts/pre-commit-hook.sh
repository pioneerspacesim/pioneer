#!/bin/sh

# Run clang-format, storing the output in MSG
# Git runs hooks in the base directory, so we don't need to do any fancy dirname stuff.
MSG="$(./scripts/clang-format.sh)"
# $? will be non-zero if clang-format detected formatting issues.
if [ "$?" != 0 ]; then
    # We've had a major malfunction!
    echo "$MSG"
    read -p "Do you want to automatically apply these changes (y/N)?" apply
    case "$apply" in
        y|Y)
            # Get the patch info from the message
            patch="/tmp/$(date +%s).patch"
            echo "$MSG" | git mailinfo --scissors /dev/null "$patch" &>/dev/null
            # And apply it to the staged changes.
            git apply --index "$patch"
            rm "$patch"
            ;;
        *) exit 1;;
    esac
else
    # All good, carry on.
    exit 0
fi
