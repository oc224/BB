#!/bin/bash
echo 'show' $2 'from' $1
scp root@$1:~/raw_data/$2.out ./.
./read_res.m $2
