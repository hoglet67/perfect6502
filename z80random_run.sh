#!/bin/bash

# If called with no argumnets, spawn 4 children
if [ $# -eq 0 ]; then
    nohup ./z80random_run.sh 0 > z80random_run.log0 &
    nohup ./z80random_run.sh 1 > z80random_run.log1 &
    nohup ./z80random_run.sh 2 > z80random_run.log2 &
    nohup ./z80random_run.sh 3 > z80random_run.log3 &
    exit
fi

for i in $(seq 1 1000)
do

    n=$(($i%4))

    if [ "$n" == "$1" ]; then

        ./z80random -u $i -t r$n.bin > r$n.out
        ../Z80Decoder/decodez80 -d 0 -a -h -i -s2 -y --cpu nmos_zilog --phi= r$n.bin > r$n.log
        fail=`grep fail r$n.log | wc -l`
        warn=`grep -i warn r$n.log | wc -l`
        echo $i $fail $warn
        if [ "$fail" != "0" ] || [ "$warn" != "2" ]
        then
            exit
        fi
    fi
done
