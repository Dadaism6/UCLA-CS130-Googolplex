#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
cd $SCRIPTPATH
echo "Script executed from: ${PWD}"
set -e
cd ../build && cmake &>/dev/null .. && make &>/dev/null
cd $SCRIPTPATH
echo "Start the server and sending request..."
../build/bin/webserver ./config_t_port &>/dev/null  & curl -s -I localhost:80 > ./tmp.txt
echo "Shutting down server"
kill %-1
echo "Compare the result"
if cmp --silent ../tests/expected.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test pass"
    exit 0
else
    rm ./tmp.txt
    echo -e "Test fail - content not same"
    exit 1
fi
