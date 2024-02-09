sub Main()
    dim a = [1, 2, 3]
    dim b = ["A", "B", "C"]
    dim c = a + b
end sub
--output--
Compiler error
kTypeMismatch
Main
4:15
The types List of Number and List of String are not valid operands for the "+" operator.
