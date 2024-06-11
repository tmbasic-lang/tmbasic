type Foo
    child as Optional Foo
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
Foo
2:23
The type "Foo" contains a recursive type reference which is not allowed. The cycle is: foo -> optional foo -> foo
