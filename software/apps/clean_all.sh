#!/usr/bin/env bash

set -e
for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "./libs" ]; then continue; fi
	if [ $dir == "./tests" ]; then continue; fi
	if [ $dir == "./support" ]; then continue; fi
	pushd $dir
	make clean
	popd
done

for dir in `find tests -maxdepth 1 -type d`; do
	if [ $dir == "tests" ]; then continue; fi
	pushd $dir
	make clean
	popd
done
