#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

set -e
for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "./libs" ]; then continue; fi
	if [ $dir == "./support" ]; then continue; fi
	if [ $dir == "./tock_examples" ]; then continue; fi
	if [ $dir == "./tests" ]; then continue; fi
	if [ $dir == "./controller" ]; then continue; fi
	if [ $dir == "./storage_master" ]; then continue; fi
	if [ $dir == "./audio_module" ]; then continue; fi
	if [ $dir == "./ambient_module" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find tests -maxdepth 1 -type d`; do
	if [ $dir == "tests" ]; then continue; fi
	if [ $dir == "tests/mbedtls" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find tests/mbedtls -maxdepth 1 -type d`; do
	if [ $dir == "tests/mbedtls" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find controller -maxdepth 1 -type d`; do
	if [ $dir == "controller" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find storage_master -maxdepth 1 -type d`; do
	if [ $dir == "storage_master" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find audio_module -maxdepth 1 -type d`; do
	if [ $dir == "audio_module" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

for dir in `find ambient_module -maxdepth 1 -type d`; do
	if [ $dir == "ambient_module" ]; then continue; fi
	pushd $dir > /dev/null
	make clean > /dev/null || echo "${bold} ⤤ $dir${normal}"
	popd > /dev/null
done

echo "${bold}All Cleaned.${normal}"
