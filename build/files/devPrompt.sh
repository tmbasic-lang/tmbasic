#!/bin/bash
while true
do
    echo -e "\e[1;30mcompile:\e[0m \e[1;33mm\e[0make \e[1;33mg\e[0mhpages \e[1;33mc\e[0mlean"
    echo -e "\e[1;30mrun:\e[0m     \e[1;33mr\e[0mun \e[1;33m1\e[0m6color \e[1;33m2\e[0m56color \e[1;33mt\e[0mest"
    echo -e "\e[1;30medit:\e[0m    \e[1;33mf\e[0mormat \e[1;33mu\e[0mpdateCompilerTest update\e[1;33mE\e[0mrrors"
    echo -n "───────> "
    read -n1 x
    echo
    if [ "$x" == "m" ]
    then
        clear
        make
    elif [ "$x" == "r" ] 
    then
        TERM=xterm-256color COLORTERM=truecolor make run
    elif [ "$x" == "u" ]
    then
        build/scripts/updateCompilerTest.sh
    elif [ "$x" == "e" ]
    then
        build/scripts/updateErrors.sh
    elif [ "$x" == "1" ]
    then
        TERM=xterm COLORTERM= make run
    elif [ "$x" == "2" ]
    then
        TERM=xterm-256color COLORTERM= make run
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
