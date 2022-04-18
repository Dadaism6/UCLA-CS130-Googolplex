#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
cd $SCRIPTPATH
echo "Testing bad request..."
echo "Script executed from: ${PWD}"
set -e
echo "Start the server and sending request..."
timeout 5s ../build/bin/webserver ./config_t_port &>/dev/null  & 
cat ./bad_request.txt | nc -C localhost 80 -q 0 > ./tmp2.txt
echo "Shutting down server"
echo "Compare the result"
if cmp --silent ../tests/expected_bad.txt ./tmp2.txt ; then
    rm ./tmp2.txt
    echo -e "Test pass"
    exit 0
else
    rm ./tmp2.txt
    echo -e "Test fail - content not same"
    exit 1
fi