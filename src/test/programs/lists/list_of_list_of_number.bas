#procedure
sub Main()
    dim foo as List of List of Number
    foo = [ [ 1234 ] ]
    print "Len(foo)="; Len(foo)

    dim bar as List of Number
    bar = foo(0)
    print "Len(bar)="; Len(bar)

    dim baz as Number
    baz = bar(0)
    print "baz="; baz

    dim baz2 as Number
    baz2 = foo(0)(0)
    print "baz2="; baz2
end sub

--output--
Len(foo)=1
Len(bar)=1
baz=1234
baz2=1234
