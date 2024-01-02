sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    select case x
        case 1
            throw 999, "blah"
        case 2
            print "2"
    end select
end function

--output--
Compiler error
kControlReachesEndOfFunction
Foo
1:1
