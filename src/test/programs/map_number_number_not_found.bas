#procedure
sub Main()
    try
        dim foo as Map from Number to Number
        print foo(5)
    catch
        if ErrorCode() = ERR_MAP_KEY_NOT_FOUND then
            print "ERR_MAP_KEY_NOT_FOUND"
        else
            print "some other error"
        end if
    end try
end sub
--output--
ERR_MAP_KEY_NOT_FOUND
