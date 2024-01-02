function GetNum(i as Number) as Number
    return i
end function
sub Main()
    dim x = { foo: [111, 222, 333], bar: 444 }
    dim i = 1
    print x.foo(GetNum(2) * GetNum(i))
end sub
--output--
333
