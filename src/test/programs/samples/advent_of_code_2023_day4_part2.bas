sub Main()
    dim lines = ReadFileLines("../../src/test/files/advent_of_code_2023_day4_input.txt")

    ' index is card number, value is count of winning numbers on that card
    dim list cardWins
        ' there is no card zero
        yield 0

        for each line in lines
            line = Replace(line, "  ", " ")
            line = Replace(line, "  ", " ")
            dim parts = Split(line, " ")
            
            dim set winningNumbers
                for i = 0 to 9
                    yield ParseNumber(parts(2 + i))
                next
            end dim

            dim wins = 0
            for i = 0 to 24
                dim num = ParseNumber(parts(13 + i))
                if Contains(winningNumbers, num) then
                    wins = wins + 1
                end if
            next

            yield wins
        next
    end dim

    ' index is card number, value is the number of copies we have
    dim list heldCards
        ' there is no card zero
        yield 0

        for each line in lines
            yield 1
        next
    end dim

    ' process cards starting at card number 1
    for i = 1 to len(heldCards) - 1
        dim numHeld = heldCards(i)
        dim winsOnCard = cardWins(i)
        for j = i + 1 to i + winsOnCard
            heldCards(j) = heldCards(j) + numHeld
        next
    next

    ' count all the cards
    dim sum = 0
    for each numHeld in heldCards
        sum = sum + numHeld
    next

    print sum
end sub

--output--
8805731
