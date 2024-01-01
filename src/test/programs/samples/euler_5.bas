#procedure
function GCD(a as Number, b as Number) as Number
    while b <> 0
        dim temp = b
        b = a mod b
        a = temp
    wend
    return a
end function

#procedure
function LCM(a as Number, b as Number) as Number
    return (a / GCD(a, b)) * b
end function

#procedure
sub Main()
    dim result = 1
    for i = 1 to 20
        result = LCM(result, i)
    next
    print result
end sub

--output--
232792560
