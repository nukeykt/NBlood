#!/bin/sh

set -o errexit

if [ -z "$1" -o -z "$2" ]; then
	echo "Usage: $0 <in.glsl> <out.glsl.cpp>"
	exit 1
fi

INFILE="$1"
OUTFILE="$2"

OUTDIR=$(dirname -- "$OUTFILE")
if [ ! -e "$OUTDIR" ]; then
	mkdir -p "$OUTDIR"
fi

VARNAME=$(basename -- "$INFILE")
VARNAME=${VARNAME%.*}

echo "extern char const *$VARNAME;" > "$OUTFILE"
echo "char const *$VARNAME = R\"shader(" >> "$OUTFILE"
cat "$INFILE" >> "$OUTFILE"
echo ')shader";' >> "$OUTFILE"
