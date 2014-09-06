#!/bin/bash

export ASAN_OPTIONS="abort_on_error=1:alloc_dealloc_mismatch=0"
PIONEER="sanitizer-release/src/pioneer"
THREADS=3

tmpdir=$(mktemp -d --tmpdir galgen_verify.XXXXXX)
function cleanup {
    rm -f "$tmpdir"/?.out
    rm -f "$tmpdir"/verify.out
    rmdir "$tmpdir"
}
trap cleanup EXIT

centers=()

function run_it {
    local i
    local threads=0
    for ((i=0;i<${#centers[@]};++i)) ; do
	local center="${centers[$i]}"
	echo "$center $("$PIONEER" -gd - "$radius" $center | md5sum)" > $tmpdir/$threads.out &
	threads=$((threads+1))
	if [ $threads -ge $THREADS ] ; then
	    wait
	    cat $tmpdir/?.out
	    rm $tmpdir/?.out
	    threads=0
	fi
    done
    if [ $threads -gt 0 ] ; then
	wait
	cat $tmpdir/?.out
	rm $tmpdir/?.out
    fi
    centers=()
}

function generate {
    local file="$1"
    local radius="$2"
    local shells="$3"
    local step=$((radius*2 + 1))
    local x
    local y
    local z
    local shell
    local layer
    {
	echo "radius $radius shells $shells"
	echo 0,0,0 "$("$PIONEER" -gd - "$radius" 0,0,0 | md5sum)"
	centers=()
	for ((shell=1;shell<=shells;++shell)) ; do
	    echo "shell $shell bottom -$shell"
	    z=$((-shell*step))
	    for ((y=-shell*step; y<=shell*step; y+=step)) ; do
		for ((x=-shell*step; x<=shell*step; x+=step)) ; do
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		done
	    done
	    run_it
	    for ((layer=-shell+1; layer<=shell-1; ++layer)) ; do
		z=$((layer*step))
		echo "shell $shell middle $layer"
		y=$((-shell*step))
		for ((x=-shell*step; x<=shell*step; x+=step)) ; do
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		done
		for ((y=(-shell+1)*step; y<=(shell-1)*step; y+=step)) ; do
		    x=$((-shell*step))
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		    x=$((shell*step))
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		done
		y=$((shell*step))
		for ((x=-shell*step; x<=shell*step; x+=step)) ; do
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		done
		run_it
	    done
	    echo "shell $shell top $shell"
	    z=$((shell*step))
	    for ((y=-shell*step; y<=shell*step; y+=step)) ; do
		for ((x=-shell*step; x<=shell*step; x+=step)) ; do
		    centers+=("$x,$y,$z")
		    #echo "$x,$y,$z $("$PIONEER" -gd - "$radius" $x,$y,$z | md5sum)"
		done
	    done
	    run_it
	done
    } > "$file"
}

op="${1:?'generate or verify expected'}"
file="${2:?'no file given'}"

if [ "$op" = "generate" ] ; then
    generate "$file" "${3:?'no radius given'}" "${4:?'no number of shells given'}"
elif [ "$op" = "verify" ] ; then
    read dummy1 radius dummy2 shells < "$file"
    if [ "$dummy1" != "radius" -o "$dummy2" != "shells" ] ; then
	echo "Parse error"
	exit 1
    fi
    generate "$tmpdir/verify.out" "$radius" "$shells"
    sdiff -s -d "$file" "$tmpdir/verify.out"
    rm "$tmpdir/verify.out"
else
    echo "generate or verify expected" >&2
    exit 1
fi
