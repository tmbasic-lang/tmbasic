sub Main()
    dim x = 0
    do
        x = x + 1
        if x mod 2 = 0 then
            exit do
        end if
        print x
    loop while x < 10
end sub
--output--
1
