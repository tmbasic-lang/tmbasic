#procedure
sub Main()
    ' https://projecteuler.net/problem=3
    dim n = 600851475143
    dim factor = 2
    dim largestFactor = 1

    while n > 1
        if n mod factor = 0 then
            largestFactor = factor
            n = n / factor
            while n mod factor = 0
                n = n / factor
            wend
        end if
        factor = factor + 1
    wend

    print largestFactor
end sub

--output--
6857
