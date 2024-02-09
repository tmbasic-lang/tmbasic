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
This type conversion is not allowed because the source and target types have different field names. The field at index 0 is named "z" in the source type but "a" in the target type.
