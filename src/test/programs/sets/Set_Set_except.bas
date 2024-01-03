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
    dim set bar
        yield key1
    end dim
    dim set baz
        yield key2
    end dim

    print Len(foo)

    dim x = foo - bar
    print Len(x)

    x = x - baz
    print Len(x)
end sub

--output--
2
1
0
