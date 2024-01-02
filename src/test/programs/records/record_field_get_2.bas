sub Main()
    dim y = { baz: 3 }
    dim x = { foo: y, bar: 2 }
    print x.foo.baz
end sub
--output--
3
