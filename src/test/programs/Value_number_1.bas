#procedure
sub Main()
    dim x as Optional Number
    try
        dim y = Value(x)
    catch
        print ErrorCode  ' kValueNotPresent 302
    end try
end sub
--output--
302
