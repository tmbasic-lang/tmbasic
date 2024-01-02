#procedure
sub Main()
    dim map foo
        yield 1 to 2
        yield 3 to 4
    end dim
    dim map bar
        yield 5 to 6
        yield 3 to 8
    end dim
    dim baz = foo + bar
    print Len(baz)
    print baz(1)
    print baz(3)
    print baz(5)
end sub

--output--
3
2
8
6
