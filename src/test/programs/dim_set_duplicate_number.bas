#procedure
sub Main()
    dim set foo
        yield 999
        yield 999
    end dim
    print Len(foo)
    for each x in foo
        print x
    next
end sub

--output--
1
999
