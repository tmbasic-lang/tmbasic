type Foo
    Bar as Number
end type

sub Main()
    dim a as Foo
    dim b as Foo
    if a <> b then
        print "wrong"
    else
        print "ok"
    end if
end sub

--output--
ok
