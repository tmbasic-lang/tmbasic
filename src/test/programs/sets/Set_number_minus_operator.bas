#procedure
sub Main()
    dim set foo
        yield 5
    end dim
    print Len(foo)
    foo = foo - 5
    print Len(foo)
end sub

--output--
1
0
