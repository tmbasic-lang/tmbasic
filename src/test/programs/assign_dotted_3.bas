#procedure
sub Main()
    dim x = [{ foo: 111 }]
    x(0).foo = 999
    print x(0).foo
end sub
--output--
999
