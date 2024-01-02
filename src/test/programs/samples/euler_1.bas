sub Main()
    ' https://projecteuler.net/problem=1
    dim sum = 0
    for i = 1 to 999
        if i mod 3 = 0 or i mod 5 = 0 then
            sum = sum + i
        end if
    next
    print sum
end sub

--output--
233168
