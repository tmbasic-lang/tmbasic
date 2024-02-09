type Foo
    z as Number
end type
sub Main()
    dim bar = { a: 5 } as Foo
end sub
--output--
Compiler error
kInvalidTypeConversion
Main
2:15
This type conversion is not allowed because the source and target types have different field names. The field at index 0 is named "a" in the source type but "z" in the target type.
