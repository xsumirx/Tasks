#!/bin/bash

echo $1 >/sys/class/gpio/export
echo $2 >/sys/class/gpio/export

echo in >/sys/class/gpio/gpio$1/direction
echo out >/sys/class/gpio/gpio$2/direction


pinVal=0
while :
do
    value=`cat /sys/class/gpio/gpio$1/value`
    if [[ $value == 1 ]]
    then
        if [[ $pinVal == 0 ]]
        then
            pinVal = 1;
        else
            pinVal = 0;
        fi

        echo $pinVal >/sys/class/gpio/gpio$2/value
    fi

    if [[ $value == 1 ]]
    then
        pinVal=0;
        echo $pinVal >/sys/class/gpio/gpio$2/value
    fi

    sleep 1
done

