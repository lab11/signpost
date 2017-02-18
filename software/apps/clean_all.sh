#!/usr/bin/env bash

set -e
for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "./libs" ]; then continue; fi
	if [ $dir == "./tests" ]; then continue; fi
	if [ $dir == "./support" ]; then continue; fi
	if [ $dir == "./storage_master" ]; then continue; fi
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

for dir in `find storage_master -maxdepth 1 -type d`; do
	if [ $dir == "storage_master" ]; then continue; fi
	pushd $dir
	make clean
	popd
done

for dir in `find audio_module -maxdepth 1 -type d`; do
	if [ $dir == "audio_module" ]; then continue; fi
	pushd $dir
	make clean
	popd
done

