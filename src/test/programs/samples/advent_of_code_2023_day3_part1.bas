' Two extra for the buffer we add on either side.
const kWidth = 142

sub Part1(grid as List of List of Number)
    dim sum = 0

    ' First and last rows are empty margin.
    for row = 1 to Len(grid) - 2
        ' First and last columns are empty margin.
        dim insideNumber = false
        dim currentNumberStr as List of Number
        dim currentNumberStartCol = 0
        for col = 1 to kWidth - 1
            dim ch = grid(row)(col)
            if ch >= 48 and ch <= 57 then
                if insideNumber then
                    ' We're continuing to build up a number.
                    currentNumberStr = currentNumberStr + ch
                else
                    ' We found the start of a number.
                    insideNumber = true
                    currentNumberStr = [ch]
                    currentNumberStartCol = col
                end if
            else if insideNumber then
                ' We've found a complete number.
                insideNumber = false
                dim currentNumber = ParseNumber(StringFromCodeUnits(currentNumberStr))

                ' Is there a symbol adjacent?
                dim minRow = row - 1
                dim maxRow = row + 1
                dim minCol = currentNumberStartCol - 1
                dim maxCol = col ' col is the first non-digit
                if IsPunctuationInRange(grid, minRow, minCol, maxRow, maxCol) then
                    ' This is a part number.
                    sum = sum + currentNumber
                end if

                dim blankList as List of Number
                currentNumberStr = blankList
            end if
        next
    next

    print sum
end sub

sub Main()
    dim lines = ReadFileLines("src/test/files/advent_of_code_2023_day3_input.txt")
    dim grid = [ListFill(46, kWidth)]
    for each line in lines
        dim padded = "." + line + "."
        dim lineCodeUnits = CodeUnits(padded)
        grid = grid + lineCodeUnits
    next
    grid = grid + ListFill(46, kWidth)
    Part1 grid
end sub

function IsPunctuationInRange(grid as List of List of Number, minRow as Number, minCol as Number, maxRow as Number, maxCol as Number) as Boolean
    for row = minRow to maxRow
        for col = minCol to maxCol
            dim ch = grid(row)(col)
            if (ch < 48 or ch > 57) and ch <> 46 then
                return true
            end if
        next
    next
    return false
end function

--output--
557705
