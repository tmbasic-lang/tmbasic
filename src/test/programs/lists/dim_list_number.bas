#procedure
sub Main()
    dim list foo
        yield 1
        yield 2
        yield 3
    end dim
    print Len(foo)
    print foo(0)
    print foo(1)
    print foo(2)
end sub
--output--
3
1
2
3
