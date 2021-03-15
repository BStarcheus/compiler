#!/bin/sh
success=0
total=0

for f in test/correct/*.src
do
    if ./compiler $f; then
        echo Success: $f
        ((success+=1))
    else
        echo Failure: $f
    fi
    ((total+=1))
done

echo Total Success: $success / $total