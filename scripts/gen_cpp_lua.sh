#!/bin/bash
BASE=`dirname "$0"`
FILES=`ls $BASE/../src/lua/*.cpp`

for FILE in $FILES
do
    lname=`basename $FILE`
	cat $FILE | \
    grep -E -e $'^[ \t]*//' -e $'^[ \t]*/?\*' -e $'^$' | \
    sed -r -e "s/^([ \t]*\/\/|[ \t]*\/?\*\/?)/--/" -e "s/\*\/$//" > gen/$lname.lua
done
