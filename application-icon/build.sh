#!/bin/sh

FLAGS='--export-area-page --export-background="#000000" --export-background-opacity=0.0 --without-gui'

png_name() {
   for sz in $*; do
      printf 'pngs/pioneer-%dx%d.png\n' "$sz" "$sz"
   done
}

build_png() {
   SIZE=$1
   SVG=$2
   OUTFILE="$(png_name "$SIZE")"
   test "$SVG" -nt "$OUTFILE" || return 0
   printf 'Generating %sx%s PNG from %s\n' $SIZE $SIZE "$SVG"
   inkscape --export-png="$OUTFILE" -w$SIZE -h$SIZE $FLAGS "$SVG"
   optipng -clobber "$OUTFILE"
}

test -d pngs || mkdir pngs
build_png 256 badge-enlarged-text.svg
build_png 128 badge-enlarged-text.svg
build_png 64 badge-notext-extrastars.svg
build_png 48 badge-notext-extrastars.svg
build_png 40 badge-notext-extrastars.svg
build_png 32 badge-notext-extrastars.svg
build_png 24 badge-square.svg
build_png 22 badge-square.svg
build_png 16 badge-square.svg

sizes="16 24 32 48 64 256"
names="$(png_name $sizes)"
CMD="icotool --create --icon --output=pioneer.ico $names"
printf 'running %s\n' "$CMD"
$CMD
