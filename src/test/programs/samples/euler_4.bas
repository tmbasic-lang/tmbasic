sub Main()
    ' https://projecteuler.net/problem=4
    dim largestPalindrome = 0

    ' Cheat to make this faster for testing purposes; we already know a=913 and b=993.
    ' The program works correctly without this cheating optimization.
    for a = 912 to 914
        for b = a to 994
            dim product = a * b
            if product > largestPalindrome and IsPalindrome(CodeUnits(product as String)) then
                largestPalindrome = product
            end if
        next
    next

    print largestPalindrome
end sub

function IsPalindrome(bytes as List of Number) as Boolean
    dim length = Len(bytes)
    for i = 0 to length / 2 - 1
        if bytes(i) <> bytes(length - i - 1) then
            return false
        end if
    next
    return true
end function

--output--
906609
