#!/bin/bash

function make_colorized() {
    ccache --zero-stats
    clear
    # Create a file descriptor 3 for stderr
    exec 3>&1
    time make 2> >(while IFS= read -r line; do
                  echo "$line" >&3  # Pass stderr to the console
                  if [[ "$line" == *"error:"* ]]; then
                      return
                  fi
              done) | {
        counter=1
        while IFS= read -r line; do
            # Colorize and number each line from stdout
            echo
            echo -e "\033[47;30m ${counter} \033[0m $line"
            ((counter++))
        done
    }
    # Close the extra file descriptor
    exec 3>&-
    ccache --show-stats | awk 'BEGIN { RS = "\n\n" } { print $0; exit }'
}

while true
do
    echo
    echo -e "\e[1;30mcompile:\e[0m \e[1;33mm\e[0make \e[1;33mg\e[0mhpages \e[1;33mc\e[0mlean"
    echo -e "\e[1;30mrun:\e[0m     \e[1;33mr\e[0mun \e[1;33m1\e[0m6color \e[1;33m2\e[0m56color \e[1;33mt\e[0mest test\e[1;33mF\e[0milter"
    echo -e "\e[1;30medit:\e[0m    \e[1;33mf\e[0mormat \e[1;33mu\e[0mpdate"
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
        build/scripts/updateErrors.sh
        build/scripts/updateSystemCalls.sh
    elif [ "$x" == "1" ]
    then
        TERM=xterm COLORTERM= make run
    elif [ "$x" == "2" ]
    then
        TERM=xterm-256color COLORTERM= make run
    elif [ "$x" == "f" ]
    then
        ./dev-format.sh
    elif [ "$x" == "t" ]
    then
        export GTEST_FILTER="*"
        if [ -f "test_filter" ]; then
            export GTEST_FILTER=$(cat test_filter)
        fi
        make_colorized
        ./dev-test.sh
    elif [ "$x" == "F" ]
    then
        echo -n "GTEST_FILTER="
        read GTEST_FILTER
        echo "$GTEST_FILTER" > test_filter
    elif [ "$x" == "g" ]
    then
        ./make-ghpages.sh
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
