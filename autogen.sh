#!/bin/sh
aclocal && libtoolize --automake && automake -a && autoconf && \
echo "Great. Now type ' ./configure && make '."
