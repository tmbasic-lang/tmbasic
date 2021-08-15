#procedure
sub Main()
    for i = 1 to 10
        if i mod 2 = 0 then
            exit for
        end if
        print i
    next
end sub
--output--
1
