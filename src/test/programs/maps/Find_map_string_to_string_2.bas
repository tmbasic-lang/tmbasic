sub Main()
    dim map foo
        yield "a" to "x"
        yield "b" to "y"
    end dim
    dim a = Find(foo, 1)
end sub

--output--
Compiler error
kProcedureNotFound
Main
6:13
