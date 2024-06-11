type Foo
    child as List of Bar
end type

type Bar
    child as Foo
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
Bar
2:14
The type "Foo" contains a recursive type reference which is not allowed. The cycle is: foo -> list of bar -> bar -> foo
