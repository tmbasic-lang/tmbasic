sub Main()
    dim x = { foo: [111, 222, 333], bar: 444 }
    dim i = 1
    print x.foo(i + 1)
end sub
--output--
333
