#!/usr/bin/env bash

set -e
for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "libs" ]; then continue; fi
	pushd $dir
	make
	popd
done
