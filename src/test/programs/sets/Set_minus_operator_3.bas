sub Main()
    dim set foo
        yield 1
        yield 2
    end dim
    dim baz = foo - 2
    print baz
    baz = baz - 3
    print baz
end sub

--output--
Set [1]
Set [1]
