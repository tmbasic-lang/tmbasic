sub Main()
    dim set foo
        yield 1
        yield 2
    end dim
    dim set bar
        yield 1
    end dim
    dim baz = foo - bar
    print baz
end sub

--output--
Set [2]
