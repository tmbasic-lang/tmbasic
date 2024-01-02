sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    dim set foo
        yield 1
        throw 999, "blah"
    end dim
end function

--output--
Error
999
blah
