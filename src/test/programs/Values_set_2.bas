#procedure
sub Main()
    dim set foo
        yield 1
        yield 2
    end dim
    dim bar as List of Number
    bar = Values(foo)
    print Len(bar)
end sub

--output--
2
