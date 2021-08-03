#procedure
sub Main()
    do
        print 1
    loop while false

    dim x = true
    do
        print 2
        x = false
    loop while x

    dim n = 3
    do
        print "3 "; n
        n = n - 1
    loop while n > 0
end sub
--output--
1
2
3 3
3 2
3 1
