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

# Don't fail on build failure, for now:
#set -e

# Instead collect and print a list
declare -a failures

declare -a skips


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

	echo "${bold}${blue}Compiling${teal} $(dirname $makefile)${normal}"
	#make -j || echo "${bold} ⤤ $(dirname $makefile)${normal}"
	make -j || failures+=($dir)
	popd > /dev/null
done

if [ ${#skips[@]} -gt 0 ]; then
	echo ""
	echo "${bold}${pink}Skipped:${normal}"
	for skip in ${skips[@]}; do
		echo $skip
	done
fi

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
