sub Main()
    dim set foo
        yield 5
    end dim
    print Contains(foo, 5)
    print Contains(foo, 6)
end sub

--output--
true
false
