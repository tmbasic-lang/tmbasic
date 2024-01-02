type Foo
    a as Number
end type
sub Main()
    dim bar = { a: 5 } as Foo
    print bar.a
end sub
--output--
5
