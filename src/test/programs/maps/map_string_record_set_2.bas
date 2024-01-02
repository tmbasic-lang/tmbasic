sub Main()
    dim foo as Map from String to Record(a as Number, b as Number)
    foo("abc") = {a: 111, b: 222}
    print foo("abc").a
    foo("abc").a = 999
    print foo("abc").a
end sub
--output--
111
999
