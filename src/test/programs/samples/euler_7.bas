sub Main()
    dim primeCount = 0
    dim num = 1

    ' This program works with "primeCount < 10001"; the answer is 104743.
    ' For speed in testing, we'll compute the 1001th prime instead, which is 7927.
    while primeCount < 1001
        num = num + 1
        if IsPrime(num) then
            primeCount = primeCount + 1
        end if
    wend

    print num
end sub

function IsPrime(n as Number) as Boolean
    if n <= 1 then
        return false
    end if
    if n <= 3 then
        return true
    end if
    if n mod 2 = 0 or n mod 3 = 0 then
        return false
    end if

    dim i = 5
    while i * i <= n
        if n mod i = 0 or n mod (i + 2) = 0 then
            return false
        end if
        i = i + 6
    wend

    return true
end function

--output--
7927
