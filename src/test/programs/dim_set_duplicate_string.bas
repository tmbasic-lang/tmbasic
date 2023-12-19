#procedure
sub Main()
    dim set foo
        yield "a"
        yield "a"
    end dim
    print Len(foo)
    for each x in foo
        print x
    next
end sub

--output--
1
a
