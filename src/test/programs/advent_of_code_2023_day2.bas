#type
type Round
    red as Number
    green as Number
    blue as Number
end type

#procedure
function ParseRounds(parts as List of String) as List of Round
    ' Game 1: 3 blue, 4 red; 1 red, 2 green, 6 blue; 2 green
    dim rounds as List of Round

    dim round1 as Round
    for i = 2 to Len(parts) - 1 step 2
        dim colorCharacters = Characters(parts(i + 1))
        dim firstCharacter = colorCharacters(0)

        dim colorCount = ParseNumber(parts(i))
        if firstCharacter = "r" then
            round1.red = colorCount
        else if firstCharacter = "g" then
            round1.green = colorCount
        else if firstCharacter = "b" then
            round1.blue = colorCount
        end if

        dim lastCharacter = colorCharacters(Len(colorCharacters) - 1)
        if lastCharacter = ";" then
            rounds = rounds + round1
            round1.red = 0
            round1.green = 0
            round1.blue = 0
        end if
    next

    rounds = rounds + round1
    return rounds
end function

#procedure
function ParseGameNumber(part as String) as Number
    ' part is like "1:"
    dim partCodeUnits = CodeUnits(part)
    dim truncatedCodeUnits = Mid(partCodeUnits, 0, Len(partCodeUnits) - 1)
    return ParseNumber(StringFromCodeUnits(truncatedCodeUnits))
end function

#procedure
function Part1(gameNumber as Number, rounds as List of Round) as Number
    for each r in rounds
        if r.red > 12 or r.green > 13 or r.blue > 14 then
            return 0
        end if
    next
    return gameNumber
end function

#procedure
function Part2(gameNumber as Number, rounds as List of Round) as Number
    dim maxRed = 0
    dim maxGreen = 0
    dim maxBlue = 0
    for each r in rounds
        if r.red > maxRed then
            maxRed = r.red
        end if
        if r.green > maxGreen then
            maxGreen = r.green
        end if
        if r.blue > maxBlue then
            maxBlue = r.blue
        end if
    next
    return maxRed * maxGreen * maxBlue
end function

#procedure
sub Main()
    dim lines = ReadFileLines("../src/test/files/advent_of_code_2023_day2_input.txt")
    dim sum1 = 0
    dim sum2 = 0
    for each line in lines
        dim parts = Split(line, " ")
        dim gameNumber = ParseGameNumber(parts(1))
        dim rounds = ParseRounds(parts)
        sum1 = sum1 + Part1(gameNumber, rounds)
        sum2 = sum2 + Part2(gameNumber, rounds)
    next
    print sum1
    print sum2
end sub

--output--
2207
62241
