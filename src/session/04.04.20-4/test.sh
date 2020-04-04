#!/bin/bash

RCCBIN=../../release/0.3.1/
RCCTESTFILES=../../test/
${RCCBIN}server &> server_log &
sleep 0.1
${RCCBIN}client &> client_log << EOD
${RCCTESTFILES}
test11111.c
EOD

