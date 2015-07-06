#!/bin/sh 

aclocal
libtoolize 
automake --foreign
autoconf
