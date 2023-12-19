#procedure
sub Main()
    dim x = ["A", "B", "C"]
    dim y = Skip(x, 1)
    print Len(y)
    print y(0)
    print y(1)
end sub
--output--
2
B
C
