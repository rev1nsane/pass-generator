#!/usr/bin/bash

set -xe

CC=g++
CFLAGS="-Wall -Wextra"

$CC $CFLAGS -o pass_generator pass_generator.cpp -lcurl -lcrypto
