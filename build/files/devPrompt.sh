#!/bin/bash
while true
do
    echo -ne "[$IMAGE_NAME] (m)ake, (r)un, (v)t100, (f)ormat, (t)est, (g)hpages, (q)uit? "
    read -n1 x
    echo
    [ "$x" == "m" ] && clear && make
    [ "$x" == "r" ] && make run
    [ "$x" == "v" ] && TERM=vt100 make run
    [ "$x" == "f" ] && make format
    [ "$x" == "t" ] && make test
    [ "$x" == "g" ] && make ghpages && make ghpages-test
    [ "$x" == "q" ] && break
done
