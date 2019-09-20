#!/bin/sh

parent_path=$( cd "$(dirname "${BASH_SOURCE[0]}")" ; pwd -P )

INPUT_files=$parent_path/parser_tests/inputs/*

RED=`tput setaf 1`
GREEN=`tput setaf 2`
reset=`tput sgr0`


for inputFile in $INPUT_files
do
	cd $parent_path
	../src/ifj18 < $inputFile > trash
	echo $? >> myOutputs

done

cd $parent_path

for i in {1..33}
do
	if [ `sed "${i}q;d" $parent_path/myOutputs` -eq `sed "${i}q;d" $parent_path/parser_tests/outputs` ]; then
		echo "TEST no.$i ${GREEN}SUCCESS!${reset}"
	else
		echo "TEST no.$i ${RED}FAILURE!${reset}"
	fi 

done


rm -f trash
rm -f myOutputs
