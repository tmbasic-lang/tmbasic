#procedure
sub Main()
    dim set foo
        yield "a"
    end dim
    print Len(foo)
    foo = foo - "a"
    print Len(foo)
end sub

--output--
1
0
