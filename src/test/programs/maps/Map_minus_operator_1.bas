#procedure
sub Main()
    dim map foo
        yield 1 to 2
        yield 3 to 4
    end dim
    dim map bar
        yield 1 to 6
        yield 7 to 8
    end dim
    dim baz = foo - bar
    print Len(baz)
    print ContainsKey(baz, 1)
    print ContainsKey(baz, 3)
    print ContainsKey(baz, 5)
    print ContainsKey(baz, 7)
    print ContainsKey(baz, 9)
end sub

--output--
1
false
true
false
false
false
