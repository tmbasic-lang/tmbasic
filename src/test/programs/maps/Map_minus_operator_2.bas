sub Main()
    dim map foo
        yield 1 to 2
        yield 3 to 4
    end dim
    dim map bar
        yield 5 to "b"
        yield 6 to "d"
    end dim
    dim baz = foo - bar
end sub

--output--
Compiler error
kTypeMismatch
Main
10:19
The types Map from Number to Number and Map from Number to String are not valid operands for the "-" operator.
