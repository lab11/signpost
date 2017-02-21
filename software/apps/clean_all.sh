#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

set -e

for makefile in $(find . | grep '/Makefile$'); do
	pushd $(dirname $makefile) > /dev/null

	# Is submodule?
	pushd $(git rev-parse --show-toplevel)/.. > /dev/null
	is_sub=$(git rev-parse --is-inside-work-tree 2>/dev/null | grep -q true && echo 1 || echo 0)
	popd > /dev/null
	if [ $is_sub == "1" ]; then
		#echo "Skipping submodule $(dirname $makefile)"
		popd > /dev/null
		continue
	fi

	make clean > /dev/null || echo "${bold} ⤤ $(dirname $makefile)${normal}"
	popd > /dev/null
done


echo "${bold}All Cleaned.${normal}"
