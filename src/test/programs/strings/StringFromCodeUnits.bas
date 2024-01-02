sub Main()
    dim poopEmoji = "💩"  ' U+1F4A, four code units
    dim poopCodeUnits = CodeUnits(poopEmoji)
    print Len(poopCodeUnits)
    print StringFromCodeUnits(poopCodeUnits)
end sub
--output--
4
💩
