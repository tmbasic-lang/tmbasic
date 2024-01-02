sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    if x = 0 then
        throw 111, "foo"
    else
        throw 222, "bar"
    end if
end function

--output--
Error
222
bar
