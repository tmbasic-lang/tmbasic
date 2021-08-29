#procedure
sub Main()
    dim x = [1]
    x(0) = 2
    print x(0)

    dim y = ["A"]
    y(0) = "B"
    print y(0)
end sub
--output--
2
B
