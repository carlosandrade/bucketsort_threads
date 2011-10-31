#!/bin/bash

t="1 2 4 8 16"
f="0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 18 19"
a="50 75 100 125 150 175 200 225 250 275 300"

for n in $t
do
	for x in $a
	do
		for y in $f
		do
			java BucketSort $n ${y}_${x}.txt
		done
	done
done

exit 0
