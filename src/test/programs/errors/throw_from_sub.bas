#procedure
sub Main()
    try
        Foo
    catch
        print "caught"
    end try
end sub
#procedure
sub Foo()
    throw ""
end sub
--output--
caught
