sub Main()
    dim a = Hours(4)
    dim b = Hours(6)
    dim c as TimeSpan
    c = b - a
    print c
    c = a - b
    print c
end sub

--output--
0:02:00:00.000
-0:02:00:00.000
