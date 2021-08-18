#procedure
sub Main()
    dim a as List of Number
    print Len(a)
    a = a + 999
    print Len(a)
    print a(0)
    a = a + -999
    print Len(a)
    print a(1)

    dim b as List of String
    print Len(b)
    b = b + "hello"
    print Len(b)
    print b(0)
    b = b + "world"
    print Len(b)
    print b(1)
end sub
--output--
0
1
999
2
-999
0
1
hello
2
world
