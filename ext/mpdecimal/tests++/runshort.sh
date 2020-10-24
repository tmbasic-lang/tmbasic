#!/bin/sh


# malloc() on OS X does not conform to the C standard.
SYSTEM=`uname -s`
case $SYSTEM in
    darwin*|Darwin*)
        export MallocLogFile=/dev/null
        export MallocDebugReport=crash
        ;;
    *)
        ;;
esac

# Download or copy the official test cases (text files).
./gettests.sh || exit 1


printf "\n# ========================================================================\n"
printf "#                        libmpdec++: static library\n"
printf "# ========================================================================\n\n"

printf "Running official tests ...\n\n"

if  ! ./runtest official.topTest --threaded; then
    printf "\nFAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running additional tests ...\n\n"

if ! ./runtest additional.topTest --threaded; then
    printf "\nFAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running API tests (single thread) ... \n\n"

if ! ./apitest; then
    printf "FAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running API tests (threaded) ... \n\n"

if ! ./apitest --threaded; then
    printf "FAIL\n\n\n"
    exit 1
else
    printf "\n"
fi


printf "\n# ========================================================================\n"
printf "#                        libmpdec++: shared library\n"
printf "# ========================================================================\n\n"

printf "Running official tests ...\n\n"

export LD_LIBRARY_PATH="$PWD/../libmpdec:$PWD/../libmpdec++"
export LD_32_LIBRARY_PATH="$PWD/../libmpdec:$PWD/../libmpdec++"

if  ! ./runtest_shared official.topTest --threaded; then
    printf "\nFAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running additional tests ...\n\n"

if ! ./runtest_shared additional.topTest --threaded; then
    printf "\nFAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running API tests (single thread) ... \n\n"

if ! ./apitest_shared; then
    printf "FAIL\n\n\n"
    exit 1
else
    printf "\n"
fi

printf "Running API tests (threaded) ... \n\n"

if ! ./apitest_shared --threaded; then
    printf "FAIL\n\n\n"
    exit 1
else
    printf "\n"
fi
