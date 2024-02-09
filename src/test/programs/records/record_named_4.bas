type Foo
    a as Number
    b as Number
end type
sub Main()
    dim bar = { a: 5 } as Foo
end sub
--output--
Compiler error
kInvalidTypeConversion
Main
2:15
This type conversion is not allowed because the target type has 2 field(s) but the source type has 1 field(s). The number of fields must match.
