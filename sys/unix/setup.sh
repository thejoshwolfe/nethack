#!/bin/sh

# Were we started from the top level?  Cope.
if [ -f sys/unix/Makefile.top ]; then cd sys/unix; fi

echo "Copying Makefiles."

cp Makefile.top ../../Makefile
cp Makefile.dat ../../dat/Makefile
cp Makefile.doc ../../doc/Makefile
cp Makefile.src ../../src/Makefile
cp Makefile.utl ../../util/Makefile
