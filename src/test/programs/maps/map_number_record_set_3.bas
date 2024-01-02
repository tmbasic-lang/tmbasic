sub Main()
    dim foo as Map from Number to Record(a as Number, b as Number)
    try
        foo(5).a = 999
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
