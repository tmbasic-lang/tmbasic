#procedure
sub Main()
    dim x = DateTimeOffsetFromParts(2021, 3, 12, 4, 30, 59, 488, Hours(-4))
    print Millisecond(x)
end sub

--output--
488
