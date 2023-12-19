#procedure
sub Main()
    dim foo as Set of Number
    foo = foo + 5
    foo = foo + 5
    print Len(foo)
    for each x in foo
        print x
    next
end sub

--output--
1
5
