#procedure
sub Main()
    dim lines = ReadFileLines("../src/test/files/advent_of_code_2023_day4_input.txt")
    for each line in lines
        line = Replace(line, "  ", " ")
        line = Replace(line, "  ", " ")
        dim parts = Split(line, " ")
        
        dim cardNumber = ParseNumber(Replace(parts(1), ":", ""))

        dim list winningNumbers
            for i = 0 to 9
                yield ParseNumber(parts(2 + i))
            next
        end dim

        dim list myNumbers
            for i = 0 to 24
                yield ParseNumber(parts(13 + i))
            next
        end dim
    next
end sub

