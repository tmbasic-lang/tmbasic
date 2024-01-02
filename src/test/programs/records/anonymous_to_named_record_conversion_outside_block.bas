type Bar
    baz as Number
end type

sub Main()
    dim x = { baz: 0 } as Bar
    print "ok"
end sub

--output--
ok
