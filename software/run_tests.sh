#!/usr/bin/env bash

bold=$(tput bold)
normal=$(tput sgr0)

black=$(tput setaf 0)
red=$(tput setaf 1)
green=$(tput setaf 2)
blue=$(tput setaf 4)

set -e

echo "${bold}Building all boards...${normal}"
pushd kernel/boards > /dev/null
./build_all.sh
popd > /dev/null

echo "${bold}Building all apps...${normal}"
pushd apps > /dev/null
./clean_all.sh
./build_all.sh
popd > /dev/null

