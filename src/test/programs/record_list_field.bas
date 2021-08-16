#procedure
sub Main()
    dim x = { foo: [1, 2, 3], bar: 4 }
    print x.foo(0)
    print x.foo(1)
    print x.foo(2)
    print x.bar
end sub
--output--
1
2
3
4
