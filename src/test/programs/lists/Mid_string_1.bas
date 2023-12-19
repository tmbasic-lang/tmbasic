#procedure
sub Main()
    dim x = ["A", "B", "C", "D", "E"]
    dim y = Mid(x, 1, 3)
    print Len(y)
    print y(0)
    print y(1)
    print y(2)
end sub
--output--
3
B
C
D
