#!/bin/sh

set -o errexit

if [ -z "$1" ]; then
	echo "Usage: ${0} shader.glsl"
	exit 1
fi

if [ ! -e shaders ]; then
	mkdir shaders
fi

INFILE=$(basename -- "$1")
OUTFILE=shaders/${INFILE}.cpp
VARNAME=${INFILE%.*}

echo char const* ${VARNAME} = R\"shader\( > $OUTFILE
cat "$1" >> $OUTFILE
echo ')shader";' >> $OUTFILE
