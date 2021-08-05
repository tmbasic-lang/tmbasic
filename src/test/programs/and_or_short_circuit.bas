#procedure
function AlwaysThrows() as Boolean
    print "should not have called this!"
    throw 1, "error"
end function
#procedure
sub Main()
    if true or AlwaysThrows then
        print "1 good"
    else
        print "1 bad"
    end if

    if not (false and AlwaysThrows) then
        print "2 good"
    else
        print "2 bad"
    end if
end sub
--output--
1 good
2 good
