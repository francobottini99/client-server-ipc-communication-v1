#!/bin/bash

count=-1

for process in $(ps aux | grep /bin/Clients | awk '{print $2}')
do
    count=$((count + 1))
done

echo -e "\nCLIENTES ACTIVOS TOTALES $count\n"