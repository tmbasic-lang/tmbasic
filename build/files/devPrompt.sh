#!/bin/bash

function make_colorized() {
    clear
    make | {
        counter=1

        while IFS= read -r line; do
            echo -e "\033[47;30m[${counter}]\033[0m"
            echo $line
            ((counter++))
        done
    }
}

while true
do
    echo
    echo -e "\e[1;30mcompile:\e[0m \e[1;33mm\e[0make \e[1;33mg\e[0mhpages \e[1;33mc\e[0mlean"
    echo -e "\e[1;30mrun:\e[0m     \e[1;33mr\e[0mun \e[1;33m1\e[0m6color \e[1;33m2\e[0m56color \e[1;33mt\e[0mest test\e[1;33mF\e[0milter"
    echo -e "\e[1;30medit:\e[0m    \e[1;33mf\e[0mormat \e[1;33mu\e[0mpdateCompilerTest updat\e[1;33me\e[0mErrors"
    echo -n "───────> "
    read -n1 x
    echo
    if [ "$x" == "m" ]
    then
        make_colorized
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
        export GTEST_FILTER="*"
        if [ -f "test_filter" ]; then
            export GTEST_FILTER=$(cat test_filter)
        fi
        make_colorized
        make test
    elif [ "$x" == "F" ]
    then
        echo -n "GTEST_FILTER="
        read GTEST_FILTER
        echo "$GTEST_FILTER" > test_filter
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
