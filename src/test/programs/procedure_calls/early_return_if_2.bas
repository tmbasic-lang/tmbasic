sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    if x = 0 then
        return 1
    else
        return 2
    end if
end function

--output--
2
