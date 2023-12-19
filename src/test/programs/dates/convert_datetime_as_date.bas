#procedure
sub Main()
    dim a = DateTimeFromParts(1776, 7, 4, 5, 15, 30, 450)
    dim b = a as Date
    print b
end sub
--output--
1776-07-04
