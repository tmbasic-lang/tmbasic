#procedure
sub Main()
    dim x = DateTimeOffsetFromParts(2021, 3, 12, 14, 30, 59, 488, Hours(-4))
    print Hour(x)
end sub

--output--
14
