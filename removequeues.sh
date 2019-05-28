#!/bin/bash
ipcs -q
ipcs -q | grep victor | cut -d " " -f 2 | while read q; do 
	echo "removed queue with msqid $q"
	ipcrm -q $q
done
ipcs -q
