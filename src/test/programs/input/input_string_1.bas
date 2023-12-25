#procedure
sub Main()
    dim n as String
    input n

    ' Make sure input hasn't messed up the stack.
    dim x = 5
    print x + 1
    print x + 1
    print x + 1
    print x * 2
end sub

--input--
foo
--output--
6
6
6
10
