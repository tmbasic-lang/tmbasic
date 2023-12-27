#procedure
sub Main()
    dim x = DateTimeFromParts(2021, 3, 12, 14, 30, 59, 488)
    print Second(x)
end sub

--output--
59
