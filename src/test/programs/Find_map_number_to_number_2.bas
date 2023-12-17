#procedure
sub Main()
    dim map foo
        yield 1 to 2
        yield 3 to 4
    end dim
    dim a = Find(foo, "a")
end sub

--output--
Compiler error
kProcedureNotFound
Main
6:13
