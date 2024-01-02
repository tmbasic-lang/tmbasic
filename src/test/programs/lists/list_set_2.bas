sub Main()
    dim foo = [111, 222, 333]
    try
        foo(100) = 999
    catch
        if ErrorCode() = ERR_LIST_INDEX_OUT_OF_RANGE then
            print "ERR_LIST_INDEX_OUT_OF_RANGE"
        else
            print "some other error"
        end if
    end try
end sub
--output--
ERR_LIST_INDEX_OUT_OF_RANGE
