sub Main()
    dim map foo
        yield 1 to "a"
        yield 3 to "b"
    end dim
    dim a = Find(foo, 1)
    print "HasValue(a)="; HasValue(a)
    print "Value(a)="; Value(a)
    dim b = Find(foo, 2)
    print "HasValue(b)="; HasValue(b)
end sub

--output--
HasValue(a)=true
Value(a)=a
HasValue(b)=false
