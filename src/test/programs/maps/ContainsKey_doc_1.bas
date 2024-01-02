sub Main()
    dim map myMap
        yield "aaa" to "111"
        yield "bbb" to "222"
    end dim
    print ContainsKey(myMap, "aaa")
    print ContainsKey(myMap, "ccc")
end sub

--output--
true
false
