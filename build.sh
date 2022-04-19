#!/bin/bash
DISABLE_RTTI_EXCEPTIONS="-fno-exceptions -fno-rtti"
DISABLED_WARNINGS="-Wno-writable-strings -Wno-missing-braces -Wno-unused-function"

INCLUDE_DIR="-I src/irlibs"


clang++ src/main.cpp -o mayflyc -g -Wall -O0 $INCLUDE_DIR $DISABLE_RTTI_EXCEPTIONS $DISABLED_WARNINGS
