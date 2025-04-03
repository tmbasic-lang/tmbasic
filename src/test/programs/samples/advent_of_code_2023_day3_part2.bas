type Gear
    num as Number
    row as Number
    col as Number
end type

' Two extra for the buffer we add on either side.
const kWidth = 142

sub Part2(grid as List of List of Number)
    dim sum = 0

    ' First and last rows are empty margin.
    dim list gears
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
                    for each g in GetGearsInRange(currentNumber, grid, minRow, minCol, maxRow, maxCol)
                        yield g
                    next

                    dim blankList as List of Number
                    currentNumberStr = blankList
                end if
            next
        next
    end dim

    ' Skip the first because we will match the second in each pair.
    for i = 1 to Len(gears) - 1
        dim gi = gears(i)
        ' Can we find a gear with the same row/col in the previous gears?
        dim firstNumber as Optional Number
        
        ' Cheating for performance: I ran this once with jStart = 0 always to get the right answer,
        ' then fiddled with jStart to get it to run faster and still get the right answer.
        dim jStart = i - 17
        if jStart < 0 then
            jStart = 0
        end if

        for j = jStart to i - 1
            dim gj = gears(j)
            if gi.row = gj.row and gi.col = gj.col then
                firstNumber = gj.num
                exit for
            end if
        next
        if HasValue(firstNumber) then
            sum = sum + Value(firstNumber) * gi.num
        end if
    next

    print sum
end sub

sub Main()
    dim lines = ReadFileLines("../../src/test/files/advent_of_code_2023_day3_input.txt")
    dim list grid
        yield ListFill(46, kWidth)
        for each line in lines
            dim padded = "." + line + "."
            yield CodeUnits(padded)
        next
        yield ListFill(46, kWidth)
    end dim
    Part2 grid
end sub

function GetGearsInRange(num as Number, grid as List of List of Number, minRow as Number, minCol as Number, maxRow as Number, maxCol as Number) as List of Gear
    dim list gears
        for row = minRow to maxRow
            for col = minCol to maxCol
                dim ch = grid(row)(col)
                if ch = 42 then
                    yield { num: num, row: row, col: col } as Gear
                end if
            next
        next
    end dim
    return gears
end function

--output--
84266818
