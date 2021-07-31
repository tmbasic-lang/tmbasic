#procedure
sub Main()
    ' CodeUnit1
    dim x = Chr(65)
    print x
    print CodeUnit(x)

    ' CodeUnit2 - good call
    dim str = "ABCD"
    dim unit = CodeUnit(str, 1)
    print Chr(unit)
    print unit

    ' CodeUnit2 - index out of range
    print CodeUnit(str, -1)
    print CodeUnit(str, 50)
end sub
--output--
A
65
B
66
0
0
