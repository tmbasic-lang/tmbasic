#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    try
        return 1
    catch
        return 2
    end try
end function

--output--
1
