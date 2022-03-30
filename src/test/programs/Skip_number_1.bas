#procedure
sub Main()
    dim x = [1, 2, 3]
    dim y = Skip(x, 1)
    print Len(y)
    print y(0)
    print y(1)
end sub
--output--
2
2
3
