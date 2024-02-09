sub Main()
    dim a = 1
    if true then
        dim a = 1
    end if
    print "ok"
end sub

--output--
Compiler error
kDuplicateSymbolName
Main
4:9
There is already a variable named "a". Try another name for this variable.
