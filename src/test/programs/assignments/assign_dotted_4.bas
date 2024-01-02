dim x = [{ foo: 111 }]
sub Main()
    x(0).foo = 999
    print x(0).foo
end sub
--output--
Compiler error
kInvalidGlobalVariableType
dim x = [{ foo: 111 }]
1:9
