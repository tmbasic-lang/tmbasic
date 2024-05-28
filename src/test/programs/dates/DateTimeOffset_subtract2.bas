sub Main()
    dim a = DateTimeOffsetFromParts(2000, 5, 6, 13, 30, 45, 999, Hours(-5))
    dim b = DateTimeOffsetFromParts(2000, 5, 6, 13, 30, 45, 999, Hours(-6))
    print b - a
    print a - b
end sub

--output--
0:01:00:00.000
-0:01:00:00.000
