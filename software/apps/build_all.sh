#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

black=$(tput setaf 0)
red=$(tput setaf 1)
green=$(tput setaf 2)
blue=$(tput setaf 4)

# Don't fail on build failure, for now:
#set -e

# Instead collect and print a list
declare -a failures

for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "./libs" ]; then continue; fi
	if [ $dir == "./tests" ]; then continue; fi
	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
	make || failures+=($dir)
	popd > /dev/null
done

for dir in `find tests -maxdepth 1 -type d`; do
	if [ $dir == "tests" ]; then continue; fi
	pushd $dir > /dev/null
	make || failures+=($dir)
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
