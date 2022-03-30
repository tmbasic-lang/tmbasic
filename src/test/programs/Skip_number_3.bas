#procedure
sub Main()
    dim x = [1, 2, 3]
    try
        dim y = Skip(x, -10)
        print Len(y)
        print y(0)
    catch
        print ErrorCode
    end try
end sub
--output--
50
