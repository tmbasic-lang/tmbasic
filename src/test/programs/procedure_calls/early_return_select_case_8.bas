sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    select case x
        case 1
            throw 111, "foo"
        case 2
            throw 222, "bar"
        case else
            throw 333, "baz"
    end select
end function

--output--
Error
333
baz
