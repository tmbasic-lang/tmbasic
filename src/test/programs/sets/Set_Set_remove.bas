sub Main()
    dim set key1
        yield 1
    end dim
    dim set key2
        yield 2
    end dim

    dim set foo
        yield key1
        yield key2
    end dim

    print Len(foo)

    foo = foo - key1
    print Len(foo)

    foo = foo - key2
    print Len(foo)
end sub

--output--
2
1
0
