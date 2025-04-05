sub Main()
    dim lines = ReadFileLines("src/test/files/advent_of_code_2023_day4_input.txt")
    dim sum = 0
    for each line in lines
        line = Replace(line, "  ", " ")
        line = Replace(line, "  ", " ")
        dim parts = Split(line, " ")
        
        dim cardNumber = ParseNumber(Replace(parts(1), ":", ""))

        dim set winningNumbers
            for i = 0 to 9
                yield ParseNumber(parts(2 + i))
            next
        end dim

        dim score = 0
        for i = 0 to 24
            dim num = ParseNumber(parts(13 + i))
            if Contains(winningNumbers, num) then
                if score = 0 then
                    score = 1
                else
                    score = score * 2
                end if
            end if
        next

        sum = sum + score
    next

    print sum
end sub

--output--
25571
