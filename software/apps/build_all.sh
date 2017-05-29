#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

black=$(tput setaf 0)
red=$(tput setaf 1)
green=$(tput setaf 2)
yellow=$(tput setaf 3)
blue=$(tput setaf 4)
pink=$(tput setaf 5)
teal=$(tput setaf 6)
white=$(tput setaf 7)
grey=$(tput setaf 8)

set -e
set -u
set -o pipefail

# Instead collect and print a list
declare -a failures

for makefile in $(find . | grep '/Makefile$'); do
	pushd $(dirname $makefile) > /dev/null
	name=$(dirname $makefile)

	# Is submodule?
	pushd $(git rev-parse --show-toplevel)/.. > /dev/null
	is_sub=$(git rev-parse --is-inside-work-tree 2>/dev/null | grep -q true && echo 1 || echo 0)
	popd > /dev/null
	if [ $is_sub == "1" ]; then
		#echo "Skipping submodule $(dirname $makefile)"
		popd > /dev/null
		continue
	fi

    if [ $name == "./tests/erpc_test" -o $name == "./tests/erpc_crypt" -o $name == "./storage_master/fat_test" ]; then
		#echo "Skipping $(dirname $makefile)"
		popd > /dev/null
		continue
	fi

	echo "${bold}${blue}Compiling${teal} $name${normal}"
	make -j || { echo "${bold} ⤤ $name${normal}" ; echo "" ; failures+=("$name");}
	popd > /dev/null
done

if [ ${#failures[@]} -gt 0 ]; then
	echo ""
	echo "${bold}${red}Build Failures:${normal}"
	for fail in ${failures[@]}; do
		echo $fail
	done
	exit 1
fi

echo ""
echo "${bold}${green}All Built.${normal}"
