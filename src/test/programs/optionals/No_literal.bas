type Foo
    bar as Number
end type

sub Main()
    dim x = No $1
    print HasValue(x)
end sub
--output--
false
--cases--
1|Number
2|String
3|List of Number
4|Foo
5|Record (bar as Number)
