sub Main()
    dim map foo
        yield "a" to 2
        yield "b" to 4
    end dim
    dim a = Find(foo, "a")
    print "HasValue(a)="; HasValue(a)
    print "Value(a)="; Value(a)
    dim b = Find(foo, "c")
    print "HasValue(b)="; HasValue(b)
end sub

--output--
HasValue(a)=true
Value(a)=2
HasValue(b)=false
