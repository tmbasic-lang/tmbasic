#procedure
sub Main()
    dim x = { foo: 1, bar: 2 }
    x.foo = 999
    print x
end sub
--output--
{ foo: 999, bar: 2 }
