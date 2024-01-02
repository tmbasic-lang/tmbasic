type Bar
    baz as Number
end type

sub Main()
    dim aaa as List of Number
    for each bbb in aaa
        dim x = { baz: 0 } as Bar
    next
    print "ok"
end sub

--output--
ok
