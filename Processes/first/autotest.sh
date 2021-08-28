#!/bin/bash

test_count=$(ls tests |  grep input$ | wc -l)

in_file="t.input"
exp_file="t.expected"

successful=0
i=1
while [ "$i" -le "$test_count" ]; do
    r=$(./program $(cat tests/$i$in_file))
    e=$(cat tests/$i$exp_file)

    echo "---------------------------------------------------"
    echo "result: $r"
    echo "expected: $e"

    if [ "$r" == "$e" ]; then
        echo -e "\e[32mTEST №$i PASSED\e[0m"
        successful=$(expr $successful + 1)
    else
        echo -e "\e[31mTEST №$i FAILED\e[0m"
    fi

    echo "---------------------------------------------------"
    echo ""
    i=$(expr $i + 1)

done

if [ "$successful" -eq "$test_count" ]; then
    echo -e "\e[32mTESTS PASSED\e[0m: $successful/$test_count"
else
    echo -e "\e[31mTESTS PASSED\e[0m: $successful/$test_count"
fi
echo ""
