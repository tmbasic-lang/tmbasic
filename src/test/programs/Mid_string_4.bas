#procedure
sub Main()
    dim x = ["A", "B", "C", "D", "E"]
    try
        dim y = Mid(x, 50, 50)
    catch
        print ErrorCode()
    end try
end sub
--output--
301
