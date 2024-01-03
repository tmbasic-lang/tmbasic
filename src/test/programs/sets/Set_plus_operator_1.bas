sub Main()
    dim foo as Set of String
    foo = foo + "a"
    foo = foo + "a"
    print Len(foo)
    for each x in foo
        print x
    next
end sub

--output--
1
a
