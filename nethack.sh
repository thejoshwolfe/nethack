#!/bin/sh

mkdir -p run
# see files.c lock_file()
touch run/perm
./src/nethack
