#!/usr/bin/env bash

mkdir -p data/systems/factions

FILES=data/factions/*.lua
if [ -n "$1" ]; then FILES="$1"; fi

for i in $FILES; do
	MATCH=$(rg ':homeworld\((-?\d+),\s*(-?\d+),\s*(-?\d+),\s*(\d+)' -or '$1 $2 $3 $4' $i)

	if [ -n "$MATCH" ]; then
		xargs ./build/editor --dump --output="data/systems/factions/$(basename $i .lua)_homeworld.json" <<< "$MATCH"
	fi
done
