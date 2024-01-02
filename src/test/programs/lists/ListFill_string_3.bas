sub Main()
    try
        dim x = ListFill("A", -1)
    catch
        print ErrorCode()
    end try
end sub
--output--
50
