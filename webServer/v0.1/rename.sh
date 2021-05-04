#!/bin/bash

find ./ -name *.c  | while read i
do
        echo "$i";
        mv $i.h  $i.hpp
done
