sub Main()
    dim list foo
        yield 5
        yield "A"
    end dim
end sub
--output--
Compiler error
kTypeMismatch
Main
4:9
All "yield" statements in a "dim list" must be of the same type. This block has "yield" statements of the incompatible types Number and String.
