#!/bin/bash

if [ $# -ne 1 ];
then
        echo 'Wrong number of arguments' >&2
        exit 1
fi

if [ ! -d $1 ];
then
        echo 'Directory does not exist' >&2
        exit 1
fi

ls -F $1 | grep \/$
ls -l | grep ^- | awk '{print $2, $5, $6, $7, $8, $9}'

exit 0