#!/bin/bash
NAME=$(whoami)	# Insert the process owner's name here.
ipcs -q
ipcs -q | grep $NAME | cut -d " " -f 2 | while read q; do
	echo "removed queue with msqid $q"
	ipcrm -q $q
done
ipcs -q
unset NAME
