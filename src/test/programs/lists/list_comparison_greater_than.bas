sub Main()
    dim x = [1, 2, 3]
    dim y = [2, 3, 4]
    if x > y then print "wrong"
end sub

--output--
Compiler error
kTypeMismatch
Main
4:10
The ">" operator does not support the type "List of Number".
