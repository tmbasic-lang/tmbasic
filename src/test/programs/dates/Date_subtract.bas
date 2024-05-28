sub Main()
    dim a = DateFromParts(2000, 5, 6)
    dim b = DateFromParts(2000, 5, 7)
    dim c as TimeSpan
    c = b - a
    print c
    c = a - b
    print c
end sub

--output--
1:00:00:00.000
-1:00:00:00.000
