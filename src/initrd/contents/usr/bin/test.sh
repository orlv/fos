#!/bin/shell
echo ==========
echo TMPFS TEST
echo ==========

echo ls before creating (empty):
ls /tmp
echo creating
test2
echo and ls after:
ls /tmp

