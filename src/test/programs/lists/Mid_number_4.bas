sub Main()
    dim x = [1, 2, 3, 4, 5]
    try
        dim y = Mid(x, 50, 50)
    catch
        print ErrorCode()
    end try
end sub
--output--
301
