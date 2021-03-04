#!/bin/bash

version=$(git describe --tags --always --dirty 2>/dev/null)
if [ ! -e ".git" ] || [ -z $version ]
then
   version=$(grep "^VERSION=" makefile_include.mk | sed "s/.*=//")
fi
echo "Testing version:" $version
#grep "VERSION=" makefile | perl -e "@a = split('=', <>); print @a[1];"`

# get uname
echo "uname="`uname -a`

# get gcc name
if [ -z ${CC} ]
then
   CC="gcc"
fi
echo "${CC}="`${CC} -dumpversion`
echo

# ref:         HEAD -> develop
# git commit:  910d6252770f1e517d9ed02dc0549a1d61dfe159
# commit time: 2021-01-19 13:51:22 +0100
