#procedure
sub Main()
    dim set foo
        yield "a"
    end dim
    print Contains(foo, "a")
    print Contains(foo, "b")
end sub

--output--
true
false
