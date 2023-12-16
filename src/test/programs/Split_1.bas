#procedure
sub Main()
    dim xs = Split("a,b,c", ",")
    print Len(xs)
    print xs(0)
    print xs(1)
    print xs(2)
end sub
--output--
3
a
b
c
