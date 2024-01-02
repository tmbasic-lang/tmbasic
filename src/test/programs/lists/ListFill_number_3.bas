sub Main()
    try
        dim x = ListFill(123, -1)
    catch
        print ErrorCode()
    end try
end sub
--output--
50
