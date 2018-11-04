#!/bin/bash

for i in $(seq 100 1000)
do
./z80random -u $i -t r.bin > r.out
../Z80Decoder/decodez80 -d 0 -a -h -i -s2 -y --cpu nmos_zilog --phi= r.bin > r.log
fail=`grep fail r.log | wc -l`
warn=`grep -i warn r.log | wc -l`
echo $i $fail $warn
if [ "$fail" != "0" ] || [ "$warn" != "2" ]
then
    exit
fi
done
