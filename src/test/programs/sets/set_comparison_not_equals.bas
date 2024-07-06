sub Main()
    dim set a
        yield 5
        yield 6
    end dim
    dim set b
        yield 6
        yield 5
    end dim
    dim set c
        yield 1
    end dim
    if a <> a then print "a <> a"
    if a <> b then print "a <> b"
    if a <> c then print "a <> c"
end sub

--output--
a <> c
