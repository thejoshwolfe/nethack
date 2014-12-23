#!/bin/sh

mkdir -p dumps

# see files.c lock_file()
touch perm

# see topten.c topten()
touch logfile
touch record

./src/nethack
