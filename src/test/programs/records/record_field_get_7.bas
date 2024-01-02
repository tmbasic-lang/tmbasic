function GetNum() as Number
    return 1
end function
sub Main()
    dim x = { foo: [111, 222, 333], bar: 444 }
    dim i = 1
    print x.foo(GetNum() + 1)
end sub
--output--
333
