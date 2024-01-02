sub Main()
    dim foo as Map from Record(a as String, b as String) to Record(a as Number, b as Number)
    foo({a: "abc", b: "def"}) = {a: 111, b: 222}
    print foo({a: "abc", b: "def"}).a
end sub
--output--
111
