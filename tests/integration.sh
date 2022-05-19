#!/bin/bash
SCRIPT=$(readlink -f "$0")
SCRIPTPATH=$(dirname "$SCRIPT")
cd $SCRIPTPATH
echo "Script executed from: ${PWD}"
set -e
cd ../build && cmake &>/dev/null .. && make &>/dev/null
cd $SCRIPTPATH

# Test 1: testing good request
echo "Test1: good request..."
echo "Start the server and sending request..."
timeout 0.3s ../build/bin/webserver ./config_files/config_t_port &>/dev/null & 
sleep 0.1
curl -s -I localhost:80 > ./tmp.txt
sleep 0.2
echo "Compare the result"
if cmp -s ../static/static1/expected.txt ./tmp.txt ; then
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
timeout 0.2s ../build/bin/webserver ./config_files/config_t_port &> /dev/null &
sleep 0.1
cat ../static/static1/bad_request.txt | nc -C localhost 80 -q 0 > ./tmp2.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/expected_bad.txt ./tmp2.txt ; then
    rm ./tmp2.txt
    echo -e "Test 2 pass"
else
    rm ./tmp2.txt
    echo -e "Test 2 fail - content not same"
    exit 1
fi

#Test 3: logging
echo "Test3: logging"
../build/bin/webserver &> /dev/null &
sleep 0.1
if sed 's/\[.*\] //g' ../log/SERVER_LOG_0.log | cmp -s ../static/static1/no_arg_log.log ; then
    echo -e "Test 3.1 pass"
else
    echo -e "Test 3.1 fail - logging incorrect"
    exit 1
fi

timeout 0.1s ../build/bin/webserver ./config_files/config_t_port &> /dev/null &
sleep 0.1
lines=`wc -l ../log/SERVER_LOG_0.log | awk '{print $1}'`
if test $lines -ge 7 ; then
    echo -e "Test 3.2 pass"
else
    echo -e "Test 3.2 fail - logging incorrect"
    exit 1
fi

#Test 4: static server
echo "Test4: static server"
timeout 6s ../build/bin/webserver ./config_files/config_t_multipath &> /dev/null &
sleep 0.1
curl localhost:80/static1/minion.jpg --output ./tmp.jpg
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/minion.jpg ./tmp.jpg ; then
    rm ./tmp.jpg
    echo -e "Test 4.1 (image) pass"
else
    rm ./tmp.jpg
    echo -e "Test 4.1 (image) fail - content not same"
    exit 1
fi

sleep 0.1
curl localhost:80/static1/expected.txt > ./tmp.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/expected.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 4.2 (text file) pass"
else
    rm ./tmp.txt
    echo -e "Test 4.2 (text file) fail - content not same"
    exit 1
fi

sleep 0.1
curl localhost:80/static1/minion.jpg.zip --output ./tmp.zip
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/minion.jpg.zip ./tmp.zip ; then
    rm ./tmp.zip
    echo -e "Test 4.3 (zip file) pass"
else
    rm ./tmp.zip
    echo -e "Test 4.3 (zip file) fail - content not same"
    exit 1
fi

sleep 0.1
curl localhost:80/static1/index.html > ./tmp.html
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/index.html ./tmp.html ; then
    rm ./tmp.html
    echo -e "Test 4.4 (html page) pass"
else
    rm ./tmp.html
    echo -e "Test 4.4 (html page) fail - content not same"
    exit 1
fi

sleep 0.1
cat /dev/urandom | head -c 973159 > ../static/static1/random_file_10M
curl localhost:80/static1/random_file_10M --output ./tmp_10M
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/random_file_10M ./tmp_10M ; then
    rm ./tmp_10M
    rm ../static/static1/random_file_10M
    echo -e "Test 4.5 (large file) pass"
else
    rm ./tmp_10M
    rm ../static/static1/random_file_10M
    echo -e "Test 4.5 (large file) fail - content not same"
    exit 1
fi

sleep 0.1
curl localhost:80/static1/static3/test.txt > ./tmp.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static3/test.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 4.6 (longest prefix) pass"
else
    rm ./tmp.txt
    echo -e "Test 4.6 (longest prefix) fail - content not same"
    exit 1
fi

