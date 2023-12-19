#procedure
function Part1(line as List of String) as Number
    dim firstDigit as Optional String
    dim lastDigit as String

    for each ch in line
        if IsDigit(ch) then
            if not HasValue(firstDigit) then
                firstDigit = ch
            end if
            lastDigit = ch
        end if
    next
    
    return ParseNumber(Value(firstDigit) + lastDigit)
end function

#procedure
function Part2(lineChs as List of String) as Number
    dim firstDigit as Optional Number
    dim lastDigit as Number

    for i = 0 to Len(lineChs) - 1
        dim digit = GetDigit(lineChs, i)
        if digit >= 0 then
            if not HasValue(firstDigit) then
                firstDigit = digit
            end if
            lastDigit = digit
        end if
    next

    return 10 * Value(firstDigit) + lastDigit
end function

#procedure
sub Main()
    dim lines = ReadFileLines("../src/test/files/advent_of_code_2023_day1_input.txt")
    dim sum1 = 0
    dim sum2 = 0
    for each line in lines
        dim chs = Characters(line)
        sum1 = sum1 + Part1(chs)
        sum2 = sum2 + Part2(chs)
    next
    print sum1
    print sum2
end sub

#procedure
function Range(chars as List of String, offset as Number, length as Number) as String
    dim subchars = Mid(chars, offset, length)
    return Concat(subchars)
end function

#procedure
function Match(chars as List of String, offset as Number, target as String) as Boolean
    dim substr = Range(chars, offset, Len(target))
    return substr = target
end function

#procedure
function IsDigit(ch as String) as Boolean
    dim asc = CodeUnit(ch)
    return asc >= 48 and asc <= 57
end function

#procedure
function GetDigit(chs as List of String, i as Number) as Number
    dim ch = chs(i)
    dim asc = CodeUnit(ch)
    if (IsDigit(ch)) then
        return ParseNumber(ch)
    end if

    if asc = 111 then ' o
        if Match(chs, i, "one") then
            return 1
        end if    
    else if asc = 116 then ' t
        if Match(chs, i, "two") then
            return 2
        end if
        if Match(chs, i, "three") then
            return 3
        end if
    else if asc = 102 then ' f
        if Match(chs, i, "four") then
            return 4
        end if
        if Match(chs, i, "five") then
            return 5
        end if
    else if asc = 115 then ' s
        if Match(chs, i, "six") then
            return 6
        end if
        if Match(chs, i, "seven") then
            return 7
        end if
    else if asc = 101 then ' e
        if Match(chs, i, "eight") then
            return 8
        end if
    else if asc = 110 then ' n
        if Match(chs, i, "nine") then
            return 9
        end if
    else if asc = 122 then ' z
        if Match(chs, i, "zero") then
            return 0
        end if
    end if
    return -1
end function

--output--
54697
54885
