#procedure
sub Main()
    dim set foo
        yield "a"
    end dim
    dim set bar
        yield "b"
    end dim
    print Len(foo + bar)
    print Len(foo + foo)
end sub

--output--
2
1
