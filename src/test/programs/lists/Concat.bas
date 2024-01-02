sub Main()
    dim x = ["Hello ", "world", "!"]
    print Concat(x)

    dim y = ["A", "B", "C"]
    print Concat(y, "123")
end sub
--output--
Hello world!
A123B123C
