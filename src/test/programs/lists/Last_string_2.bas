sub Main()
    dim x as List of String
    try
        dim y = Last(x)
    catch
        print ErrorCode()
    end try
end sub
--output--
304
