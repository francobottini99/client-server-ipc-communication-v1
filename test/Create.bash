#!/bin/bash

fifo=0
shared_memory=0
message_queue=0

if ! [ $# -eq 1 ]; then
    echo "Se requiere un parametro de ingreso y es el numero de clientes a crear."
    exit 1
fi

if ! [[ "$1" =~ ^[0-9]+$ ]]; then
    echo "El parametro de ingreso es el numero de clientes a crear y debe ser un numero."
    exit 1
fi

for ((i=1; i<=$1; i++))
do
    new=$((RANDOM % 3))

    ./bin/Clients $new &

    if [ $new == 0 ]; then
        fifo=$((fifo + 1))
    fi

    if [ $new == 1 ]; then
        shared_memory=$((shared_memory + 1))
    fi

    if [ $new == 2 ]; then
        message_queue=$((message_queue + 1))
    fi

    sleep $(echo "scale=3; $RANDOM/32767*0.1" | bc -l)
    
    echo "FIFO CLIENTS          : $fifo"
    echo "SHARED MEMORY CLIENTS : $shared_memory"
    echo "MESSAGE QUEUE CLIENTS : $message_queue"
    echo "TOTAL CLIENTS         : $((message_queue + shared_memory + fifo))"
    echo -e "\n"
done

exit 0