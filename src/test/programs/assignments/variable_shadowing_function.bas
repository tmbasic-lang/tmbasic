function Foo() as String
    return "hello"
end function

sub Main()
    dim foo = Foo()
    print foo
end sub

--output--
hello
