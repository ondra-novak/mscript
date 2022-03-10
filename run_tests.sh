#!/bin/sh
for I in testdata/*.mscript
do 
    echo TEST $I
     bin/test run $I
     echo "------------------"
done 

