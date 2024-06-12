sub Main()
    dim a as Map from Number to Number
    a(5) = 10
    a(6) = 11
    dim b as Map from Number to Number
    b(6) = 11
    b(5) = 10
    dim c as Map from Number to Number
    c(50) = 100
    if a <> b then print "a <> b"
    if a <> c then print "a <> c"
end sub

--output--
a <> c
