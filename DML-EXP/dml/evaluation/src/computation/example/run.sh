#!/bin/bash
#basic for command

for test in  2 3 4 5
do
for size in 1000 10000 100000 1000000 10000000 100000000
do
#echo The next state is $test $size
./a.out $test $size
done
done