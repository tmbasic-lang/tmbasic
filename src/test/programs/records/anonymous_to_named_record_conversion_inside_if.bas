type Bar
    baz as Number
end type

sub Main()
    if true then
        dim x = { baz: 0 } as Bar
    end if
    print "ok"
end sub

--output--
ok
