#!/bin/bash

RCCBIN=../../release/0.3.1/
RCCTESTFILES=../../test/
${RCCBIN}server &> server_log &
sleep 0.1
${RCCBIN}client -n 3 &> client_log << EOD
${RCCTESTFILES}
test33.h
${RCCTESTFILES}
test32.c
${RCCTESTFILES}
test31.c
EOD

