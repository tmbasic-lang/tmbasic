#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    try
        throw 111, "foo"
    catch
        throw 222, "bar"
    end try
end function

--output--
Error
222
bar
