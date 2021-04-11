#!/bin/bash
while true
do
    echo -ne "(m)ake (r)un (x)term (f)ormat (t)est tid(y) (g)hpages (q)uit? "
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
        make ghpages && make ghpages-test
    elif [ "$x" == "q" ]
    then
        break
    else
        echo "Eating stray keystrokes. Press Ctrl+D to return to the prompt."
        cat > /dev/null
    fi
done
