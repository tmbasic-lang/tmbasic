#procedure
sub Main()
    dim map foo
        yield 1 to "a"
        yield 3 to "b"
    end dim
    dim a = Find(foo, "a")
end sub

--output--
Compiler error
kProcedureNotFound
Main
6:13
