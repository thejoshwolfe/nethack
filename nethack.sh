#!/bin/sh

mkdir -p run/dumps
# see files.c lock_file()
touch run/perm
./src/nethack
