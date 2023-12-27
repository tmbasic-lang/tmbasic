#procedure
sub Main()
    dim x = [1, 2, 3]
    dim y = RemoveAt(x, [0, 2, 2, 2])
    print Len(y)
    print y(0)
end sub

--output--
1
2
