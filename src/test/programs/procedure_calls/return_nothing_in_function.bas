sub Main()
end sub

function Foo() as Number
    return
end function

--output--
Compiler error
kInvalidReturn
Foo
2:5
This function expects to return a value of type Number.
