sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    select case x
        case 1
            return 1
        case 2
            return 2
    end select
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
Control may reach the end of this function without returning or throwing.
