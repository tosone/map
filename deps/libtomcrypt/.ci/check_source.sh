#!/bin/bash

# output version
bash .ci/printinfo.sh

make clean > /dev/null

echo "checking..."
./helper.pl --check-all || exit 1

exit 0

# ref:         HEAD -> develop
# git commit:  cfbd7f8d364e1438555ff2a247f7e17add11840e
# commit time: 2020-08-29 11:30:23 +0200
