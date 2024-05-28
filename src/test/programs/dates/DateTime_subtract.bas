sub Main()
    dim a = DateTimeFromParts(2000, 5, 6, 12, 30, 45, 998)
    dim b = DateTimeFromParts(2000, 5, 7, 12, 30, 45, 999)
    dim c as TimeSpan
    c = b - a
    print c
    c = a - b
    print c
end sub

--output--
1:00:00:00.001
-1:00:00:00.001
