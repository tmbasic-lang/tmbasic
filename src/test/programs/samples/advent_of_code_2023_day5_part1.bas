#type
type MapRange
    dstStart as Number
    srcStart as Number
    length as Number
end type

#type
type InputData
    seeds as List of Number
    maps as List of List of MapRange
end type

#procedure
sub Main()
    dim data = ParseFile()
    Part1 data
end sub

#procedure
function ParseFile() as InputData
    dim lines = ReadFileLines("../src/test/files/advent_of_code_2023_day5_input.txt")
    dim seeds = ParseSeeds(lines(0))
    dim list maps
        dim i = 3
        for n = 1 to 7
            dim m = ParseMap(lines, i)
            yield m.ranges
            i = m.index + 2
        next
    end dim
    return { seeds: seeds, maps: maps } as InputData
end function

#procedure
function ParseSeeds(line as String) as List of Number
    dim parts = Split(line, " ")
    dim list seeds
        for i = 1 to Len(parts) - 1
            yield ParseNumber(parts(i))
        next
    end dim
    return seeds
end function

#procedure
function ParseMap(lines as List of String, i as Number) as Record (ranges as List of MapRange, index as Number)
    dim list ranges
        while Len(lines(i)) > 0
            dim parts = Split(lines(i), " ")
            yield { dstStart: ParseNumber(parts(0)), srcStart: ParseNumber(parts(1)), length: ParseNumber(parts(2)) } as MapRange
            i = i + 1
        wend
    end dim
    return { ranges: ranges, index: i }
    ' index is the blank line after the map
end function

#procedure
sub Part1(data as InputData)
    dim minFinalNum as Optional Number
    for each seed in data.seeds
        dim num = seed
        for each mapRanges in data.maps
            for each mapRange in mapRanges
                if num >= mapRange.srcStart and num < mapRange.srcStart + mapRange.length then
                    num = mapRange.dstStart + num - mapRange.srcStart
                    exit for
                end if
            next
        next

        if not HasValue(minFinalNum) or num < Value(minFinalNum) then
            minFinalNum = num
        end if
    next
    print Value(minFinalNum)
end sub

--output--
157211394
