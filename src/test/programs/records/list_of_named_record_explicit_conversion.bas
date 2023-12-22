#type
type Foo
    zzz as Number
end type

#type
type Bar
    baz as Foo
end type

#procedure
sub Main()
    dim f as Foo
    dim x = { baz: f } as Bar
end sub

--output--
