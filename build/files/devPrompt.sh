#!/bin/bash
while true
do
    echo -ne "[$IMAGE_NAME] (m)ake, (r)un, (f)ormat, (t)est, (q)uit? "
    read -n1 x
    echo
    [ "$x" == "m" ] && clear && make
    [ "$x" == "r" ] && make run
    [ "$x" == "f" ] && make format
    [ "$x" == "t" ] && make test
    [ "$x" == "q" ] && break
done
