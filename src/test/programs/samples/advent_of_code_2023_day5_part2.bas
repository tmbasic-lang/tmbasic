type MapRange
    srcMin as Number
    srcMax as Number
    dstMin as Number
end type

type InputData
    seeds as List of Number
    maps as List of List of MapRange
end type

type SeedRange
    min as Number
    max as Number
end type

sub Main()
    dim data = ParseFile()
    dim list seedRanges
        for i = 0 to Len(data.seeds) - 1 step 2
            yield { min: data.seeds(i), max: data.seeds(i) + data.seeds(i + 1) - 1 } as SeedRange
        next
    end dim

    for each mapRanges in data.maps
        seedRanges = Subdivide(seedRanges, mapRanges)
        dim list newSeedRanges
            for each seedRange in seedRanges
                yield Part2Step(seedRange, mapRanges)
            next
        end dim
        seedRanges = newSeedRanges
    next

    dim minStart as Optional Number
    for each seedRange in seedRanges
        if not HasValue(minStart) or seedRange.min < Value(minStart) then
            minStart = seedRange.min
        end if
    next
    print Value(minStart)
end sub

function Subdivide(seedRanges as List of SeedRange, mapRanges as List of MapRange) as List of SeedRange
    dim list newSeedRanges
        for each seedRange in seedRanges
            for each x in Subdivide(seedRange, mapRanges)
                yield x
            next
        next
    end dim
    return newSeedRanges
end function

function Subdivide(seedRange as SeedRange, mapRanges as List of MapRange) as List of SeedRange
    ' If any of the mapRange min or max is within the seedRange, then subdivide and recurse.
    for each mapRange in mapRanges
        if mapRange.srcMin = seedRange.min then
            ' no subdivide needed.
        else if mapRange.srcMax = seedRange.max then
            ' no subdivide needed.
        else if mapRange.srcMin >= seedRange.min and mapRange.srcMin <= seedRange.max then
            ' subdivide.
            dim seedRange1 = { min: seedRange.min, max: mapRange.srcMin - 1 } as SeedRange
            dim seedRange2 = { min: mapRange.srcMin, max: seedRange.max } as SeedRange
            return Subdivide(seedRange1, mapRanges) + Subdivide(seedRange2, mapRanges)
        else if mapRange.srcMax >= seedRange.min and mapRange.srcMax <= seedRange.max then
            ' subdivide.
            dim seedRange1 = { min: seedRange.min, max: mapRange.srcMax } as SeedRange
            dim seedRange2 = { min: mapRange.srcMax + 1, max: seedRange.max } as SeedRange
            return Subdivide(seedRange1, mapRanges) + Subdivide(seedRange2, mapRanges)
        end if
    next
    return [seedRange]
end function

function Part2Step(seedRange as SeedRange, mapRanges as List of MapRange) as SeedRange
    for each mapRange in mapRanges
        ' Thanks to Subdivide() we know that the seedRange is either entirely inside or entirely outside the mapRange.
        if seedRange.min >= mapRange.srcMin and seedRange.max <= mapRange.srcMax then
            ' Entirely inside.
            dim seedRange1 = { min: mapRange.dstMin + seedRange.min - mapRange.srcMin, max: mapRange.dstMin + seedRange.max - mapRange.srcMin } as SeedRange
            return seedRange1
        end if
    next

    ' If we made it this far, then seedRange is not affected by any mapRange.
    return seedRange
end function

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

function ParseSeeds(line as String) as List of Number
    dim parts = Split(line, " ")
    dim list seeds
        for i = 1 to Len(parts) - 1
            yield ParseNumber(parts(i))
        next
    end dim
    return seeds
end function

function ParseMap(lines as List of String, i as Number) as Record (ranges as List of MapRange, index as Number)
    dim list ranges
        while Len(lines(i)) > 0
            dim parts = Split(lines(i), " ")
            dim srcMin = ParseNumber(parts(1))
            dim length = ParseNumber(parts(2))
            yield { srcMin: srcMin, srcMax: srcMin + length - 1, dstMin: ParseNumber(parts(0)) } as MapRange
            i = i + 1
        wend
    end dim
    return { ranges: ranges, index: i }
    ' index is the blank line after the map
end function

--output--
50855035
