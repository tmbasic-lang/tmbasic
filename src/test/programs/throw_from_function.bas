#procedure
sub Main()
    try
        dim x = 5 + Foo(100)
    catch
        print "caught"
    end try
end sub
#procedure
function Foo(bar as Number) as Number
    throw ""
end function
--output--
caught
