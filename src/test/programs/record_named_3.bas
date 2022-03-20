#type
type Foo
    a as Number
end type
#procedure
sub Main()
    dim bar = { a: 5 } as Foo
    dim baz = bar as record (a as Number)
    print baz.a
end sub
--output--
5
