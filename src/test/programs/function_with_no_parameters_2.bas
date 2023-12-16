#procedure
function Foo() as Number
    return 42
end function

#procedure
sub Main()
    ' Must use parentheses when calling a function.
    print Foo
end sub

--output--
Compiler error
kSymbolNotFound
Main
2:63
