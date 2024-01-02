sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    select case x
        case 1
            return 1
        case 2
            return 2
        case else
            return 3
    end select
end function

--output--
3
