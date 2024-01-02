sub Main()
    dim offset = Hours(-6)
    dim x = DateTimeOffsetFromParts(2021, 3, 12, 4, 30, 0, 0, offset)
    print x
end sub
--output--
2021-03-12 04:30:00.000 -06:00
