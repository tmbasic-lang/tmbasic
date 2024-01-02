sub Main()
    print Foo(123)
end sub

function Foo(x as Number) as Number
    do
        do
            throw 999, "blah"
        loop while x = 5
    loop while x = 5
end function

--output--
Error
999
blah
