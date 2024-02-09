type Foo
    a as Number
    b as Number
end type
sub Main()
    dim bar = { a: 5, b: 6 } as Foo
    dim baz = bar as record (a as Number)
end sub
--output--
Compiler error
kInvalidTypeConversion
Main
3:15
This type conversion is not allowed because the target type has 1 field(s) but the source type has 2 field(s). The number of fields must match.
