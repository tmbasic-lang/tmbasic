type Foo
    Bar as Number
end type

sub Main()
    dim a as Foo
    dim b as Foo
    if a = b then print "ok"
end sub

--output--
ok
