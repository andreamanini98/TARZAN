#!/bin/bash

mkdir -p build

cd build

cmake ..

cmake --build .

cmake --install .

# Remember to run this shell script with sudo permissions.

# Library will be located at:
# /usr/local/include/TARZAN        - This contains the headers.
# /usr/local/lib/cmake/TARZAN      - This contains cmake files.
# /usr/local/lib/libTARZAN.a       - This is the library archive.

# You can install the library in a custom directory by specifying the installation path as follows:
# cmake --install . --prefix=/path/to/custom/install