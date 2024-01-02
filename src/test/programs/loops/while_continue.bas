sub Main()
    dim x = 0
    while x < 10
        x = x + 1
        if x mod 2 = 0 then
            continue while
        end if
        print x
    wend
end sub
--output--
1
3
5
7
9
