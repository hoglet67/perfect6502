#!/bin/bash

# max number of half-cycles to simulate for
max=3698307

# Input file
infile=z80basic_test.txt

# Output file base
outfile=z80basic_test

echo "Z80 Trap Test"
echo
echo "Removal of a trap should not cause any failures"
echo
echo "Note: each test case takes ~10 minutes"
echo
echo "Generating reference bin/log files:"
./z80basic -t ${outfile}.bin -m ${max} -d < ${infile} | grep -v "## Transistor" > ${outfile}.log
md5sum ${outfile}.bin ${outfile}.log

exit

for i in 2866 3275 3343 3457 3470 3633 3659 3813 3823 4814 4908 5097 5137 5588 5750 5858 5901 6179 6359 6428
do
    echo "Testing for trap at t$i"
    ./z80basic -x ${i} -t ${outfile}_${i}.bin -m ${max} -d < ${infile} | grep -v "## Transistor" > ${outfile}_${i}.log
    md5sum ${outfile}_${i}.bin ${outfile}_${i}.log
done
