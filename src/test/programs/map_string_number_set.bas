#procedure
sub Main()
    dim foo as Map from String to Number
    foo("abc") = 10
    print foo("abc")
end sub
--output--
10
