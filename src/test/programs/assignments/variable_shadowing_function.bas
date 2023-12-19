#procedure
function Foo() as String
    return "hello"
end function

#procedure
sub Main()
    dim foo = Foo()
    print foo
end sub

--output--
hello
