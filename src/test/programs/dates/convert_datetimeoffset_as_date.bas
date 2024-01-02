sub Main()
    dim a = DateTimeOffsetFromParts(1776, 7, 4, 5, 15, 30, 450, Hours(-6))
    dim b = a as Date
    print b
end sub
--output--
1776-07-04
