#type
type Bar
    baz as Number
end type

#procedure
sub Main()
    if true then
        dim x = { baz: 0 } as Bar
    end if
    print "ok"
end sub

--output--
ok
