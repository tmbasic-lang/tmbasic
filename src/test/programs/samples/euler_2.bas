sub Main()
    ' https://projecteuler.net/problem=2
    dim sum = 0
    dim a = 1
    dim b = 1
    while a < 4000000
        dim c = a + b
        a = b
        b = c
        if a mod 2 = 0 then
            sum = sum + a
        end if
    wend
    print sum
end sub

--output--
4613732
