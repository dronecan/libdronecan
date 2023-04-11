#!/usr/bin/env bash
# run astylerc on specified files or directory in the repo

# check if astyle is installed
if ! [ -x "$(command -v astyle)" ]; then
    echo "Error: astyle is not installed." >&2
    exit 1
fi

# ensure that we got argument(s)
if [ $# -eq 0 ]; then
    echo "Usage: $0 <files or directories>"
    exit 1
fi

# create a list of files to process
files=""
for arg in "$@"; do
    if [ -d "$arg" ]; then
        files="$files $(find $arg -name '*.cpp' -o -name '*.h' -o -name '*.c')"
    elif [ -f "$arg" ]; then
        files=$arg
    fi
done
echo "Processing files: $files"
# run astyle with astylerc config file
astyle --options=astylerc $files
