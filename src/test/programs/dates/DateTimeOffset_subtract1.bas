sub Main()
    dim a = DateTimeOffsetFromParts(2000, 5, 6, 12, 30, 45, 998, Hours(-6))
    dim b = DateTimeOffsetFromParts(2000, 5, 7, 12, 30, 45, 999, Hours(-6))
    print b - a
    print a - b
end sub

--output--
1:00:00:00.001
-1:00:00:00.001
