sub Main()
    try
        Foo
    catch
        print "caught"
    end try
end sub
sub Foo()
    throw ""
end sub
--output--
caught
