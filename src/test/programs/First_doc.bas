#procedure
sub PrintFirstNumberIfPresent(x as List of Number)
    if Len(x) > 0 then
        print First(x)
    else
        print "Empty list!"
    end if
end sub
#procedure
sub Main()
    dim x = [1, 2, 3]
    PrintFirstNumberIfPresent x
    dim y as List of Number
    PrintFirstNumberIfPresent y
end sub
--output--
1
Empty list!
