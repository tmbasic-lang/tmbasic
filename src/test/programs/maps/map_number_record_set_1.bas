sub Main()
    dim foo as Map from Number to Record(a as Number, b as Number)
    foo(5) = {a: 111, b: 222}
    print foo(5).a
end sub
--output--
111
