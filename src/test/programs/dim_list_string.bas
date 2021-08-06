#procedure
sub Main()
    dim list foo
        yield "A"
        yield "B"
        yield "C"
    end dim
    print Len(foo)
    print foo(0)
    print foo(1)
    print foo(2)
end sub
--output--
3
A
B
C
