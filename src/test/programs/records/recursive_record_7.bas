type Foo
    child as Set of Foo
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
Foo
2:21
The type "Foo" contains a recursive type reference which is not allowed. The cycle is: foo -> set of foo -> foo
