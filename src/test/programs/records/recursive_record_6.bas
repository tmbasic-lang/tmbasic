type Foo
    child as Map from Foo to Foo
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
Foo
2:23
The type "Foo" contains a recursive type reference which is not allowed. The cycle is: foo -> map from foo to foo -> foo
