#!/bin/bash
while true
do
    echo -ne "(m)ake (r)un (x)term (f)ormat (t)est tid(y) (g)hpages (c)lean (q)uit? "
    read -n1 x
    echo
    if [ "$x" == "m" ]
    then
        clear
        make
    elif [ "$x" == "r" ] 
    then
        make run
    elif [ "$x" == "y" ]
    then
        make tidy-commit
    elif [ "$x" == "x" ]
    then
        TERM=xterm make run
    elif [ "$x" == "f" ]
    then
        make format
    elif [ "$x" == "t" ]
    then
        make test
    elif [ "$x" == "g" ]
    then
        make ghpages
    elif [ "$x" == "c" ]
    then
        make clean
    elif [ "$x" == "q" ]
    then
        break
    elif [ "$x" == "" ]
    then
        >/dev/null echo
    else
        echo "Eating stray keystrokes. Press Ctrl+D to return to the prompt."
        cat > /dev/null
    fi
done
