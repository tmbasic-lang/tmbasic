#procedure
sub Main()
end sub

#procedure
function Foo() as List of Number
    yield 1
    return 1
end function

--output--
Compiler error
kYieldOutsideDimCollection
Foo
2:5
