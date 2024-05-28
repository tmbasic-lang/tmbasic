type MyRecord
    foo as Number
    bar as Number
end type

sub Main()
    dim x = { foo: 4, bar: 5 } as MyRecord
    print x
end sub

--output--
{ foo: 4, bar: 5 }
