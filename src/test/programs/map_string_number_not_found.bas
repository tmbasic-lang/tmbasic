#procedure
sub Main()
    try
        dim foo as Map from String to Number
        print foo("abc")
    catch
        if ErrorCode = ERR_MAP_KEY_NOT_FOUND then
            print "ERR_MAP_KEY_NOT_FOUND"
        else
            print "some other error"
        end if
    end try
end sub
--output--
ERR_MAP_KEY_NOT_FOUND
