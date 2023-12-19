#procedure
sub Main()
    dim set foo
        yield 1
    end dim
    dim set bar
        yield 2
    end dim
    print Len(foo + bar)
    print Len(foo + foo)
end sub

--output--
2
1
