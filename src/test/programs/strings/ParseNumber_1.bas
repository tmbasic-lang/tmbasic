sub Main()
    ' Establish that it returns a Number.
    dim a as Number
    a = ParseNumber("5")
    print a

    print ParseNumber("5.5")
    print ParseNumber("-5.5")
    print ParseNumber("-5e5")
    print ParseNumber("-5.5000000")
    print ParseNumber("9.00000000000008446744")
end sub
--output--
5
5.5
-5.5
-500000
-5.5
9.00000000000008446744
