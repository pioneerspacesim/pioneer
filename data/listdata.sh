#! /bin/sh -e
cd "`dirname "$0"`"
find * -type f | grep -vf listdata.exclude | sort | sed -e 's/[ '\'']/\\&/g'