#Test 4.7 trailing slashes
sleep 0.1
curl localhost:80/static1/expected.txt/ > ./tmp.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/expected.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 4.7 (trailing slashes) pass"
else
    rm ./tmp.txt
    echo -e "Test 4.2 (trailing slashes) fail - content not same"
    exit 1
fi

#Test 5: 404 Handler
echo "Test5: 404 Handler"
sleep 0.1
curl localhost:80 > ./tmp_html
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/not_found.html ./tmp_html ; then
    rm ./tmp_html
    echo -e "Test 5 pass"
else
    rm ./tmp_html
    echo -e "Test 5 fail - content not same"
    exit 1
fi

#Test 6: multithread
echo "Test7: Multithrad"
sleep 0.1
curl localhost/sleep/1 > ./tmp0.txt &
curl -s -I localhost > ./tmp.txt
sleep 0.1
echo "Check for mulththread"
lines1=`wc -l ./tmp0.txt | awk '{print $1}'` # should be 0 here
lines2=`wc -l ./tmp.txt | awk '{print $1}'` # multithread, shoule be > 0 here
if test $lines1 -eq 0 && test $lines2 -gt 0 ; then
    sleep 1
    lines3=`wc -l ./tmp0.txt | awk '{print $1}'` # block ends, now > 0
    if test $lines3 -gt 0 ; then
        rm ./tmp.txt ./tmp0.txt
        echo -e "Test 6 pass"
    else 
        rm ./tmp.txt ./tmp0.txt
        echo -e "Test 6 fail - sleep handler not returning on time"
        exit 1
    fi
else
    sleep 1
    rm ./tmp.txt ./tmp0.txt
    echo -e "Test 6 fail - mulththread not supported"
    exit 1
fi


#Test 7: CRUD Handler
#Test 7.1: POST
echo "Test7.1: POST"
#For unknown reasons, CRUD requests only seem to function when the server is started from the build directory
sleep 3
cd ../build
timeout 3s ./bin/webserver ../tests/config_files/config_t_crud &> /dev/null &
cd ../tests
sleep 0.3
curl -v -X POST -H "application/json" -d "{'a': 1}" http://localhost/api/Shoes
sleep 0.1
echo "Compare the result"
if cmp -s ../crud_data/Shoes/1 ../static/static1/crud_post.txt ; then
    echo -e "Test 7.1 pass"
    continue
else
    rm ../crud_data/Shoes/1
    echo -e "Test 7.1 fail - content not same"
    exit 1
fi

#Test 7.2: GET LIST
echo "Test7.2: GET LIST"
sleep 0.1
curl -v http://localhost/api/Shoes | tr -d "\n" > ./tmp.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../static/static1/crud_list.txt ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 7.2 pass"
else
    rm ./tmp.txt
    echo -e "Test 7.2 fail - content not same"
    exit 1
fi

#Test 7.3: GET READ
echo "Test7.3: GET READ"
sleep 0.1
curl -v -X GET http://localhost/api/Shoes/1 | tr -d "\n" > ./tmp.txt
sleep 0.1
echo "Compare the result"
if cmp -s ../crud_data/Shoes/1 ./tmp.txt ; then
    rm ./tmp.txt
    echo -e "Test 7.3 pass"
else
    rm ./tmp.txt
    echo -e "Test 7.3 fail - content not same"
    exitd 1
fi

#Test 7.4: PUT
echo "Test7.4: PUT"
sleep 0.1
curl -v -X PUT -d "It has been PUT!" http://localhost/api/Shoes/1
sleep 0.1
echo "Compare the result"
if cmp -s ../crud_data/Shoes/1 ../static/static1/crud_put.txt ; then
    echo -e "Test 7.4 pass"
else
    rm ../crud_data/Shoes/1
    echo -e "Test 7.4 fail - content not same"
    exit 1
fi

#Test 7.5: DELETE
echo "Test7.5: DELETE"
sleep 0.1
curl -v -X DELETE http://localhost/api/Shoes/1
sleep 0.1
echo "Check for deletion"
if ! test -f ../crud_data/Shoes/1 ; then
    echo -e "Test 7.5 pass"
    exit 0
else
    rm ../crud_data/Shoes/1
    echo -e "Test 7.5 fail - file found"
    exit 1
fi