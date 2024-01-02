sub Main()
    dim map foo
        yield 1 to 2
        yield 3 to 4
    end dim
    dim a = Find(foo, 1)
    print "HasValue(a)="; HasValue(a)
    print "Value(a)="; Value(a)
    dim b = Find(foo, 2)
    print "HasValue(b)="; HasValue(b)
end sub

--output--
HasValue(a)=true
Value(a)=2
HasValue(b)=false
