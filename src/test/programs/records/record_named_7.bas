type Foo
    z as Number
end type
sub Main()
    dim bar = { z: 5 } as Foo
    dim baz = bar as record (a as Number)
end sub
--output--
Compiler error
kInvalidTypeConversion
Main
3:15
