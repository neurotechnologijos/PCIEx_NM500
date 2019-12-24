#!/bin/bash

_base_dir=${1:-"./"}

echo ${_base_dir}

find ${_base_dir} \
     -type f \( \
       -iname "*.h" \
    -o -iname "*.hpp" \
    -o -iname "*.hxx" \
    -o -iname "*.h++" \
\
    -o -iname "*.c" \
    -o -iname "*.cpp" \
    -o -iname "*.cxx" \
    -o -iname "*.c++" \
    -o -iname "*.cc" \
    \) \
  -execdir clang-format -i '{}' \;
