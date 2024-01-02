sub Main()
    dim x as List of Number
    try
        dim y = First(x)
    catch
        print ErrorCode()
    end try
end sub
--output--
304
