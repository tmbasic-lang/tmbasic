type Foo
    child as Foo
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
Foo
2:14
The type "Foo" contains a recursive type reference which is not allowed. The cycle is: foo -> foo
