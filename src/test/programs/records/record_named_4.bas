#type
type Foo
    a as Number
    b as Number
end type
#procedure
sub Main()
    dim bar = { a: 5 } as Foo
end sub
--output--
Compiler error
kInvalidTypeConversion
Main
2:15
