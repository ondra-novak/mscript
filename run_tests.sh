#!/bin/sh
for I in testdata/*.mscript
do 
    echo TEST $I
     bin/mscript_cli run $I
     echo "------------------"
done 

