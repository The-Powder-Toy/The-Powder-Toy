#!/bin/bash

# Documentation generator
# =======================

# the purpose of this script is to generate html documentation from the source code of specified files.

# requirements
# ============

# this script requires pylit to be installed and the rst2html command to be available

# pre generation
# ==============

# make sure the script terminates on errors

set -e

# skip pregeneration if we get any commandline parameters

if [ $# == 0 ]
then

# list of files to generate documentation for. the format is always::

#   "$0 filename language"

# the following languages are available:
# - c
# - c++
# - css
# - python
# - shell
# - slang
# - latex

    $0 SConscript python
    $0 gendocs.sh shell

# exit program after running all the generation steps

    exit
fi

# generation
# ==========

# inform the user of which file we're processing

echo "--- generating docs for $1"

# run pylit to convert source code to restructured text

pylit $1 --language $2 $1.txt

# run rst2html to convert restructured text to html

rst2html.py $1.txt --stylesheet docs/style.css > docs/$1.html

# clean up the restructured text file

rm $1.txt
