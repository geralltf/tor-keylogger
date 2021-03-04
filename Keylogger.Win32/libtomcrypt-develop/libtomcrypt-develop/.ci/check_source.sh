#!/bin/bash

# output version
bash .ci/printinfo.sh

make clean > /dev/null

echo "checking..."
./helper.pl --check-all || exit 1

exit 0

# ref:         HEAD -> develop
# git commit:  910d6252770f1e517d9ed02dc0549a1d61dfe159
# commit time: 2021-01-19 13:51:22 +0100
