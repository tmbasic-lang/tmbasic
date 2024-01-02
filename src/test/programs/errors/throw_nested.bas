sub Main()
    try
        try
            print "a"
            throw ""
        catch
            print "b"
            throw ""
        end try
    catch
        print "c"
    end try
end sub
--output--
a
b
c
