type Bar
    baz as Number
end type

sub Main()
    for i = 1 to 1
        dim x = { baz: 0 } as Bar
    next
    print "ok"
end sub

--output--
ok
