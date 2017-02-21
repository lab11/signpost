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

for dir in `find . -maxdepth 1 -type d`; do
	if [ $dir == "." ]; then continue; fi
	if [ $dir == "./libs" ]; then continue; fi
	if [ $dir == "./support" ]; then continue; fi
	if [ $dir == "./tock_examples" ]; then continue; fi
	if [ $dir == "./tests" ]; then continue; fi
	if [ $dir == "./storage_master" ]; then continue; fi
	if [ $dir == "./audio_module" ]; then continue; fi
	if [ $dir == "./ambient_module" ]; then continue; fi

	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
	make -j || failures+=($dir)
	popd > /dev/null
done

for dir in `find tests -maxdepth 1 -type d`; do
	if [ $dir == "tests" ]; then continue; fi
	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
	make -j || failures+=($dir)
	popd > /dev/null
done

for dir in `find storage_master -maxdepth 1 -type d`; do
	if [ $dir == "storage_master" ]; then continue; fi
	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
	make -j || failures+=($dir)
	popd > /dev/null
done

for dir in `find audio_module -maxdepth 1 -type d`; do
	if [ $dir == "audio_module" ]; then continue; fi
	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
	make -j || failures+=($dir)
	popd > /dev/null
done

for dir in `find ambient_module -maxdepth 1 -type d`; do
	if [ $dir == "ambient_module" ]; then continue; fi
	echo "${bold}${blue}Compiling${black} $dir${normal}"
	pushd $dir > /dev/null
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
