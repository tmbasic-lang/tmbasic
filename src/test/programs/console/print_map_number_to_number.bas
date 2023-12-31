#procedure
sub Main()
    dim x as Map from Number to Number
    x(5) = 6
    x(99) = 100
    print x
end sub

--output--
Map {5: 6, 99: 100}
