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

# Download the official test cases (text files).
./gettests.sh || exit 1


printf "\n# ========================================================================\n"
printf "#                         libmpdec: static library\n"
printf "# ========================================================================\n\n"

printf "Running official tests with allocation failures ...\n\n"

if  ! ./runtest_alloc official.decTest
then
    printf "\nFAIL\n\n\n"
    exit 1
fi

printf "Running additional tests with allocation failures ...\n\n"

if ! ./runtest_alloc additional.decTest
then
    printf "\nFAIL\n\n\n"
    exit 1
fi


printf "\n# ========================================================================\n"
printf "#                         libmpdec: shared library\n"
printf "# ========================================================================\n\n"

printf "Running official tests with allocation failures ...\n\n"

export LD_LIBRARY_PATH="$PWD/../libmpdec"
export LD_32_LIBRARY_PATH="$PWD/../libmpdec"

if  ! ./runtest_alloc_shared official.decTest
then
    printf "\nFAIL\n\n\n"
    exit 1
fi

printf "Running additional tests with allocation failures ...\n\n"

if ! ./runtest_alloc_shared additional.decTest
then
    printf "\nFAIL\n\n\n"
    exit 1
fi



