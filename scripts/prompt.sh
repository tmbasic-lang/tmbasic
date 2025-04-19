#!/bin/bash

# Change to the repository root.
cd "$( dirname "${BASH_SOURCE[0]}" )"
cd ..

function make_tmbasic() {
    clear
    time scripts/make-tmbasic.sh
}

while true
do
    echo
    echo
    echo
    echo
    echo -e "\e[1;33mm\e[0make \e[1;33mg\e[0mhpages \e[1;33mc\e[0mlean : \e[1;33mr\e[0mun \e[1;33m1\e[0m6color \e[1;33m2\e[0m56color \e[1;33mt\e[0mest test\e[1;33mF\e[0milter : \e[1;33mf\e[0mormat \e[1;33mu\e[0mpdate : \e[1;33ms\e[0mhell \e[1;33mq\e[0muit"
    echo -n "? "
    read -n1 x
    echo
    if [ "$x" == "m" ]
    then
        make_tmbasic
    elif [ "$x" == "r" ] 
    then
        TERM=xterm-256color COLORTERM=truecolor cmake/bin/tmbasic || (printf "\r\nCrash detected! Resetting terminal in 5 seconds...\r\n" && sleep 5 && reset && echo "Eating input. Press Ctrl+D." && cat >/dev/null)
    elif [ "$x" == "u" ]
    then
        build/scripts/updateCompilerTest.sh
        build/scripts/updateErrors.sh
        build/scripts/updateSystemCalls.sh
    elif [ "$x" == "1" ]
    then
        TERM=xterm COLORTERM= bin/tmbasic || (printf "\r\nCrash detected! Resetting terminal in 5 seconds...\r\n" && sleep 5 && reset && echo "Eating input. Press Ctrl+D." && cat >/dev/null)
    elif [ "$x" == "2" ]
    then
        TERM=xterm-256color COLORTERM= bin/tmbasic || (printf "\r\nCrash detected! Resetting terminal in 5 seconds...\r\n" && sleep 5 && reset && echo "Eating input. Press Ctrl+D." && cat >/dev/null)
    elif [ "$x" == "f" ]
    then
        scripts/format.sh
    elif [ "$x" == "t" ]
    then
        export GTEST_FILTER="*"
        if [ -f "test_filter" ]; then
            export GTEST_FILTER=$(cat test_filter)
        fi
        make_tmbasic
        scripts/test.sh
    elif [ "$x" == "F" ]
    then
        echo -n "GTEST_FILTER="
        read GTEST_FILTER
        echo "$GTEST_FILTER" > test_filter
    elif [ "$x" == "g" ]
    then
        scripts/make-ghpages.sh
    elif [ "$x" == "c" ]
    then
        scripts/clean.sh
    elif [ "$x" == "s" ]
    then
        bash -i
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
