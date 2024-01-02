dim x as List of Record(foo as Number)
sub Main()
    print Len(x)
    x = [{ foo: 111 }]
    x(0).foo = 999
    print x(0).foo
end sub
--output--
0
999
