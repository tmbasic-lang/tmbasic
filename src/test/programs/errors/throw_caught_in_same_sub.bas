sub Main()
    try
        throw "hello"
    catch
        print "caught"
    end try
end sub
--output--
caught
