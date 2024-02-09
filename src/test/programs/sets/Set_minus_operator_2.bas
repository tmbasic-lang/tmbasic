sub Main()
    dim set foo
        yield 1
        yield 2
    end dim
    dim set bar
        yield "a"
    end dim
    dim baz = foo - bar
end sub

--output--
Compiler error
kTypeMismatch
Main
9:19
The types Set of Number and Set of String are not valid operands for the "-" operator.
