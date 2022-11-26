#!/bin/bash


# exit the script if any statement returns a non-true return value
set -o errexit

COMPILER=clang


# Make destination folder:
if [ ! -d ./dist ];
then
	mkdir -p ./dist
fi


# Compile wasm lib:
if [[ $1 == "build" ]];
then
	# Requires clang,
	# no other compiler can compile to wasm:
	echo "Building wasm..."
	cd ./lib
	make
	cd ..
	echo "...built!"

	# If you need to have llvm first in your PATH, run:
	#  echo 'export PATH="/usr/local/opt/llvm/bin:$PATH"' >> ~/.zshrc
	# For compilers to find llvm you may need to set:
	#  export LDFLAGS="-L/usr/local/opt/llvm/lib"
	#  export CPPFLAGS="-I/usr/local/opt/llvm/include"

	# Move compiled binary:
	mv ./lib/module.wasm ./dist/m64Mb_x32.wasm
fi


# Compile concat:
if [ ! -f ./concat ];
then
	echo "Compiling concat..."
	$COMPILER -std=c11 -O3 -Wall -Werror tools/concat.c -o concat
	echo "...compiled!"
fi


# Compile jsmin:
if [ ! -f ./jsmin ];
then
	# https://www.crockford.com/jsmin.html

	echo "Compiling jsmin..."
	$COMPILER -std=c11 -O3 -Wall -Werror tools/jsmin.c -o jsmin
	echo "...compiled!"
fi


# Concal files into the js bundle:
echo "Building JS code..."
./concat dist/rastr.js \
		src/intro.js \
		src/common.js \
		src/palette.js \
		src/layers.js \
		src/vertices.js \
		src/history.js \
		src/outro.js
echo "...built!"


# Minify js bundle:
echo "Minifying..."
./jsmin <dist/rastr.js>dist/rastr.min.js
echo "...minified!"


rm dist/rastr.js


echo "Done."
