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
	if [ $dir == "./tests" ]; then continue; fi
	if [ $dir == "./storage_master" ]; then continue; fi

	# XXX Temporary
	if [ $dir == "./erpc_crypt" ]; then skips+=($dir-JOSH); continue; fi
	if [ $dir == "./erpc_test" ]; then skips+=($dir-JOSH); continue; fi
	if [ $dir == "./mbedtls_aes" ]; then skips+=($dir-PAT-stacksize); continue; fi
	if [ $dir == "./mbedtls_hash" ]; then skips+=($dir-PAT-stacksize); continue; fi
	if [ $dir == "./mbedtls_hmac" ]; then skips+=($dir-PAT-stacksize); continue; fi

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

for dir in `find storage_master -maxdepth 1 -type d`; do
	if [ $dir == "storage_master" ]; then continue; fi
	pushd $dir > /dev/null
	make || failures+=($dir)
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
