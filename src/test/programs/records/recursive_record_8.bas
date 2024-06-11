type AAA
    Foo as List of BBB
end type

type BBB
    Foo as Map from CCC to DDD
end type

type CCC
    Foo as Set of DDD
end type

type DDD
    Foo as Optional AAA
end type

sub Main()
end sub

--output--
Compiler error
kRecursiveRecordType
DDD
2:21
The type "AAA" contains a recursive type reference which is not allowed. The cycle is: aaa -> list of bbb -> bbb -> map from ccc to ddd -> ccc -> set of ddd -> ddd -> optional aaa -> aaa
