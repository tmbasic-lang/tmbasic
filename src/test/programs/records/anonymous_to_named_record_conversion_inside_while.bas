type Bar
    baz as Number
end type

sub Main()
    while false
        dim x = { baz: 0 } as Bar
    wend
    print "ok"
end sub

--output--
ok
