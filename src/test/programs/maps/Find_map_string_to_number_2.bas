sub Main()
    dim map foo
        yield "a" to 2
        yield "b" to 4
    end dim
    dim a = Find(foo, 1)
end sub

--output--
Compiler error
kProcedureNotFound
Main
6:13
Call to procedure "Find" with the wrong parameters.
