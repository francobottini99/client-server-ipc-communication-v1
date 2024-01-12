#!/bin/bash

for process in $(ps aux | grep /bin/Clients | awk '{print $2}')
do
    if kill -0 "$process" > /dev/null 2>&1; then
        kill $process
    fi
done

echo ""
echo ""