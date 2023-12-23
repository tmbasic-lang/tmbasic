#procedure
sub Main()
    print Foo(123)
end sub

#procedure
function Foo(x as Number) as Number
    try
        throw 111, "foo"
    catch
        rethrow
    end try
end function

--output--
Error
111
foo
