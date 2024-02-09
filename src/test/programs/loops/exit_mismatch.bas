sub Main()
    dim x = 0
    while x < 10
        x = x + 1
        if x mod 2 = 0 then
            exit do
        end if
        print x
    wend
end sub
--output--
Compiler error
kExitTypeMismatch
Main
6:13
This "exit do" statement is inside a "while" loop. The loop type must match.
