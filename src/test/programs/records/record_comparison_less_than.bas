type Foo
    Bar as Number
end type

sub Main()
    dim a as Foo
    dim b as Foo
    if a < b then print "wrong"
end sub

--output--
Compiler error
kTypeMismatch
Main
4:10
The "<" operator does not support the type "Foo".
