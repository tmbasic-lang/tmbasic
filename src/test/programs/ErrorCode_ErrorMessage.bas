#procedure
sub Main()
    try
        throw 5, "hello"
    catch
        print ErrorCode
        print ErrorMessage
    end try
end sub
--output--
5
hello
