#procedure
sub Main()
    dim a = [1, 2, 3]
    dim b = [4, 5, 6]
    dim c = a + b
    print Len(c)
    print c(0)
    print c(1)
    print c(2)
    print c(3)
    print c(4)
    print c(5)

    print "-"

    dim d = ["A", "B", "C"]
    dim e = ["D", "E", "F"]
    dim f = d + e
    print Len(f)
    print f(0)
    print f(1)
    print f(2)
    print f(3)
    print f(4)
    print f(5)
end sub
--output--
6
1
2
3
4
5
6
-
6
A
B
C
D
E
F
