#type
type Foo
    a as Number
end type
#procedure
sub Main()
    dim bar as Foo
    print bar.a
end sub
--output--
0
