#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
cd $SCRIPTPATH
echo "Script executed from: ${PWD}"
set -e
# cd ../build && cmake &>/dev/null .. && make &>/dev/null
cd $SCRIPTPATH

# Test 1: testing good request
echo "Test1: good request..."
echo "Start the server and sending request..."
timeout 1s ../build/bin/webserver ./config_t_port &>/dev/null & 
sleep 0.5
curl -s -I localhost:80 > ./tmp.txt
sleep 0.5
echo "Compare the result"
if cmp -s ../tests/expected.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 1 pass"
else
    rm ./tmp.txt
    echo -e "Test 1 fail - content not same"
    exit 1
fi

# Test 2: testing bad request
echo "Test2: bad request..."
echo "Start the server and sending request..."
timeout 1s ../build/bin/webserver ./config_t_port &> /dev/null &
sleep 0.5
cat ./bad_request.txt | nc -C localhost 80 -q 0 > ./tmp2.txt
sleep 0.5
echo "Compare the result"
if cmp -s ../tests/expected_bad.txt ./tmp2.txt ; then
    rm ./tmp2.txt
    echo -e "Test 2 pass"
else
    # rm ./tmp2.txt
    echo -e "Test 2 fail - content not same"
    exit 1
fi

#Test 3: logging
echo "Test3: logging"
../build/bin/webserver &> /dev/null &
sleep 0.1
if sed 's/\[.*\] //g' ../log/SERVER_LOG_0.log | cmp -s ../tests/no_arg_log.log ; then
    echo -e "Test 3.1 pass"
else
    echo -e "Test 3.1 fail - logging incorrect"
    exit 1
fi

timeout 0.5s ../build/bin/webserver ./config_t_port &> /dev/null &
sleep 0.5
if sed 's/\[.*\] //g' ../log/SERVER_LOG_0.log | cmp -s ../tests/normal_log.log ; then
    echo -e "Test 3.2 pass"
    exit 0
else
    echo -e "Test 3.2 fail - logging incorrect"
    exit 1
fi